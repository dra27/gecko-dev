/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is JSIRC Library.
 *
 * The Initial Developer of the Original Code is
 * New Dimensions Consulting, Inc.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Robert Ginda, rginda@ndcico.com, original author
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

var utils = new Object();

var DEBUG = true;
var dd, warn, TEST, ASSERT;

if (DEBUG) {
    var _dd_pfx = "";
    var _dd_singleIndent = "  ";
    var _dd_indentLength = _dd_singleIndent.length;
    var _dd_currentIndent = "";
    var _dd_lastDumpWasOpen = false;
    var _dd_timeStack = new Array();
    var _dd_disableDepth = Number.MAX_VALUE;
    var _dd_currentDepth = 0;
    dd = function _dd(str) {
             if (typeof str != "string") {
                 dump(str + "\n");
             } else if (str == "") {
                 dump("<empty-string>\n");
             } else if (str[str.length - 1] == "{") {
                 ++_dd_currentDepth;
                 if (_dd_currentDepth >= _dd_disableDepth)
                     return;
                 if (str.indexOf("OFF") == 0)
                     _dd_disableDepth = _dd_currentDepth;
                 _dd_timeStack.push (new Date());
                 if (_dd_lastDumpWasOpen)
                     dump("\n");
                 dump (_dd_pfx + _dd_currentIndent + str);
                 _dd_currentIndent += _dd_singleIndent;
                 _dd_lastDumpWasOpen = true;
             } else if (str[0] == "}") {
                 if (--_dd_currentDepth >= _dd_disableDepth)
                     return;
                 _dd_disableDepth = Number.MAX_VALUE;
                 var sufx = (new Date() - _dd_timeStack.pop()) / 1000 + " sec";
                 _dd_currentIndent =
                     _dd_currentIndent.substr(0, _dd_currentIndent.length -
                                              _dd_indentLength);
                 if (_dd_lastDumpWasOpen)
                     dump(str + " " + sufx + "\n");
                 else
                     dump(_dd_pfx + _dd_currentIndent + str + " " +
                          sufx + "\n");
                 _dd_lastDumpWasOpen = false;
             } else {
                 if (_dd_currentDepth >= _dd_disableDepth)
                     return;
                 if (_dd_lastDumpWasOpen)
                     dump("\n");
                 dump(_dd_pfx + _dd_currentIndent + str + "\n");
                 _dd_lastDumpWasOpen = false;
             }
         }
    warn = function (msg) { dd("** WARNING " + msg + " **"); }
    TEST = ASSERT = function _assert(expr, msg) {
                 if (!expr) {
                     var m = "** ASSERTION FAILED: " + msg + " **\n" +
                             getStackTrace();
                     try {
                         alert(m);
                     } catch(ex) {}
                     dd(m);
                     return false;
                 } else {
                     return true;
                 }
             }
} else {
    dd = warn = TEST = ASSERT = function (){};
}

var jsenv = new Object();
jsenv.HAS_SECURITYMANAGER = ((typeof netscape == "object") &&
                             (typeof netscape.security == "object"));
jsenv.HAS_XPCOM = ((typeof Components == "object") &&
                   (typeof Components.classes == "object"));
jsenv.HAS_JAVA = (typeof java == "object");
jsenv.HAS_RHINO = (typeof defineClass == "function");
jsenv.HAS_DOCUMENT = (typeof document == "object");
jsenv.HAS_NSPR_EVENTQ = jsenv.HAS_DOCUMENT;
jsenv.HAS_STREAM_PROVIDER = ("nsIStreamProvider" in Components.interfaces);
jsenv.HAS_SERVER_SOCKETS = ("nsIServerSocket" in Components.interfaces);

function dumpObject (o, pfx, sep)
{
    var p;
    var s = "";

    sep = (typeof sep == "undefined") ? " = " : sep;
    pfx = (typeof pfx == "undefined") ? "" : pfx;

    for (p in o)
    {
        if (typeof (o[p]) != "function")
            s += pfx + p + sep + o[p] + "\n";
        else
            s += pfx + p + sep + "function\n";
    }

    return s;

}

/* Dumps an object in tree format, recurse specifiec the the number of objects
 * to recurse, compress is a boolean that can uncompress (true) the output
 * format, and level is the number of levels to intitialy indent (only useful
 * internally.)  A sample dumpObjectTree (o, 1) is shown below.
 *
 * + parent (object)
 * + users (object)
 * | + jsbot (object)
 * | + mrjs (object)
 * | + nakkezzzz (object)
 * | *
 * + bans (object)
 * | *
 * + topic (string) 'ircclient.js:59: nothing is not defined'
 * + getUsersLength (function) 9 lines
 * *
 */
function dumpObjectTree (o, recurse, compress, level)
{
    var s = "";
    var pfx = "";

    if (typeof recurse == "undefined")
        recurse = 0;
    if (typeof level == "undefined")
        level = 0;
    if (typeof compress == "undefined")
        compress = true;

    for (var i = 0; i < level; i++)
        pfx += (compress) ? "| " : "|  ";

    var tee = (compress) ? "+ " : "+- ";

    for (i in o)
    {
        var t, ex;

        try
        {
            t = typeof o[i];
        }
        catch (ex)
        {
            t = "ERROR";
        }

        switch (t)
        {
            case "function":
                var sfunc = String(o[i]).split("\n");
                if (sfunc[2] == "    [native code]")
                    sfunc = "[native code]";
                else
                    if (sfunc.length == 1)
                        sfunc = String(sfunc);
                    else
                        sfunc = sfunc.length + " lines";
                s += pfx + tee + i + " (function) " + sfunc + "\n";
                break;

            case "object":
                s += pfx + tee + i + " (object)";
                if (o[i] == null)
                {
                    s += " null\n";
                    break;
                }

                s += "\n";

                if (!compress)
                    s += pfx + "|\n";
                if ((i != "parent") && (recurse))
                    s += dumpObjectTree (o[i], recurse - 1,
                                         compress, level + 1);
                break;

            case "string":
                if (o[i].length > 200)
                    s += pfx + tee + i + " (" + t + ") " +
                        o[i].length + " chars\n";
                else
                    s += pfx + tee + i + " (" + t + ") '" + o[i] + "'\n";
                break;

            case "ERROR":
                s += pfx + tee + i + " (" + t + ") ?\n";
                break;

            default:
                s += pfx + tee + i + " (" + t + ") " + o[i] + "\n";

        }

        if (!compress)
            s += pfx + "|\n";

    }

    s += pfx + "*\n";

    return s;

}

function ecmaEscape(str)
{
    function replaceNonPrintables(ch)
    {
        var rv = ch.charCodeAt().toString(16);
        if (rv.length == 1)
            rv = "0" + rv;
        else if (rv.length == 3)
            rv = "u0" + rv;
        else if (rv.length == 4)
            rv = "u" + rv;

        return "%" + rv;
    };

    // Replace any character that is not in the 69 character set
    // [ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@*_+-./]
    // with an escape sequence.  Two digit sequences in the form %XX are used
    // for characters whose codepoint is less than 255, %uXXXX for all others.
    // See section B.2.1 of ECMA-262 rev3 for more information.
    return str.replace(/[^A-Za-z0-9@*_+.\-\/]/g, replaceNonPrintables);
}

function ecmaUnescape(str)
{
    function replaceEscapes(seq)
    {
        var ary = seq.match(/([\da-f]{1,2})(.*)|u([\da-f]{1,4})/i);
        if (!ary)
            return "<ERROR>";

        var rv;
        if (ary[1])
        {
            // two digit escape, possibly with cruft after
            rv = String.fromCharCode(parseInt(ary[1], 16)) + ary[2];
        }
        else
        {
            // four digits, no cruft
            rv = String.fromCharCode(parseInt(ary[3], 16));
        }

        return rv;
    };

    // Replace the escape sequences %X, %XX, %uX, %uXX, %uXXX, and %uXXXX with
    // the characters they represent, where X is a hexadecimal digit.
    // See section B.2.2 of ECMA-262 rev3 for more information.
    return str.replace(/%u?([\da-f]{1,4})/ig, replaceEscapes);
}

function replaceVars(str, vars)
{
    // replace "string $with a $variable", with
    // "string " + vars["with"] + " with a " + vars["variable"]

    function doReplace(symbol)
    {
        var name = symbol.substr(1);
        if (name in vars)
            return vars[name];

        return "$" + name;
    };

    return str.replace(/(\$\w[\w\d\-]+)/g, doReplace);
}

function formatException (ex)
{
    if (ex instanceof Error)
        return getMsg (MSG_FMT_JSEXCEPTION, [ex.name, ex.message, ex.fileName,
                                             ex.lineNumber]);

    return String(ex);
}

/*
 * Clones an existing object (Only the enumerable properties
 * of course.) use as a function..
 * var c = Clone (obj);
 * or a constructor...
 * var c = new Clone (obj);
 */
function Clone (obj)
{
    var robj = new Object();

    for (var p in obj)
        robj[p] = obj[p];

    return robj;

}

function Copy(source, dest, overwrite)
{
    if (!dest)
        dest = new Object();

    for (var p in source)
    {
        if (overwrite || !(p in dest))
            dest[p] = source[p];
    }

    return dest;
}

/*
 * matches a real object against one or more pattern objects.
 * if you pass an array of pattern objects, |negate| controls wether to check
 * if the object matches ANY of the patterns, or NONE of the patterns.
 */
function matchObject (o, pattern, negate)
{
    negate = Boolean(negate);

    function _match (o, pattern)
    {
        if (pattern instanceof Function)
            return pattern(o);

        for (p in pattern)
        {
            var val;
                /* nice to have, but slow as molases, allows you to match
                 * properties of objects with obj$prop: "foo" syntax      */
                /*
                  if (p[0] == "$")
                  val = eval ("o." +
                  p.substr(1,p.length).replace (/\$/g, "."));
                  else
                */
            val = o[p];

            if (pattern[p] instanceof Function)
            {
                if (!pattern[p](val))
                    return false;
            }
            else
            {
                var ary = (new String(val)).match(pattern[p]);
                if (ary == null)
                    return false;
                else
                    o.matchresult = ary;
            }
        }

        return true;

    }

    if (!(pattern instanceof Array))
        return Boolean (negate ^ _match(o, pattern));

    for (var i in pattern)
        if (_match (o, pattern[i]))
            return !negate;

    return negate;

}

function utils_lcfn(text)
{
    return text.toLowerCase();
}

function matchEntry (partialName, list, lcFn)
{

    if ((typeof partialName == "undefined") ||
        (String(partialName) == ""))
    {
        var ary = new Array();
        for (var i in list)
            ary.push(i);
        return ary;
    }

    if (typeof lcFn != "function")
        lcFn = utils_lcfn;

    ary = new Array();

    for (i in list)
    {
        if (lcFn(list[i]).indexOf(lcFn(partialName)) == 0)
            ary.push(i);
    }

    return ary;

}

function encodeChar(ch)
{
   return "%" + ch.charCodeAt(0).toString(16);
}

function escapeFileName(fileName)
{
    return fileName.replace(/[^\w\d.,#\-_%]/g, encodeChar);
}

function getCommonPfx (list, lcFn)
{
    var pfx = list[0];
    var l = list.length;

    if (typeof lcFn != "function")
        lcFn = utils_lcfn;

    for (var i = 0; i < l; i++)
    {
        for (var c = 0; c < pfx.length; ++c)
        {
            if (c >= list[i].length)
            {
                pfx = pfx.substr(0, c);
                break;
            }
            else
            {
                if (lcFn(pfx[c]) != lcFn(list[i][c]))
                    pfx = pfx.substr(0, c);
            }
        }
    }

    return pfx;

}

function openTopWin (url)
{
    return openDialog (getBrowserURL(), "_blank", "chrome,all,dialog=no", url);
}

function getWindowByType (windowType)
{
    const MEDIATOR_CONTRACTID =
        "@mozilla.org/appshell/window-mediator;1";
    const nsIWindowMediator  = Components.interfaces.nsIWindowMediator;

    var windowManager =
        Components.classes[MEDIATOR_CONTRACTID].getService(nsIWindowMediator);

    return windowManager.getMostRecentWindow(windowType);
}

function renameProperty (obj, oldname, newname)
{

    if (oldname == newname)
        return;

    obj[newname] = obj[oldname];
    delete obj[oldname];

}

function newObject(contractID, iface)
{
    if (!jsenv.HAS_XPCOM)
        return null;

    var rv;
    var cls = Components.classes[contractID];
    
    if (!cls)
        return null;

    switch (typeof iface)
    {
        case "undefined":
            rv = cls.createInstance();
            break;

        case "string":
            rv = cls.createInstance(Components.interfaces[iface]);
            break;

        case "object":
            rv = cls.createInstance(iface);
            break;

        default:
            rv = null;
            break;
    }

    return rv;

}

function getService(contractID, iface)
{
    if (!jsenv.HAS_XPCOM)
        return null;

    var rv;
    var cls = Components.classes[contractID];
    
    if (!cls)
        return null;

    switch (typeof iface)
    {
        case "undefined":
            rv = cls.getService();
            break;

        case "string":
            rv = cls.getService(Components.interfaces[iface]);
            break;

        case "object":
            rv = cls.getService(iface);
            break;

        default:
            rv = null;
            break;
    }

    return rv;

}

function getContentWindow(frame)
{
    try
    {
        if (!frame || !("contentWindow" in frame))
            return false;

        return frame.contentWindow;
    }
    catch (ex)
    {
        // throws exception is contentWindow is gone
        return null;
    }
}

function getPriv (priv)
{
    if (!jsenv.HAS_SECURITYMANAGER)
        return true;

    var rv = true;

    try
    {
        netscape.security.PrivilegeManager.enablePrivilege(priv);
    }
    catch (e)
    {
        dd ("getPriv: unable to get privlege '" + priv + "': " + e);
        rv = false;
    }

    return rv;

}

function len(o)
{
    var l = 0;

    for (var p in o)
        ++l;

    return l;
}

function keys (o)
{
    var rv = new Array();

    for (var p in o)
        rv.push(p);

    return rv;

}

function stringTrim (s)
{
    if (!s)
        return "";
    s = s.replace (/^\s+/, "");
    return s.replace (/\s+$/, "");

}

/* the offset should be in seconds, it will be rounded to 2 decimal places */
function formatDateOffset (offset, format)
{
    var seconds = roundTo(offset % 60, 2);
    var minutes = Math.floor(offset / 60);
    var hours = Math.floor(minutes / 60);
    minutes = minutes % 60;
    var days = Math.floor(hours / 24);
    hours = hours % 24;

    if (!format)
    {
        var ary = new Array();
        if (days > 0)
            ary.push (getMsg(MSG_DAYS, days));
        if (hours > 0)
            ary.push (getMsg(MSG_HOURS, hours));
        if (minutes > 0)
            ary.push (getMsg(MSG_MINUTES, minutes));
        if (seconds > 0 || offset == 0)
            ary.push (getMsg(MSG_SECONDS, seconds));

        format = ary.join(", ");
    }
    else
    {
        format = format.replace ("%d", days);
        format = format.replace ("%h", hours);
        format = format.replace ("%m", minutes);
        format = format.replace ("%s", seconds);
    }

    return format;
}

function arrayHasElementAt(ary, i)
{
    return typeof ary[i] != "undefined";
}

function arrayContains (ary, elem)
{
    return (arrayIndexOf (ary, elem) != -1);
}

function arrayIndexOf (ary, elem)
{
    for (var i in ary)
        if (ary[i] == elem)
            return i;

    return -1;
}

function arrayInsertAt (ary, i, o)
{
    ary.splice (i, 0, o);
}

function arrayRemoveAt (ary, i)
{
    ary.splice (i, 1);
}

/* length should be an even number >= 6 */
function abbreviateWord (str, length)
{
    if (str.length <= length || length < 6)
        return str;

    var left = str.substr (0, (length / 2) - 1);
    var right = str.substr (str.length - (length / 2) + 1);

    return left + "..." + right;
}

/*
 * Inserts the string |hyphen| into string |str| every |pos| characters.
 * If there are any wordbreaking characters in |str| within -/+5 characters of
 * of a |pos| then the hyphen is inserted there instead, in order to produce a
 * "cleaner" break.
 */
function hyphenateWord (str, pos, hyphen)
{
    if (str.length <= pos)
        return str;
    if (typeof hyphen == "undefined")
        hyphen = " ";

    /* search for a nice place to break the word, fuzzfactor of +/-5, centered
     * around |pos| */
    var splitPos =
        str.substring(pos - 5, pos + 5).search(/[^A-Za-z0-9]/);

    splitPos = (splitPos != -1) ? pos - 4 + splitPos : pos;
    var left = str.substr (0, splitPos);
    var right = hyphenateWord(str.substr (splitPos), pos, hyphen);

    return left + hyphen + right;
}

/*
 * Like hyphenateWord, except individual chunks of the word are returned as
 * elements of an array.
 */
function splitLongWord (str, pos)
{
    if (str.length <= pos)
        return [str];

    var ary = new Array();
    var right = str;

    while (right.length > pos)
    {
        /* search for a nice place to break the word, fuzzfactor of +/-5,
         * centered around |pos| */
        var splitPos =
            right.substring(pos - 5, pos + 5).search(/[^A-Za-z0-9]/);

        splitPos = (splitPos != -1) ? pos - 4 + splitPos : pos;
        ary.push(right.substr (0, splitPos));
        right = right.substr (splitPos);
    }

    ary.push (right);

    return ary;
}

function getRandomElement (ary)
{

    return ary[Math.floor(Math.random() * ary.length)];

}

function roundTo (num, prec)
{

    return Math.round(num * Math.pow (10, prec)) / Math.pow (10, prec);

}

function randomRange (min, max)
{

    if (typeof min == "undefined")
        min = 0;

    if (typeof max == "undefined")
        max = 1;

    return Math.floor(Math.random() * (max - min + 1)) + min;

}

function getStackTrace ()
{

    if (!jsenv.HAS_XPCOM)
        return "No stack trace available.";

    var frame = Components.stack.caller;
    var str = "<top>";

    while (frame)
    {
        var name = frame.name ? frame.name : "[anonymous]";
        str += "\n" + name + "@" + frame.lineNumber;
        frame = frame.caller;
    }

    return str;

}

function getInterfaces (cls)
{
    if (!jsenv.HAS_XPCOM)
        return null;

    var rv = new Object();
    var e;

    for (var i in Components.interfaces)
    {
        try
        {
            var ifc = Components.interfaces[i];
            cls.QueryInterface(ifc);
            rv[i] = ifc;
        }
        catch (e)
        {
            /* nada */
        }
    }

    return rv;

}

/**
 * Calls a named function for each element in an array, sending
 * the same parameter each call.
 *
 * @param ary           an array of objects
 * @param func_name     string name of function to call.
 * @param data          data object to pass to each object.
 */
function mapObjFunc(ary, func_name, data)
{
    /*
     * WARNING: Caller assumes resonsibility to verify ary
     * and func_name
     */

    for (var i in ary)
        ary[i][func_name](data);
}

/**
 * Passes each element of an array to a given function object.
 *
 * @param func  a function object.
 * @param ary   an array of values.
 */
function map(func, ary) {

    /*
     * WARNING: Caller assumnes responsibility to verify
     * func and ary.
     */

    for (var i in ary)
        func(ary[i]);

}

function getSpecialDirectory(name)
{
    if (!("directoryService" in utils))
    {
        const DS_CTR = "@mozilla.org/file/directory_service;1";
        const nsIProperties = Components.interfaces.nsIProperties;

        utils.directoryService =
            Components.classes[DS_CTR].getService(nsIProperties);
    }

    return utils.directoryService.get(name, Components.interfaces.nsIFile);
}

function getFileFromURLSpec(url)
{
    const nsIFileProtocolHandler = Components.interfaces.nsIFileProtocolHandler;

    var service = getService("@mozilla.org/network/io-service;1", 
                             "nsIIOService");

    /* In sept 2002, bug 166792 moved this method to the nsIFileProtocolHandler
     * interface, but we need to support older versions too. */
    if ("getFileFromURLSpec" in service)
        return service.getFileFromURLSpec(url);

    /* In builds before 2002-08-15, there is no getFileFromURLSpec at all.
     * Instead, we have nsIIOservice.initFileFromURLSpec(nsIFile, string).
     */
    if ("initFileFromURLSpec" in service)
    {
        var file = newObject("@mozilla.org/file/local;1", "nsILocalFile");
        service.initFileFromURLSpec(file, url);
        return file;
    }

    var handler = service.getProtocolHandler("file");
    handler = handler.QueryInterface(nsIFileProtocolHandler);
    return handler.getFileFromURLSpec(url);
}

function getURLSpecFromFile (file)
{
    if (!file)
        return null;

    const IOS_CTRID = "@mozilla.org/network/io-service;1";
    const LOCALFILE_CTRID = "@mozilla.org/file/local;1";

    const nsIIOService = Components.interfaces.nsIIOService;
    const nsILocalFile = Components.interfaces.nsILocalFile;

    if (typeof file == "string")
    {
        var fileObj =
            Components.classes[LOCALFILE_CTRID].createInstance(nsILocalFile);
        fileObj.initWithPath(file);
        file = fileObj;
    }

    var service = Components.classes[IOS_CTRID].getService(nsIIOService);
    /* In sept 2002, bug 166792 moved this method to the nsIFileProtocolHandler
     * interface, but we need to support older versions too. */
    if ("getURLSpecFromFile" in service)
        return service.getURLSpecFromFile(file);

    var nsIFileProtocolHandler = Components.interfaces.nsIFileProtocolHandler;
    var fileHandler = service.getProtocolHandler("file");
    fileHandler = fileHandler.QueryInterface(nsIFileProtocolHandler);
    return fileHandler.getURLSpecFromFile(file);
}

function alert(msg, parent, title)
{
    var PROMPT_CTRID = "@mozilla.org/embedcomp/prompt-service;1";
    var nsIPromptService = Components.interfaces.nsIPromptService;
    var ps = Components.classes[PROMPT_CTRID].getService(nsIPromptService);
    if (!parent)
        parent = window;
    if (!title)
        title = MSG_ALERT;
    ps.alert (parent, title, msg);
}

function confirm(msg, parent, title)
{
    var PROMPT_CTRID = "@mozilla.org/embedcomp/prompt-service;1";
    var nsIPromptService = Components.interfaces.nsIPromptService;
    var ps = Components.classes[PROMPT_CTRID].getService(nsIPromptService);
    if (!parent)
        parent = window;
    if (!title)
        title = MSG_CONFIRM;
    return ps.confirm (parent, title, msg);
}

function confirmEx(msg, buttons, defaultButton, checkText, 
                   checkVal, parent, title)
{
    /* Note that on versions before Mozilla 0.9, using 3 buttons,
     * the revert or dontsave button, or custom button titles will NOT work.
     *
     * The buttons should be listed in the 'accept', 'cancel' and 'extra' order,
     * and the exact button order is host app- and platform-dependant.
     * For example, on Windows this is usually [button 1] [button 3] [button 2],
     * and on Linux [button 3] [button 2] [button 1]. 
     */
    var PROMPT_CTRID = "@mozilla.org/embedcomp/prompt-service;1";
    var nsIPromptService = Components.interfaces.nsIPromptService;
    var ps = Components.classes[PROMPT_CTRID].getService(nsIPromptService);

    var buttonConstants = {
        ok: ps.BUTTON_TITLE_OK,
        cancel: ps.BUTTON_TITLE_CANCEL,
        yes: ps.BUTTON_TITLE_YES,
        no: ps.BUTTON_TITLE_NO,
        save: ps.BUTTON_TITLE_SAVE,
        revert: ps.BUTTON_TITLE_REVERT,
        dontsave: ps.BUTTON_TITLE_DONT_SAVE
    };
    var buttonFlags = 0;
    var buttonText = [null, null, null];

    if (!isinstance(buttons, Array)) 
        throw "buttons parameter must be an Array";
    if ((buttons.length < 1) || (buttons.length > 3))
        throw "the buttons array must have 1, 2 or 3 elements";

    for (var i = 0; i < buttons.length; i++)
    {
        var buttonFlag = ps.BUTTON_TITLE_IS_STRING;
        if ((buttons[i][0] == "!") && (buttons[i].substr(1) in buttonConstants))
            buttonFlag = buttonConstants[buttons[i].substr(1)];
        else
            buttonText[i] = buttons[i];

        buttonFlags += ps["BUTTON_POS_" + i] * buttonFlag;
    }

    // ignore anything but a proper number
    var defaultIsNumber = (typeof defaultButton == "number");
    if (defaultIsNumber && arrayHasElementAt(buttons, defaultButton))
        buttonFlags += ps["BUTTON_POS_" + defaultButton + "_DEFAULT"];

    if (!parent)
        parent = window;
    if (!title)
        title = MSG_CONFIRM;
    if (!checkVal)
        checkVal = new Object();

    rv = ps.confirmEx(parent, title, msg, buttonFlags, buttonText[0], 
                      buttonText[1], buttonText[2], checkText, checkVal);
    return rv;
}

function prompt(msg, initial, parent, title)
{
    var PROMPT_CTRID = "@mozilla.org/embedcomp/prompt-service;1";
    var nsIPromptService = Components.interfaces.nsIPromptService;
    var ps = Components.classes[PROMPT_CTRID].getService(nsIPromptService);
    if (!parent)
        parent = window;
    if (!title)
        title = MSG_PROMPT;
    rv = { value: initial };

    if (!ps.prompt (parent, title, msg, rv, null, {value: null}))
        return null;

    return rv.value;
}

function promptPassword(msg, initial, parent, title)
{
    var PROMPT_CTRID = "@mozilla.org/embedcomp/prompt-service;1";
    var nsIPromptService = Components.interfaces.nsIPromptService;
    var ps = Components.classes[PROMPT_CTRID].getService(nsIPromptService);
    if (!parent)
        parent = window;
    if (!title)
        title = MSG_PROMPT;
    rv = { value: initial };

    if (!ps.promptPassword (parent, title, msg, rv, null, {value: null}))
        return null;

    return rv.value;
}

function viewCert(cert, parent)
{
    var cd = getService("@mozilla.org/nsCertificateDialogs;1",
                        "nsICertificateDialogs");
    if (!parent)
        parent = window;
    cd.viewCert(parent, cert);
}

function getHostmaskParts(hostmask)
{
    var rv;
    // A bit cheeky this, we try the matches here, and then branch
    // according to the ones we like.
    var ary1 = hostmask.match(/(\S*)!(\S*)@(.*)/);
    var ary2 = hostmask.match(/(\S*)@(.*)/);
    var ary3 = hostmask.match(/(\S*)!(.*)/);
    if (ary1)
        rv = { nick: ary1[1],  user: ary1[2], host: ary1[3] };
    else if (ary2)
        rv = { nick: "*",      user: ary2[1], host: ary2[2] };
    else if (ary3)
        rv = { nick: ary3[1],  user: ary3[2], host: "*"     };
    else
        rv = { nick: hostmask, user: "*",     host: "*"     };
    // Make sure we got something for all fields.
    if (!rv.nick)
        rv.nick = "*";
    if (!rv.user)
        rv.user = "*";
    if (!rv.host)
        rv.host = "*";
    // And re-construct the 'parsed' hostmask.
    rv.mask = rv.nick + "!" + rv.user + "@" + rv.host;
    return rv;
}

function isinstance(inst, base)
{
    /* Returns |true| if |inst| was constructed by |base|. Not 100% accurate,
     * but plenty good enough for us. This is to work around the fix for bug
     * 254067 which makes instanceof fail if the two sides are 'from'
     * different windows (something we don't care about).
     */
    return (inst && inst.constructor && base &&
            (inst.constructor.name == base.name));
}
