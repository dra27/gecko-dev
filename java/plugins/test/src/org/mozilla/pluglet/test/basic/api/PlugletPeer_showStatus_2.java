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


package org.mozilla.pluglet.test.basic.api;

import java.util.*;
import java.io.*;
import java.net.*;
import java.lang.Thread;
import org.mozilla.pluglet.test.basic.*;
import org.mozilla.pluglet.mozilla.*;
import org.mozilla.pluglet.*;

public class PlugletPeer_showStatus_2  implements Test
{
   private static String curStatus = null;
   public static boolean actionPerformed = false;
   /**
    *
    ***********************************************************
    *  Constructor
    ***********************************************************
    *
    */
   public PlugletPeer_showStatus_2()
   {
   }


   /**
    *
    ***********************************************************
    *  Starting point of application
    *
    *  @param   args    Array of command line arguments
    *
    ***********************************************************
    */
   public static void main(String[] args)
   {
   }

   /**
    ***********************************************************
    *
    *  Execute Method 
    *
    *  @param   contex  TestContext reference with properties and parameters    
    *  @return          void, the result set to true or false via context
    *
    ***********************************************************
    */
   public void execute(TestContext context)
   {	
	Properties paramProps = null;
	PlugletPeer PlugletPeer_obj = null;
	
	if ( context == null ) {
	   TestContext.registerFAILED("ERROR:null context passed in execute(TestContext)" +
				      " of PlugletPeer_showStatus_2");
	   return;
	}
	if ( context.failedIsRegistered()) {
	   return;
	}
	Vector vectorOfParameters = context.getParameters();
	PlugletPeer_obj = context.getPlugletPeer();
	if (PlugletPeer_obj == null) {
	   TestContext.registerFAILED("ERROR:null PlugletPeer passed in execute(TestContext)" +
				      " of PlugletPeer_showStatus_2");
	   return;
	} 
	System.err.println("DEBUG: Void method PlugletPeer_showStatus(" + 
			   (String)vectorOfParameters.elementAt(0) + ")");
	curStatus = (String)vectorOfParameters.elementAt(0);
	PlugletPeer_obj.showStatus(curStatus);
	actionPerformed = true;
   }

   public static void verifyStatus(String realStatus) {
     if (curStatus == null) {
        TestContext.registerPASSED("Pluglet doesn't crashed when null status was trasfered");
	return;
     }
     if (curStatus.equals(realStatus)) {
        TestContext.registerPASSED("\"" + realStatus  + "\" value passed from JavaScript");
     } else {
        TestContext.registerFAILED("\"" + realStatus  + "\" value passed from JavaScript" +
				   " instead of \"" + curStatus + "\"");
     }
   }
   /**
    *
    ***********************************************************
    *  Routine where OS specific checks are made. 
    *
    *  @param   os      OS Name (SunOS/Linus/MacOS/...)
    ***********************************************************
    *
    */
   private void osRoutine(String os)
   {
     if(os == null) return;

     os = os.trim();
     if(os.compareTo("SunOS") == 0) {}
     else if(os.compareTo("Linux") == 0) {}
     else if(os.compareTo("Windows") == 0) {}
     else if(os.compareTo("MacOS") == 0) {}
     else {}
   }
}
