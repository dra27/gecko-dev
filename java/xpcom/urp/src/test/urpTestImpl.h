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
 * Leila.Garin@eng.sun.com
 * Igor Kushnirskiy <idk@eng.sun.com>
 */
#ifndef __urpTestImpl_h
#define __urpTestImpl_h
#include "urpITest.h"

class urpTestImpl : public  urpITest {
    NS_DECL_ISUPPORTS 
    NS_DECL_URPITEST
    urpTestImpl();
    virtual ~urpTestImpl();
    /*void test1(inout long l);*/
//    NS_IMETHOD Test1(PRInt32 *l);
    /*void test2(in long l1,inout long l2);*/
//    NS_IMETHOD Test2(PRInt32 l1, PRInt32* l2);
    /*void test3(in string s1,inout string s2);*/
//    NS_IMETHOD Test3(const char *s1, char **s2);
    /*void test4(in PRUint32 count,[array, size_is(count)] in string valueArray);*/
//    NS_IMETHOD Test4(PRUint32 count, const char **valueArray);
    /*void test5(in PRUint32 count,[array, size_is(count)] inout string valueArray);*/
//    NS_IMETHOD Test5(PRUint32 count, char ***valueArray);
    /*void test6(in urpITest o);*/
//    NS_IMETHOD Test6(class urpITest *o);
    /* void test7 (out urpITest o); */
//    NS_IMETHOD Test7(urpITest **o);
};

#endif /* __urpTestImpl_h */


