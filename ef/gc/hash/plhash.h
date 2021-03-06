/* Insert copyright and license here 1997  
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

#ifndef plhash_h___
#define plhash_h___
/*
 * API to portable hash table code.
 */
#include <stddef.h>
#include <stdio.h>
#include "prtypes.h"

PR_BEGIN_EXTERN_C

typedef struct PLHashEntry  PLHashEntry;
typedef struct PLHashTable  PLHashTable;
typedef PRUint32 PLHashNumber;
#define PL_HASH_BITS 32
typedef PLHashNumber (PR_CALLBACK *PLHashFunction)(const void *key);
typedef PRIntn (PR_CALLBACK *PLHashComparator)(const void *v1, const void *v2);
typedef PRIntn (PR_CALLBACK *PLHashEnumerator)(PLHashEntry *he, PRIntn i, void *arg);

/* Flag bits in PLHashEnumerator's return value */
#define HT_ENUMERATE_NEXT       0       /* continue enumerating entries */
#define HT_ENUMERATE_STOP       1       /* stop enumerating entries */
#define HT_ENUMERATE_REMOVE     2       /* remove and free the current entry */
#define HT_ENUMERATE_UNHASH     4       /* just unhash the current entry */

typedef struct PLHashAllocOps {
    void *              (PR_CALLBACK *allocTable)(void *pool, PRSize size);
    void                (PR_CALLBACK *freeTable)(void *pool, void *item);
    PLHashEntry *       (PR_CALLBACK *allocEntry)(void *pool, const void *key);
    void                (PR_CALLBACK *freeEntry)(void *pool, PLHashEntry *he, PRUintn flag);
} PLHashAllocOps;

#define HT_FREE_VALUE   0               /* just free the entry's value */
#define HT_FREE_ENTRY   1               /* free value and entire entry */

struct PLHashEntry {
    PLHashEntry         *next;          /* hash chain linkage */
    PLHashNumber        keyHash;        /* key hash function result */
    const void          *key;           /* ptr to opaque key */
    void                *value;         /* ptr to opaque value */
};

struct PLHashTable {
    PLHashEntry         **buckets;      /* vector of hash buckets */
    PRUint32              nentries;       /* number of entries in table */
    PRUint32              shift;          /* multiplicative hash shift */
    PLHashFunction      keyHash;        /* key hash function */
    PLHashComparator    keyCompare;     /* key comparison function */
    PLHashComparator    valueCompare;   /* value comparison function */
    PLHashAllocOps      *allocOps;      /* allocation operations */
    void                *allocPriv;     /* allocation private data */
#ifdef HASHMETER
    PRUint32              nlookups;       /* total number of lookups */
    PRUint32              nsteps;         /* number of hash chains traversed */
    PRUint32              ngrows;         /* number of table expansions */
    PRUint32              nshrinks;       /* number of table contractions */
#endif
};

/*
 * Create a new hash table.
 * If allocOps is null, use default allocator ops built on top of malloc().
 */
PR_EXTERN(PLHashTable *)
PL_NewHashTable(PRUint32 n, PLHashFunction keyHash,
                PLHashComparator keyCompare, PLHashComparator valueCompare,
                PLHashAllocOps *allocOps, void *allocPriv);

PR_EXTERN(void)
PL_HashTableDestroy(PLHashTable *ht);

/*
 * Initialize an existing HashTable struct.
 */
PR_IMPLEMENT(PRStatus)
PL_HashTableInit(PLHashTable *ht, PRUint32 n, PLHashFunction keyHash,
                 PLHashComparator keyCompare, PLHashComparator valueCompare,
                 PLHashAllocOps *allocOps, void *allocPriv);

/*
 * Cleanup (destroy contents of) existing HashTable struct. 
 */
PR_IMPLEMENT(void)
PL_HashTableCleanup(PLHashTable *ht);

/* Low level access methods */
PR_EXTERN(PLHashEntry **)
PL_HashTableRawLookup(PLHashTable *ht, PLHashNumber keyHash, const void *key);

PR_EXTERN(PLHashEntry *)
PL_HashTableRawAdd(PLHashTable *ht, PLHashEntry **hep, PLHashNumber keyHash,
                   const void *key, void *value);

PR_EXTERN(void)
PL_HashTableRawRemove(PLHashTable *ht, PLHashEntry **hep, PLHashEntry *he);

/* Higher level access methods */
PR_EXTERN(PLHashEntry *)
PL_HashTableAdd(PLHashTable *ht, const void *key, void *value);

PR_EXTERN(PRBool)
PL_HashTableRemove(PLHashTable *ht, const void *key);

PR_EXTERN(PRIntn)
PL_HashTableEnumerateEntries(PLHashTable *ht, PLHashEnumerator f, void *arg);

PR_EXTERN(void *)
PL_HashTableLookup(PLHashTable *ht, const void *key);

PR_EXTERN(PRIntn)
PL_HashTableDump(PLHashTable *ht, PLHashEnumerator dump, FILE *fp);

/* General-purpose C string hash function. */
PR_EXTERN(PLHashNumber)
PL_HashString(const void *key);

/* Compare strings using strcmp(), return true if equal. */
PR_EXTERN(int)
PL_CompareStrings(const void *v1, const void *v2);

/* Stub function just returns v1 == v2 */
PR_EXTERN(PRIntn)
PL_CompareValues(const void *v1, const void *v2);

PR_END_EXTERN_C

#endif /* plhash_h___ */
