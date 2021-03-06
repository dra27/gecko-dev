/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * Copyright (C) 1999 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */
package netscape.ldap.beans;

import java.beans.SimpleBeanInfo;
import java.beans.BeanDescriptor;
import java.beans.EventSetDescriptor;
import java.beans.MethodDescriptor;
import java.beans.PropertyDescriptor;
import java.beans.ParameterDescriptor;
import java.beans.BeanInfo;

/**
 * BeanInfo for displaying string
 *
 */

public class DisplayStringBeanInfo extends SimpleBeanInfo {

    public DisplayStringBeanInfo() throws Exception {

    beanClass = Class.forName( "netscape.ldap.beans.DisplayString" );

        // Publish descriptor ---------------------------------------------
        try {
            _beanDescriptor = new BeanDescriptor(beanClass);
			_beanDescriptor.setDisplayName( "Text Field" );
			_beanDescriptor.setShortDescription(
				"LDAP property retrieval -"
				+ " provided a host, port, base, search filter,"
				+ " and optionally a username and password,"
				+ " return an array of string values both as a"
				+ " function return and as a Property change event." );
        } catch (Exception e) {
        }
    }

    public BeanDescriptor getBeanDescriptor() {
		return _beanDescriptor;
	}

    private static Class beanClass;
	private BeanDescriptor _beanDescriptor;
    private EventSetDescriptor[] _eventSetDescriptor;
    private PropertyDescriptor[] _propertyDescriptor;
}


