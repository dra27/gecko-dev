/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Sun Microsystems,
 * Inc. Portions created by Sun are
 * Copyright (C) 1999 Sun Microsystems, Inc. All
 * Rights Reserved.
 *
 * Contributor(s):
 * Igor Kushnirskiy <idk@eng.sun.com>
 */
#ifndef  __bcIIDJava_H
#define  __bcIIDJava_H 

#include "nsIID.h"
#include "jni.h"

class   bcIIDJava {
 public:
    static jobject GetObject(nsIID * iid);
    static nsIID GetIID(jobject obj);
    static jclass GetClass();
 private:
    static jclass iidClass;
    static jmethodID iidInitMID;
    static jmethodID getStringMID;
    static void Init(void);
    static void Destroy(void);
};

#endif
