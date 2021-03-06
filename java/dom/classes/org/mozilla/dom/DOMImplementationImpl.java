/* 
 The contents of this file are subject to the Mozilla Public
 License Version 1.1 (the "License"); you may not use this file
 except in compliance with the License. You may obtain a copy of
 the License at http://www.mozilla.org/MPL/

 Software distributed under the License is distributed on an "AS
 IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 implied. See the License for the specific language governing
 rights and limitations under the License.

 The Original Code is mozilla.org code.

 The Initial Developer of the Original Code is Sun Microsystems,
 Inc. Portions created by Sun are
 Copyright (C) 1999 Sun Microsystems, Inc. All
 Rights Reserved.

 Contributor(s): 
*/

package org.mozilla.dom;

import org.w3c.dom.DOMImplementation;
import org.w3c.dom.Document;
import org.w3c.dom.DocumentType;
import org.w3c.dom.DOMException;

public class DOMImplementationImpl implements DOMImplementation {

    private long p_nsIDOMDOMImplementation = 0;

    // instantiated from JNI only
    private DOMImplementationImpl() {}

    public boolean equals(Object o) {
	if (!(o instanceof NodeListImpl))
	    return false;
	else
	    return (XPCOM_equals(o));
    }

    public int hashCode(){
	return XPCOM_hashCode();
    }

    public native boolean hasFeature(String feature, String version);

    protected native void finalize();

    private native boolean XPCOM_equals(Object o);
    private native int XPCOM_hashCode();

    //since DOM2
    public native DocumentType createDocumentType(String qualifiedName, 
						  String publicID, 
						  String systemID);

    public native Document createDocument(String namespaceURI,
					  String qualifiedName,
					  DocumentType doctype);
}
