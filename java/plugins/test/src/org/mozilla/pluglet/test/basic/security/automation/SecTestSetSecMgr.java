/* 
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
 */

package org.mozilla.pluglet.test.basic.security.automation;

import org.mozilla.pluglet.test.basic.*;
import java.security.*;

public class SecTestSetSecMgr implements Test {

private String description = " setSecurityManager";
private boolean mustPass;

public void doAction() {
        ASSSM1 sm = new ASSSM1();
        System.setSecurityManager( sm );

	// if we got here then try it out...
	sm.printClassDepth("ASSSM2");		// -1
	sm.printClassDepth("ASSSM1");		//  0
	sm.printClassDepth("AppSecSetSecMgr");	//  1

	ASSSM2.printClassDepth("ASSSM2");		//  1
	ASSSM2.printClassDepth("ASSSM1");		//  0
	ASSSM2.printClassDepth("AppSecSetSecMgr");	//  2

}
	
public void execute( TestContext c ) {
 
 mustPass = false;

 if (c.getProperty("SecTestSetSecMgr.mustPass").equals( new String("true") )) {
	mustPass = true;
 };

 try {
 	doAction();
     if( mustPass )	
		c.registerPASSED(new String("OK")); else
		c.registerFAILED(new String(description) + new String(" failed."));

 } catch ( SecurityException e ) {
     if( mustPass )	
		c.registerFAILED(e.toString()); else
		c.registerPASSED(e.toString());
 } catch ( Exception e ) {
	c.registerFAILED(e.toString());
 }
 }
}


class ASSSM1 extends SecurityManager
{
    public void printClassDepth( String name )
    {
	System.out.println( classDepth( name ) );
    }
}

class ASSSM2
{
    static void printClassDepth( String name )
    {
	((ASSSM1) System.getSecurityManager()).printClassDepth( name );
    }
}

