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
* Contributor(s): pschwartau@netscape.com
* Date: 2001-07-02
*
* SUMMARY: Testing is operator on user-defined classes
*
*/
//-----------------------------------------------------------------------------
var UBound = 0;
var bug = '(none)';
var summary = 'Testing is operator on user-defined classes';
var cnYES = 'instance X is class X == true';
var cnNO = 'instance X is class X == false';
var cnRED = 'red';
var status = '';
var statusitems = [];
var actual = '';
var actualvalues = [];
var expect= '';
var expectedvalues = [];


class A
{
}

class B extends A
{
}

class C
{
  constructor function C ()
  {
    this.color = cnRED;
  }

} 

class D
{
  constructor function D()
  {
    this.color = cnRED;
  }

} 


var objA1 = new A;
var objA2 : A = new A;
var objA3 : A = (new A);

var objB1 = new B;
var objB2 : B = new B;
var objB3 : B = (new B);

var objC1 = new C;
var objC2 : C = new C;
var objC3 : C = (new C);

var objD1 = new D;
var objD2 : D = new D;
var objD3 : D = (new D);



status = 'Section A1 of test';
actual = objA1 is A;
expect = true;
addThis();

status = 'Section A2 of test';
actual = objA2 is A;
expect = true;
addThis();

status = 'Section A3 of test';
actual = objA3 is A;
expect = true;
addThis();


status = 'Section B1 of test';
actual = objB1 is B;
expect = true;
addThis();

status = 'Section B2 of test';
actual = objB2 is B;
expect = true;
addThis();

status = 'Section B3 of test';
actual = objB3 is B;
expect = true;
addThis();


status = 'Section C1 of test';
actual = objC1 is C;
expect = true;
addThis();

status = 'Section C2 of test';
actual = objC2 is C;
expect = true;
addThis();

status = 'Section C3 of test';
actual = objC3 is C;
expect = true;
addThis();


status = 'Section D1 of test';
actual = objD1 is D;
expect = true;
addThis();

status = 'Section D2 of test';
actual = objD2 is D;
expect = true;
addThis();

status = 'Section D3 of test';
actual = objD3 is D;
expect = true;
addThis();



//-----------------------------------------------------------------------------
test();
//-----------------------------------------------------------------------------



function addThis()
{
  statusitems[UBound] = status;
  actualvalues[UBound] = isOfClass(actual);
  expectedvalues[UBound] = isOfClass(expect);
  UBound++;
}


function test()
{
  enterFunc ('test');
  printBugNumber (bug);
  printStatus (summary);

  for (var i=0; i<UBound; i++)
  {
    reportCompare(expectedvalues[i], actualvalues[i], statusitems[i]);
  }

  exitFunc ('test');
}


function isOfClass(bAnswer)
{
  return bAnswer? cnYES : cnNO;
}
