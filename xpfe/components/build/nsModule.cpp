/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#include "nsIGenericFactory.h"
#include "nsICategoryManager.h"
#include "rdf.h"
#include "nsXPIDLString.h"
#include "nsCharsetMenu.h"
#include "nsDirectoryViewer.h"
#include "nsFontPackageHandler.h"
#include "nsWindowDataSource.h"
#include "nsRDFCID.h"

#if !defined(MOZ_PHOENIX) && !defined(MOZ_XULRUNNER)
#include "nsAutoComplete.h"
#endif

#if !defined(MOZ_PHOENIX) && !defined(MOZ_XULRUNNER) && !defined(MOZ_MACBROWSER)
#include "nsBookmarksService.h"
#include "nsRelatedLinksHandlerImpl.h"
#include "nsGlobalHistory.h"
#include "nsDocShellCID.h"
#include "nsDownloadManager.h"
#include "nsDownloadProxy.h"
#include "nsAppStartup.h"
#include "nsCommandLineService.h"
#include "nsUserInfo.h"

#if defined(MOZ_LDAP_XPCOM)
#include "nsLDAPAutoCompleteSession.h"
#endif // defined(MOZ_LDAP_XPCOM)

#endif // !defined(MOZ_PHOENIX) && !defined(MOZ_XULRUNNER) && !defined(MOZ_MACBROWSER)

#if defined(XP_WIN)
#if !defined(MOZ_PHOENIX) && !defined(MOZ_XULRUNNER)
#include "nsWindowsHooks.h"
#include "nsAlertsService.h"
#include "nsUrlWidget.h"
#endif
#endif // Windows

#include "nsBrowserStatusFilter.h"
#include "nsBrowserInstance.h"
#include "nsCURILoader.h"
#include "nsXPFEComponentsCID.h"

// {9491C382-E3C4-11D2-BDBE-0050040A9B44}
#define NS_GLOBALHISTORY_CID \
{ 0x9491c382, 0xe3c4, 0x11d2, { 0xbd, 0xbe, 0x0, 0x50, 0x4, 0xa, 0x9b, 0x44} }

#define NS_GLOBALHISTORY_DATASOURCE_CONTRACTID \
    "@mozilla.org/rdf/datasource;1?name=history"

// Factory constructors
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsHTTPIndex, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDirectoryViewerFactory)

#if !defined(MOZ_MACBROWSER)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsFontPackageHandler)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsWindowDataSource, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsBrowserStatusFilter)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsBrowserInstance)
#endif

#if !defined(MOZ_PHOENIX) && !defined(MOZ_XULRUNNER)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsAutoCompleteItem)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsAutoCompleteResults)
#endif

#if !defined(MOZ_PHOENIX) && !defined(MOZ_XULRUNNER) && !defined(MOZ_MACBROWSER)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(RelatedLinksHandlerImpl, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsBookmarksService, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsGlobalHistory, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsDownloadManager, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDownloadProxy)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsCmdLineService)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsAppStartup)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsUserInfo)
#if defined(MOZ_LDAP_XPCOM)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsLDAPAutoCompleteSession)
#endif // defined(MOZ_LDAP_XPCOM)
#endif // !defined(MOZ_PHOENIX) && !defined(MOZ_XULRUNNER) && !defined(MOZ_MACBROWSER)

#if defined(XP_WIN)
#if !defined(MOZ_PHOENIX) && !defined(MOZ_XULRUNNER)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsWindowsHooks)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsAlertsService)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsUrlWidget, Init)
#endif
#endif // Windows

#if (!defined(MOZ_XUL_APP)) && !defined(MOZ_MACBROWSER)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsBrowserContentHandler)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsChromeStartupHandler)
#endif


static NS_METHOD
RegisterProc(nsIComponentManager *aCompMgr,
             nsIFile *aPath,
             const char *registryLocation,
             const char *componentType,
             const nsModuleComponentInfo *info)
{
    nsresult rv;
    nsCOMPtr<nsICategoryManager> catman = do_GetService(NS_CATEGORYMANAGER_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return rv;

    // add the MIME types layotu can handle to the handlers category.
    // this allows users of layout's viewers (the docshell for example)
    // to query the types of viewers layout can create.
    return catman->AddCategoryEntry("Gecko-Content-Viewers", "application/http-index-format",
                                    "@mozilla.org/xpfe/http-index-format-factory-constructor",
                                    PR_TRUE, PR_TRUE, nsnull);
}

static NS_METHOD
UnregisterProc(nsIComponentManager *aCompMgr,
               nsIFile *aPath,
               const char *registryLocation,
               const nsModuleComponentInfo *info)
{
    nsresult rv;
    nsCOMPtr<nsICategoryManager> catman = do_GetService(NS_CATEGORYMANAGER_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return rv;

    return catman->DeleteCategoryEntry("Gecko-Content-Viewers",
                                       "application/http-index-format", PR_TRUE);
}

static NS_METHOD
RegisterWindowDS(nsIComponentManager *aCompMgr,
                 nsIFile *aPath,
                 const char *registryLocation,
                 const char *componentType,
                 const nsModuleComponentInfo *info)
{
    nsresult rv;
    nsCOMPtr<nsICategoryManager> catman = do_GetService(NS_CATEGORYMANAGER_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return rv;

    return catman->AddCategoryEntry("app-startup", "Window Data Source",
                                    "service," NS_RDF_DATASOURCE_CONTRACTID_PREFIX "window-mediator",
                                    PR_TRUE, PR_TRUE, nsnull);
}

static const nsModuleComponentInfo components[] = {
   { "Directory Viewer", NS_DIRECTORYVIEWERFACTORY_CID,
      "@mozilla.org/xpfe/http-index-format-factory-constructor",
      nsDirectoryViewerFactoryConstructor, RegisterProc, UnregisterProc  },
    { "Directory Viewer", NS_HTTPINDEX_SERVICE_CID, NS_HTTPINDEX_SERVICE_CONTRACTID,
      nsHTTPIndexConstructor },
    { "Directory Viewer", NS_HTTPINDEX_SERVICE_CID, NS_HTTPINDEX_DATASOURCE_CONTRACTID,
      nsHTTPIndexConstructor },

#if !defined(MOZ_PHOENIX) && !defined(MOZ_XULRUNNER)
    { "AutoComplete Search Results", NS_AUTOCOMPLETERESULTS_CID, NS_AUTOCOMPLETERESULTS_CONTRACTID,
      nsAutoCompleteResultsConstructor},
    { "AutoComplete Search Item", NS_AUTOCOMPLETEITEM_CID, NS_AUTOCOMPLETEITEM_CONTRACTID,
      nsAutoCompleteItemConstructor},
#endif

#if !defined(MOZ_PHOENIX) && !defined(MOZ_XULRUNNER) && !defined(MOZ_MACBROWSER)
    { "Bookmarks", NS_BOOKMARKS_SERVICE_CID, NS_BOOKMARKS_SERVICE_CONTRACTID,
      nsBookmarksServiceConstructor },
    { "Bookmarks", NS_BOOKMARKS_SERVICE_CID,
      "@mozilla.org/embeddor.implemented/bookmark-charset-resolver;1",
      nsBookmarksServiceConstructor },
    { "Bookmarks", NS_BOOKMARKS_SERVICE_CID, NS_BOOKMARKS_DATASOURCE_CONTRACTID,
      nsBookmarksServiceConstructor },
    { "Download Manager", NS_DOWNLOADMANAGER_CID, NS_DOWNLOADMANAGER_CONTRACTID,
      nsDownloadManagerConstructor },
    { "Download", NS_DOWNLOAD_CID, NS_TRANSFER_CONTRACTID,
      nsDownloadProxyConstructor },
    { "Global History", NS_GLOBALHISTORY_CID, NS_GLOBALHISTORY2_CONTRACTID,
      nsGlobalHistoryConstructor },
    { "Global History", NS_GLOBALHISTORY_CID, NS_GLOBALHISTORY_DATASOURCE_CONTRACTID,
      nsGlobalHistoryConstructor },
    { "Global History", NS_GLOBALHISTORY_CID, NS_GLOBALHISTORY_AUTOCOMPLETE_CONTRACTID,
      nsGlobalHistoryConstructor },
    { "App Startup Service",
      NS_SEAMONKEY_APPSTARTUP_CID,
      NS_APPSTARTUP_CONTRACTID,
      nsAppStartupConstructor
    },
    { "CommandLine Service",
      NS_COMMANDLINESERVICE_CID,
      NS_COMMANDLINESERVICE_CONTRACTID,
      nsCmdLineServiceConstructor
    },
    { "User Info Service",
      NS_USERINFO_CID,
      NS_USERINFO_CONTRACTID,
      nsUserInfoConstructor
    },
#if defined(MOZ_LDAP_XPCOM)
    { "LDAP Autocomplete Session", NS_LDAPAUTOCOMPLETESESSION_CID,
	  "@mozilla.org/autocompleteSession;1?type=ldap",
	  nsLDAPAutoCompleteSessionConstructor },
#endif // defined(MOZ_LDAP_XPCOM)
    { "Related Links Handler", NS_RELATEDLINKSHANDLER_CID, NS_RELATEDLINKSHANDLER_CONTRACTID,
       RelatedLinksHandlerImplConstructor},
#endif // !defined(MOZ_PHOENIX) && !defined(MOZ_XULRUNNER) && !defined(MOZ_MACBROWSER)

#if !defined(MOZ_MACBROWSER)
    { "nsCharsetMenu", NS_CHARSETMENU_CID,
      NS_RDF_DATASOURCE_CONTRACTID_PREFIX NS_CHARSETMENU_PID,
      NS_NewCharsetMenu },
    { "nsFontPackageHandler", NS_FONTPACKAGEHANDLER_CID,
      "@mozilla.org/locale/default-font-package-handler;1",
      nsFontPackageHandlerConstructor },
    { "nsWindowDataSource",
      NS_WINDOWDATASOURCE_CID,
      NS_RDF_DATASOURCE_CONTRACTID_PREFIX "window-mediator",
      nsWindowDataSourceConstructor, RegisterWindowDS },
    { NS_BROWSERSTATUSFILTER_CLASSNAME,
      NS_BROWSERSTATUSFILTER_CID,
      NS_BROWSERSTATUSFILTER_CONTRACTID,
      nsBrowserStatusFilterConstructor
    },
    { "nsBrowserInstance",
      NS_BROWSERINSTANCE_CID,
      NS_BROWSERINSTANCE_CONTRACTID,
      nsBrowserInstanceConstructor
    },
#endif

#if defined(XP_WIN)
#if !defined(MOZ_PHOENIX) && !defined(MOZ_XULRUNNER)
    { NS_IURLWIDGET_CLASSNAME, NS_IURLWIDGET_CID,
      NS_IURLWIDGET_CONTRACTID, nsUrlWidgetConstructor },
    { "nsAlertsService", NS_ALERTSSERVICE_CID,
      NS_ALERTSERVICE_CONTRACTID, nsAlertsServiceConstructor },
    { NS_IWINDOWSHOOKS_CLASSNAME, NS_IWINDOWSHOOKS_CID,
      NS_IWINDOWSHOOKS_CONTRACTID, nsWindowsHooksConstructor },
#endif
#endif // defined(XP_WIN)

#if (!defined(MOZ_XUL_APP)) && !defined(MOZ_MACBROWSER)
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"text/html",
    nsBrowserContentHandlerConstructor
  },
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"application/vnd.mozilla.xul+xml",
    nsBrowserContentHandlerConstructor
  },
#ifdef MOZ_SVG
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"image/svg+xml",
    nsBrowserContentHandlerConstructor
  },
#endif // MOZ_SVG
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"text/rdf",
    nsBrowserContentHandlerConstructor
  },
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"text/xml",
    nsBrowserContentHandlerConstructor
  },
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"application/xml",
    nsBrowserContentHandlerConstructor
  },
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"application/xhtml+xml",
    nsBrowserContentHandlerConstructor
  },
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"text/css",
    nsBrowserContentHandlerConstructor
  },
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"text/plain",
    nsBrowserContentHandlerConstructor
  },
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"image/gif",
    nsBrowserContentHandlerConstructor
  },
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"image/jpeg",
    nsBrowserContentHandlerConstructor
  },
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"image/jpg",
    nsBrowserContentHandlerConstructor
  },
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"image/png",
    nsBrowserContentHandlerConstructor
  },
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"image/bmp",
    nsBrowserContentHandlerConstructor
  },
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"image/x-icon",
    nsBrowserContentHandlerConstructor
  },
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"image/vnd.microsoft.icon",
    nsBrowserContentHandlerConstructor
  },
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"image/x-xbitmap",
    nsBrowserContentHandlerConstructor
  },
  { "Browser Content Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_CONTENT_HANDLER_CONTRACTID_PREFIX"application/http-index-format",
    nsBrowserContentHandlerConstructor
  },
  { "Browser Startup Handler",
    NS_BROWSERCONTENTHANDLER_CID,
    NS_BROWSERSTARTUPHANDLER_CONTRACTID,
    nsBrowserContentHandlerConstructor,
    nsBrowserContentHandler::RegisterProc,
    nsBrowserContentHandler::UnregisterProc,
  },
  { "Chrome Startup Handler",
    NS_CHROMESTARTUPHANDLER_CID,
    NS_CHROMESTARTUPHANDLER_CONTRACTID,
    nsChromeStartupHandlerConstructor,
    nsChromeStartupHandler::RegisterProc,
    nsChromeStartupHandler::UnregisterProc
  },
#endif //!defined(MOZ_XUL_APP) && !defined(MOZ_MACBROWSER)
};

NS_IMPL_NSGETMODULE(application, components)
