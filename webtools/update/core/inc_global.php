<?php
// ***** BEGIN LICENSE BLOCK *****
// Version: MPL 1.1/GPL 2.0/LGPL 2.1
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// The Original Code is Mozilla Update.
//
// The Initial Developer of the Original Code is
// Chris "Wolf" Crews.
// Portions created by the Initial Developer are Copyright (C) 2004
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Chris "Wolf" Crews <psychoticwolf@carolina.rr.com>
//
// Alternatively, the contents of this file may be used under the terms of
// either the GNU General Public License Version 2 or later (the "GPL"), or
// the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
// in which case the provisions of the GPL or the LGPL are applicable instead
// of those above. If you wish to allow use of your version of this file only
// under the terms of either the GPL or the LGPL, and not to allow others to
// use your version of this file under the terms of the MPL, indicate your
// decision by deleting the provisions above and replace them with the notice
// and other provisions required by the GPL or the LGPL. If you do not delete
// the provisions above, a recipient may use your version of this file under
// the terms of any one of the MPL, the GPL or the LGPL.
//
// ***** END LICENSE BLOCK *****

//inc_global.php -- Stuff that needs to be done globally to all of Mozilla Update

//Cache Control Headers
if ($nocache==TRUE) {
  $expstr = gmdate("D, d M Y H:i:s", time() - 1800) . " GMT";
  header("Expires: $expstr");
  header("Cache-Control: public, max-age=0");
} else {
  $expstr = gmdate("D, d M Y H:i:s", time() + 1800) . " GMT";
  header("Expires: $expstr");
  header("Cache-Control: public, max-age=1800");
}

// ---------------------------
// escape_string() --  Quote a variable to make it safe
// ---------------------------
function escape_string($value)
{
  // Stripslashes if we need to
  if (get_magic_quotes_gpc()) {
    $value = stripslashes($value);
  }

  // Quote it if it's not an integer
  if (!is_numeric($value)) {
    $value = mysql_real_escape_string($value);
  }

  return $value;
}



//Remove HTML tags and escape enities from GET/POST vars.
foreach ($_GET as $key => $val) {
  $_GET["$key"] = htmlentities(str_replace("\\","",strip_tags($_GET["$key"])));
}

foreach ($_POST as $key => $val) {
  if (!is_array($_POST["$key"])) {
    $_POST["$key"] = htmlentities(str_replace("\\","",strip_tags($_POST["$key"])));
  }
}

// Bug 250596 Fixes for incoming $_GET variables.
if ($_GET["application"]) {
  $_GET["application"] = escape_string(strtolower($_GET["application"]));
  $sql = "SELECT AppID FROM  `applications`
          WHERE `AppName` = '".ucwords(strtolower($_GET["application"]))."' 
          LIMIT 1";
  $sql_result = mysql_query($sql, $connection) or trigger_error("MySQL Error ".mysql_errno().": ".mysql_error()."", E_USER_NOTICE);
  if (mysql_num_rows($sql_result)===0) {unset($_GET["application"]);}
}


if ($_GET["category"] AND $_GET["category"] !=="All"
AND $_GET["category"] !=="Editors Pick" AND $_GET["category"] !=="Popular"
AND $_GET["category"] !=="Top Rated" AND $_GET["category"] !=="Newest") {
  $sql = "SELECT CatName 
          FROM `categories` 
          WHERE `CatName` = '".escape_string(ucwords(strtolower($_GET["category"])))."' 
          LIMIT 1";
  $sql_result = mysql_query($sql, $connection) or trigger_error("MySQL Error ".mysql_errno().": ".mysql_error()."", E_USER_NOTICE);
  if (mysql_num_rows($sql_result)===0) {unset($_GET["category"]);}
}

if (!is_numeric($_GET["id"])) { unset($_GET["id"]); }
if (!is_numeric($_GET["vid"])) { unset($_GET["vid"]); }
if (!is_numeric($_GET["pageid"])) { unset($_GET["pageid"]); }
if (!is_numeric($_GET["numpg"])) { unset($_GET["numpg"]); }

// page_error() function

function page_error($reason, $custom_message) {
  global $page_header, $page_footer;

  echo"<TITLE>Mozilla Update :: Error</TITLE>\n";

  include"$page_header";
  echo"<div id=\"mBody\">";
  echo"<h1>Mozilla Update :: Error</h1>\n";
  echo"<SPAN style=\"font-size: 12pt\">\n";
  echo"Mozilla Update has encountered an error and is unable to fulfill your request. Please try your request again later. If the
       problem continues, please contact the Mozilla Update staff. More information about the error may be found at the end of this
       message.<BR><BR>
       Error $reason: $custom_message<BR><BR>
       &nbsp;&nbsp;&nbsp;<A HREF=\"javascript:history.back()\">&#171;&#171; Go Back to Previous Page</A>";
  echo"</SPAN>\n";
  echo"</div>\n";
  include"$page_footer";
  echo"</body>\n</html>\n";
  exit;
}

function writeFormKey()
{
  $sql = "SELECT UserPass FROM userprofiles WHERE UserID = '".$_SESSION["uid"]."'";
  $res = mysql_query($sql) or trigger_error("MySQL Error ".mysql_errno().": ".mysql_error()."", E_USER_NOTICE);
  $row = mysql_fetch_array($res);
  echo "<input type=\"hidden\" name=\"formkey\" value=\"".md5($row["UserPass"])."\">\n";
}

function checkFormKey()
{
  $key = $_POST["formkey"];
  $sql = "SELECT UserPass FROM userprofiles WHERE UserID = '".$_SESSION["uid"]."'";
  $res = mysql_query($sql) or trigger_error("MySQL Error ".mysql_errno().": ".mysql_error()."", E_USER_NOTICE);
  $row = mysql_fetch_array($res);
  if ($key != md5($row["UserPass"]))
  {
    echo "<br><font size=+2 color='red'>WARNING: FORMKEY MISMATCH!</font><br>\n";
    return false;
  }
  return true;
}

// -----------------------------
// function uriparams() -- print all the present and valid URI variables.
// Usage: string uriparams()
// -----------------------------

function uriparams() {
  global $app_version, $application, $items_per_page, $category, $OS, $uriparams_skip;
  $uriparams = "";

  if ($application and $uriparams_skip !="application") { $uriparams .="application=$application&amp;"; }
  if ($app_version and $uriparams_skip !="application") { $uriparams .="version=$app_version&amp;"; }
  if ($OS) { $uriparams .="os=$OS&amp;"; }
  if ($category and $uriparams_skip !="category") { $uriparams .="category=$category&amp;"; }
  if ($items_per_page) { $uriparams .="numpg=$items_per_page"; }
  if (substr($uriparams, -1)==";") {
    $uriparams = substr($uriparams,0,strlen($uriparams)-5);
  }
  return $uriparams;
}

// -----------------------------
// function installtrigger() -- print installtrigger function for extension/theme installation on page.
// Usage null uriparams(string functionname)
// -----------------------------

function installtrigger($functionname) {
  if ($functionname=="extensions") {
    echo'
        <script type="text/javascript">
        <!--

        function install( aEvent, extName, iconURL)  {   
            var p = new XMLHttpRequest();
            p.open("GET", "'.$sitehostname.'/core/install.php?uri="+aEvent.target.href, false);
            p.send(null);

            var params = new Array();
            params[extName] = {
                URL: aEvent.target.href,
                IconURL: iconURL,
                toString: function () { return this.URL; }
            };
            InstallTrigger.install(params);
            return false;
        }

        -->
        </script>
    ';


  } else if ($functionname=="themes") {
    echo'
        <script type="text/javascript">
        <!--
            function installTheme( aEvent, extName) {
                var p = new XMLHttpRequest();
                p.open("GET", "/core/install.php?uri="+aEvent.target.href, false);
                p.send(null);

                InstallTrigger.installChrome(InstallTrigger.SKIN,aEvent.target.href,extName);
                return false;
            }
        -->
        </script>
    ';
  }
}

/**
   mozupd_buildDownloadlURL function
   builds the URL for extensions/themes download
   in the form /core/install.php/filename.$ext?passthrough=yes&uri=$uri
   performing entities escaping as per W3C specification 
   
   @param string $uri the 'real' URI of the file
   @param string $name file name
   @param string $version file version
   @param string $ext suggested file extension, including leading '.'
   @param boolean $force should we force passed extension?
   
   @author: Giorgio Maone <g.maone at informaction dot com>
   @version: 0.1
*/
function mozupd_buildDownloadURL($uri, $name, $version, 
                                    $ext='.xpi', $force=FALSE) {
                                      
  if(preg_match('/.*\/(.*?)(\.[a-z]+)(\?|$)/i',$uri,$uri_parts) // uri parsing
     && strcasecmp($autoext=$uri_parts[2],$ext)==0 // extension exact matching
      || (!$force // autodetection for a set of reasonable download extensions
          && preg_match('/^\.(jar|xpi|zip|exe|gz[\w]+|bz[\w+]|rpm)$/i',$autoext)
       )
  ) {
    $filename=$uri_parts[1].$uri_parts[2];
  } else { // fall back if $uri has not a recognized extension
    $filename=preg_replace('/\W/','_',"$name $version").$ext;
  }
 
  return htmlspecialchars( // if we don't escape '&' and friends validator cries
    "/core/install.php/$filename?passthrough=yes&uri=$uri");
}
?>
