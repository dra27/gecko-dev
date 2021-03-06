/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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
/*******************************************************************************
 * Source date: 9 Apr 1997 21:45:12 GMT
 * netscape/fonts/cdoer module C stub file
 * Generated by jmc version 1.8 -- DO NOT EDIT
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "xp_mem.h"

/* Include the implementation-specific header: */
#include "Pcdoer.h"

/* Include other interface headers: */

/*******************************************************************************
 * cdoer Methods
 ******************************************************************************/

#ifndef OVERRIDE_cdoer_getInterface
JMC_PUBLIC_API(void*)
_cdoer_getInterface(struct cdoer* self, jint op, const JMCInterfaceID* iid, JMCException* *exc)
{
	if (memcmp(iid, &cdoer_ID, sizeof(JMCInterfaceID)) == 0)
		return cdoerImpl2cdoer(cdoer2cdoerImpl(self));
	return _cdoer_getBackwardCompatibleInterface(self, iid, exc);
}
#endif

#ifndef OVERRIDE_cdoer_addRef
JMC_PUBLIC_API(void)
_cdoer_addRef(struct cdoer* self, jint op, JMCException* *exc)
{
	cdoerImplHeader* impl = (cdoerImplHeader*)cdoer2cdoerImpl(self);
	impl->refcount++;
}
#endif

#ifndef OVERRIDE_cdoer_release
JMC_PUBLIC_API(void)
_cdoer_release(struct cdoer* self, jint op, JMCException* *exc)
{
	cdoerImplHeader* impl = (cdoerImplHeader*)cdoer2cdoerImpl(self);
	if (--impl->refcount == 0) {
		cdoer_finalize(self, exc);
	}
}
#endif

#ifndef OVERRIDE_cdoer_hashCode
JMC_PUBLIC_API(jint)
_cdoer_hashCode(struct cdoer* self, jint op, JMCException* *exc)
{
	return (jint)self;
}
#endif

#ifndef OVERRIDE_cdoer_equals
JMC_PUBLIC_API(jbool)
_cdoer_equals(struct cdoer* self, jint op, void* obj, JMCException* *exc)
{
	return self == obj;
}
#endif

#ifndef OVERRIDE_cdoer_clone
JMC_PUBLIC_API(void*)
_cdoer_clone(struct cdoer* self, jint op, JMCException* *exc)
{
	cdoerImpl* impl = cdoer2cdoerImpl(self);
	cdoerImpl* newImpl = (cdoerImpl*)malloc(sizeof(cdoerImpl));
	if (newImpl == NULL) return NULL;
	memcpy(newImpl, impl, sizeof(cdoerImpl));
	((cdoerImplHeader*)newImpl)->refcount = 1;
	return newImpl;
}
#endif

#ifndef OVERRIDE_cdoer_toString
JMC_PUBLIC_API(const char*)
_cdoer_toString(struct cdoer* self, jint op, JMCException* *exc)
{
	return NULL;
}
#endif

#ifndef OVERRIDE_cdoer_finalize
JMC_PUBLIC_API(void)
_cdoer_finalize(struct cdoer* self, jint op, JMCException* *exc)
{
	/* Override this method and add your own finalization here. */
	XP_FREEIF(self);
}
#endif

/*******************************************************************************
 * Jump Tables
 ******************************************************************************/

const struct cdoerInterface cdoerVtable = {
	_cdoer_getInterface,
	_cdoer_addRef,
	_cdoer_release,
	_cdoer_hashCode,
	_cdoer_equals,
	_cdoer_clone,
	_cdoer_toString,
	_cdoer_finalize,
	_cdoer_Update
};

/*******************************************************************************
 * Factory Operations
 ******************************************************************************/

JMC_PUBLIC_API(cdoer*)
cdoerFactory_Create(JMCException* *exception, nfFontObserverCallback a, void* b)
{
	cdoerImplHeader* impl = (cdoerImplHeader*)XP_NEW_ZAP(cdoerImpl);
	cdoer* self;
	if (impl == NULL) {
		JMC_EXCEPTION(exception, JMCEXCEPTION_OUT_OF_MEMORY);
		return NULL;
	}
	self = cdoerImpl2cdoer(impl);
	impl->vtablecdoer = &cdoerVtable;
	impl->refcount = 1;
	_cdoer_init(self, exception, a, b);
	if (JMC_EXCEPTION_RETURNED(exception)) {
		XP_FREE(impl);
		return NULL;
	}
	return self;
}

