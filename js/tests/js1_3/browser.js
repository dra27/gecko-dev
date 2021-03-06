/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
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

/*
 * JavaScript test library shared functions file for running the tests
 * in the browser.  Overrides the shell's print function with document.write
 * and make everything HTML pretty.
 *
 * To run the tests in the browser, use the mkhtml.pl script to generate
 * html pages that include the shell.js, browser.js (this file), and the
 * test js file in script tags.
 *
 * The source of the page that is generated should look something like this:
 *      <script src="./../shell.js"></script>
 *      <script src="./../browser.js"></script>
 *      <script src="./mytest.js"></script>
 */

GLOBAL = "[object Window]";

function writeLineToLog( string ) {
  document.write( string + "<br>\n");
}
function writeHeaderToLog( string ) {
  document.write( "<h2>" + string + "</h2>" );
}
function writeFormattedResult( expect, actual, string, passed ) {
  var s = "<tt>"+ string ;
  s += "<b>" ;
  s += ( passed ) ? "<font color=#009900> &nbsp;" + PASSED
    : "<font color=#aa0000>&nbsp;" +  FAILED + expect + "</tt>";
  writeLineToLog( s + "</font></b></tt>" );
  return passed;
}


/*
 * The earlier versions of the test code used exceptions
 * to terminate the test script in "negative" test cases
 * before the failure reporting code could run. In order
 * to be able to capture errors for the "negative" case 
 * where the exception is a sign the test actually passed,
 * the err online handler will assume that any error is a 
 * failure unless gExceptionExpected is true.
 */
window.onerror = err;
var gExceptionExpected = false;

function err( msg, page, line ) {
  var testcase;

  if (typeof(EXPECTED) == "undefined" || EXPECTED != "error") {
    /*
     * an unexpected exception occured
     */
    writeLineToLog( "Test failed with the message: " + msg );
    testcase = new TestCase(SECTION, "unknown", "unknown", "unknown");
    testcase.passed = false;
    testcase.reason = "Error: " + msg + 
      " Source File: " + page + " Line: " + line + ".";
    return;
  }

  if (typeof SECTION == 'undefined')
  {
    SECTION = 'Unknown';
  }
  if (typeof DESCRIPTION == 'undefined')
  {
    DESCRIPTION = 'Unknown';
  }
  if (typeof EXPECTED == 'undefined')
  {
    EXPECTED = 'Unknown';
  }

  testcase = new TestCase(SECTION, DESCRIPTION, EXPECTED, "error");
  testcase.reason += "Error: " + msg + 
    " Source File: " + page + " Line: " + line + ".";
  stopTest();
}

function version(v)
{
  if (v) { 
    gVersion = v; 
  } 
  return gVersion; 
}
