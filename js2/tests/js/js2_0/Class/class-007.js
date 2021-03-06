/*
* The contents of this file are subject to the Netscape Public
* License Version 1.1 (the "License"); you may not use this file
* except in compliance with the License. You may obtain a copy of
* the License at http://www.mozilla.org/NPL/
*
* Software distributed under the License is distributed on an
* "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either expressed
* or implied. See the License for the specific language governing
* rights and limitations under the License.
*
* The Original Code is mozilla.org code.
*
* The Initial Developer of the Original Code is Netscape
* Communications Corporation.  Portions created by Netscape are
* Copyright (C) 1998 Netscape Communications Corporation.
* All Rights Reserved.
*
* Contributor(s): waldemar@netscape.com,pschwartau@netscape.com
* Date: 2001-06-25
*
* SUMMARY: Testing static members of classes
*
*/
//-------------------------------------------------------------------------------------------------
var UBound = 0;
var bug = '(none)';
var summary = 'Testing static members of classes';
var cnERROR = 'ERROR!!!';
var cnNOERROR = 'NO ERRROR WAS THROWN -';
var status = '';
var statusitems = [];
var actual = '';
var actualvalues = [];
var expect= '';
var expectedvalues = [];



class C
{
  static var v = "Cv";
  static var x = "Cx";
  static var y = "Cy";
  static var z = "Cz";
}

interface A
{
  static var x = "Ax";
  static var i = "Ai";
  static var j = "Aj";
}

interface B
{
  static var x = "Bx";
  static var y = "By";
  static var j = "Bj";
}

class D extends C implements A, B
{
  static var v = "Dv";
}


status = inSection(1);
actual = C.v;
expect = "Cv";
addThis();

status = inSection(2);
actual = C.x;
expect = "Cx";
addThis();

status = inSection(3);
actual = C.y;
expect = "Cy";
addThis();

status = inSection(4);
actual = C.z;
expect = "Cz";
addThis();

status = inSection(5);
actual = A.x;
expect = "Ax";
addThis();

status = inSection(6);
actual = B.y;
expect = "By";
addThis();

status = inSection(7);
actual = D.v;
expect = "Dv";
addThis();

status = inSection(8);
actual = D.x;
expect = "Cx"; //(superclass preferred over "Ax" or "Bx")
addThis();

status = inSection(9);
actual = D.y;
expect = "Cy"; //(superclass preferred over "By")
addThis();

status = inSection(10);
actual = D.z;
expect = "Cz";
addThis();

status = inSection(11);
actual = D.i;
expect = "Ai";
addThis();

status = inSection(12);
actual = cnNOERROR;
try
{
  D.j;
}
catch (e)
{
  actual = cnERROR;
}
expect = cnERROR; /// error because of ambiguity: "Aj" or "Bj"?
addThis();

status = inSection(13);
actual = D.A::j;
expect = "Aj";
addThis();

status = inSection(14);
actual = D.B::j;
expect = "Bj";
addThis();

status = inSection(15);
actual = D.A::x;
expect = "Ax";
addThis();

status = inSection(16);
actual = D.A::i;
expect = "Ai";
addThis();

status = inSection(17);
D.x = 5;
actual = C.x;
expect = 5; //(same variable)
addThis();

status = inSection(18);
C.v = 7;
actual = D.v;
expect = "Dv"; //(different variables)
addThis();

status = inSection(19);
actual = C.v;
expect = 7;
addThis();




//-------------------------------------------------------------------------------------------------
test();
//-------------------------------------------------------------------------------------------------


function addThis()
{
  statusitems[UBound] = status;
  actualvalues[UBound] = actual;
  expectedvalues[UBound] = expect;
  UBound++;
}


function test()
{
  enterFunc ('test');
  printBugNumber (bug);
  printStatus (summary);
 
  for (var i = 0; i < UBound; i++)
  {
    reportCompare(expectedvalues[i], actualvalues[i], statusitems[i]);
  }

  exitFunc ('test');
}


function getStatus(i)
{
  return statusitems[i];
}
