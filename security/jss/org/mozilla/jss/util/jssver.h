/* ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is the Netscape security libraries.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#ifndef JSSVER_H
#define JSSVER_H

/*
 * JSS's major version, minor version, patch level, and whether
 * this is a beta release.
 *
 * The format of the version string should be
 *     "<major version>.<minor version>[.<patch level>] [<Beta>]"
 */

/********************************************************************/
/* The VERSION Strings should be updated in the following           */
/* files everytime a new release of JSS is generated:               */
/*                                                                  */
/* org/mozilla/jss/CryptoManager.java                               */
/* org/mozilla/jss/CryptoManager.c                                  */
/* org/mozilla/jss/util/jssver.h                                    */
/* lib/manifest.mn                                                  */
/*                                                                  */
/********************************************************************/

#define JSS_VERSION  "4.1"
#define JSS_VMAJOR   4
#define JSS_VMINOR   1
#define JSS_VPATCH   0
#define JSS_BETA     PR_FALSE

#endif
