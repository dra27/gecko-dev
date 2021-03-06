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

#ifndef nsDeviceContextSpecWin_h___
#define nsDeviceContextSpecWin_h___

#include "nsIDeviceContextSpec.h"
#include <windows.h>

class nsDeviceContextSpecWin : public nsIDeviceContextSpec
{
public:
  nsDeviceContextSpecWin();

  NS_DECL_ISUPPORTS

  NS_IMETHOD Init(char *aDriverName, char *aDeviceName, HGLOBAL aDEVMODE);
  NS_IMETHOD GetDriverName(char *&aDriverName) const;
  NS_IMETHOD GetDeviceName(char *&aDeviceName) const;
  NS_IMETHOD GetDEVMODE(HGLOBAL &aDevMode) const;

protected:
  virtual ~nsDeviceContextSpecWin();

  char    *mDriverName;
  char    *mDeviceName;
  HGLOBAL mDEVMODE;
};

#endif
