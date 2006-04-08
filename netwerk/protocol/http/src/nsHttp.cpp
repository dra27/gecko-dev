/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim:set ts=4 sw=4 sts=4 et cin: */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Darin Fisher <darin@netscape.com> (original author)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "nsHttp.h"
#include "nsAutoLock.h"
#include "pldhash.h"
#include "nsCRT.h"

#if defined(PR_LOGGING)
PRLogModuleInfo *gHttpLog = nsnull;
#endif

// define storage for all atoms
#define HTTP_ATOM(_name, _value) nsHttpAtom nsHttp::_name = { _value };
#include "nsHttpAtomList.h"
#undef HTTP_ATOM

// find out how many atoms we have
#define HTTP_ATOM(_name, _value) Unused_ ## _name,
enum {
#include "nsHttpAtomList.h"
    NUM_HTTP_ATOMS
};
#undef HTTP_ATOM

// we keep a linked list of atoms allocated on the heap for easy clean up when
// the atom table is destroyed.  The structure and value string are allocated
// as one contiguous block.

struct HttpHeapAtom {
    struct HttpHeapAtom *next;
    char                 value[1];
};

static struct PLDHashTable  sAtomTable = {0};
static struct HttpHeapAtom *sHeapAtoms = nsnull;
static PRLock              *sLock = nsnull;

HttpHeapAtom *
NewHeapAtom(const char *value) {
    int len = strlen(value);

    HttpHeapAtom *a =
        NS_REINTERPRET_CAST(HttpHeapAtom *, malloc(sizeof(*a) + len));
    if (!a)
        return nsnull;
    memcpy(a->value, value, len + 1);

    // add this heap atom to the list of all heap atoms
    a->next = sHeapAtoms;
    sHeapAtoms = a;

    return a;
}

// Hash string ignore case, based on PL_HashString
PR_STATIC_CALLBACK(PLDHashNumber)
StringHash(PLDHashTable *table, const void *key)
{
    PLDHashNumber h = 0;
    for (const char *s = NS_REINTERPRET_CAST(const char*, key); *s; ++s)
        h = (h >> 28) ^ (h << 4) ^ nsCRT::ToLower(*s);
    return h;
}

PR_STATIC_CALLBACK(PRBool)
StringCompare(PLDHashTable *table, const PLDHashEntryHdr *entry,
              const void *testKey)
{
    const void *entryKey =
            NS_REINTERPRET_CAST(const PLDHashEntryStub *, entry)->key;

    return PL_strcasecmp(NS_REINTERPRET_CAST(const char *, entryKey),
                         NS_REINTERPRET_CAST(const char *, testKey)) == 0;
}

static const PLDHashTableOps ops = {
    PL_DHashAllocTable,
    PL_DHashFreeTable,
    PL_DHashGetKeyStub,
    StringHash,
    StringCompare,
    PL_DHashMoveEntryStub,
    PL_DHashClearEntryStub,
    PL_DHashFinalizeStub,
    nsnull
};

// We put the atoms in a hash table for speedy lookup.. see ResolveAtom.
nsresult
nsHttp::CreateAtomTable()
{
    NS_ASSERTION(!sAtomTable.ops, "atom table already initialized");

    if (!sLock) {
        sLock = PR_NewLock();
        if (!sLock)
            return NS_ERROR_OUT_OF_MEMORY;
    }

    // The capacity for this table is initialized to a value greater than the
    // number of known atoms (NUM_HTTP_ATOMS) because we expect to encounter a
    // few random headers right off the bat.
    if (!PL_DHashTableInit(&sAtomTable, &ops, nsnull, sizeof(PLDHashEntryStub),
                           NUM_HTTP_ATOMS + 10)) {
        sAtomTable.ops = nsnull;
        return NS_ERROR_OUT_OF_MEMORY;
    }

    // fill the table with our known atoms
    const char *const atoms[] = {
#define HTTP_ATOM(_name, _value) nsHttp::_name._val,
#include "nsHttpAtomList.h"
#undef HTTP_ATOM
        nsnull
    };

    for (int i = 0; atoms[i]; ++i) {
        PLDHashEntryStub *stub = NS_REINTERPRET_CAST(PLDHashEntryStub *,
                PL_DHashTableOperate(&sAtomTable, atoms[i], PL_DHASH_ADD));
        if (!stub)
            return NS_ERROR_OUT_OF_MEMORY;
        
        NS_ASSERTION(!stub->key, "duplicate static atom");
        stub->key = atoms[i];
    }

    return NS_OK;
}

void
nsHttp::DestroyAtomTable()
{
    if (sAtomTable.ops) {
        PL_DHashTableFinish(&sAtomTable);
        sAtomTable.ops = nsnull;
    }

    while (sHeapAtoms) {
        HttpHeapAtom *next = sHeapAtoms->next;
        free(sHeapAtoms);
        sHeapAtoms = next;
    }

    if (sLock) {
        PR_DestroyLock(sLock);
        sLock = nsnull;
    }
}

// this function may be called from multiple threads
nsHttpAtom
nsHttp::ResolveAtom(const char *str)
{
    nsHttpAtom atom = { nsnull };

    if (!str || !sAtomTable.ops)
        return atom;

    nsAutoLock lock(sLock);

    PLDHashEntryStub *stub = NS_REINTERPRET_CAST(PLDHashEntryStub *,
            PL_DHashTableOperate(&sAtomTable, str, PL_DHASH_ADD));
    if (!stub)
        return atom;  // out of memory

    if (stub->key) {
        atom._val = NS_REINTERPRET_CAST(const char *, stub->key);
        return atom;
    }

    // if the atom could not be found in the atom table, then we'll go
    // and allocate a new atom on the heap.
    HttpHeapAtom *heapAtom = NewHeapAtom(str);
    if (!heapAtom)
        return atom;  // out of memory

    stub->key = atom._val = heapAtom->value;
    return atom;
}

PRBool
nsHttp::ParseInt64(const char *input, const char **next, PRInt64 *r)
{
    const char *start = input;
    *r = 0;
    while (*input >= '0' && *input <= '9') {
        PRInt64 next = 10 * (*r) + (*input - '0');
        if (next < *r) // overflow?
            return PR_FALSE;
        *r = next;
        ++input;
    }
    if (input == start) // nothing parsed?
        return PR_FALSE;
    if (next)
        *next = input;
    return PR_TRUE;
}
