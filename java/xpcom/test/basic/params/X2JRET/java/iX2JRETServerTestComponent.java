/*
 * ************* DO NOT EDIT THIS FILE ***********
 *
 * This file was automatically generated from iX2JRETServerTestComponent.idl.
 */


package org.mozilla.xpcom;


/**
 * Interface iX2JRETServerTestComponent
 *
 * IID: 0xb7dafef0-a110-11d4-9d3d-00a024a8bb88
 */

public interface iX2JRETServerTestComponent extends nsISupports
{
    public static final IID IID =
       new IID("b7dafef0-a110-11d4-9d3d-00a024a8bb88");


    /* void setTestLocation (in string testLocation, in string logLocation); */
    public void setTestLocation(String testLocation, String logLocation);

    /* void flush (in string name); */
    public void flush(String name);

    /* short testShort (); */
    public short testShort();

    /* long testLong (); */
    public int testLong();

    /* long long testLonglong (); */
    public long testLonglong();

    /* octet testByte (); */
    public byte testByte();

    /* unsigned short testUShort (); */
    public short testUShort();

    /* unsigned long testULong (); */
    public int testULong();

    /* unsigned long long testULonglong (); */
    public long testULonglong();

    /* float testFloat (); */
    public float testFloat();

    /* double testDouble (); */
    public double testDouble();

    /* boolean testBoolean (); */
    public boolean testBoolean();

    /* char testChar (); */
    public char testChar();

    /* wchar testWChar (); */
    public char testWChar();

    /* string testString (); */
    public String testString();

    /* wstring testWString (); */
    public String testWString();

    /* void testStringArray (in unsigned long count, [array, size_is (count), retval] out string stringArray); */
    public String[] testStringArray(int count);

    /* void testLongArray (in unsigned long count, [array, size_is (count), retval] out long intArray); */
    public int[] testLongArray(int count);

    /* void testCharArray (in unsigned long count, [array, size_is (count), retval] out char charArray); */
    public char[] testCharArray(int count);

    /* iX2JRETServerTestComponent testObject (); */
    public iX2JRETServerTestComponent testObject();

    /* void testObj (); */
    public void testObj();

}

/*
 * end
 */
