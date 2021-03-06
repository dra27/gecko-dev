/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

#ifndef nsIDeviceContextSpec_h___
#define nsIDeviceContextSpec_h___

#include "nsIDeviceContext.h"

#define NS_IDEVICE_CONTEXT_SPEC_IID   \
{ 0x9951f910, 0x78d7, 0x11d2, \
{ 0xa8, 0x46, 0x00, 0x40, 0x95, 0x9a, 0x28, 0xc9 } }

class nsIDeviceContextSpec : public nsISupports
{
public:
   NS_DEFINE_STATIC_IID_ACCESSOR(NS_IDEVICE_CONTEXT_SPEC_IID)
};

#endif
