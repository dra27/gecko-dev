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
 * The Original Code is Minimo.
 *
 * The Initial Developer of the Original Code is
 * Doug Turner <dougt@meer.net>.
 * Portions created by the Initial Developer are Copyright (C) 2005
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

#include "MinimoPrivate.h"

#ifdef _BUILD_STATIC_BIN
#include "nsStaticComponents.h"
#endif

// Global variables
const static char* start_url = "chrome://minimo/content/minimo.xul";
//const static char* start_url = "http://www.meer.net/~dougt/test.html";
//const static char* start_url = "resource://gre/res/start.html";

PRBool gDumpJSConsole = PR_FALSE;

static NS_DEFINE_CID(kEventQueueServiceCID, NS_EVENTQUEUESERVICE_CID);
static NS_DEFINE_CID(kAppShellCID, NS_APPSHELL_CID);


class nsBrowserStatusFilterFactory : public nsIFactory
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIFACTORY
  
  nsBrowserStatusFilterFactory();
  ~nsBrowserStatusFilterFactory() { }
};

nsBrowserStatusFilterFactory::nsBrowserStatusFilterFactory()
{
}

NS_IMPL_ISUPPORTS1(nsBrowserStatusFilterFactory, nsIFactory)

NS_IMETHODIMP
nsBrowserStatusFilterFactory::CreateInstance(nsISupports* aOuter,
                                   const nsIID& aIID,
                                   void* *aResult)
{
  NS_ENSURE_NO_AGGREGATION(aOuter);
  
  nsBrowserStatusFilter* filter = new nsBrowserStatusFilter();
  if (!filter)
    return NS_ERROR_OUT_OF_MEMORY;

  nsresult rv = filter->QueryInterface(aIID, aResult);

  if (NS_FAILED(rv))
    delete filter;

  return rv;
}

NS_IMETHODIMP
nsBrowserStatusFilterFactory::LockFactory(PRBool)
{
  return NS_OK;
}

class ApplicationObserver: public nsIObserver 
{
public:  
  ApplicationObserver(nsIAppShell* aAppShell);
  ~ApplicationObserver();  
  
  NS_DECL_ISUPPORTS
  NS_DECL_NSIOBSERVER

  nsCOMPtr<nsIAppShell> mAppShell;
  int mWindowCount;
};


ApplicationObserver::ApplicationObserver(nsIAppShell* aAppShell)
{
    mAppShell = aAppShell;
    mWindowCount = 0;
    
    nsCOMPtr<nsIObserverService> os = do_GetService("@mozilla.org/observer-service;1");


    os->AddObserver(this, "nsIEventQueueActivated", PR_FALSE);
    os->AddObserver(this, "nsIEventQueueDestroyed", PR_FALSE);

    os->AddObserver(this, "xul-window-registered", PR_FALSE);
    os->AddObserver(this, "xul-window-destroyed", PR_FALSE);
    os->AddObserver(this, "xul-window-visible", PR_FALSE);
}

ApplicationObserver::~ApplicationObserver()
{
}

NS_IMPL_ISUPPORTS1(ApplicationObserver, nsIObserver)

NS_IMETHODIMP
ApplicationObserver::Observe(nsISupports *aSubject, const char *aTopic, const PRUnichar *aData)
{
    if (!strcmp(aTopic, "nsIEventQueueActivated")) 
    {
        nsCOMPtr<nsIEventQueue> eq(do_QueryInterface(aSubject));
        if (eq)
        {
            PRBool isNative = PR_TRUE;
            // we only add native event queues to the appshell
            eq->IsQueueNative(&isNative);
            if (isNative)
                mAppShell->ListenToEventQueue(eq, PR_TRUE);
        }
    } 
    else if (!strcmp(aTopic, "nsIEventQueueDestroyed")) 
    {
        nsCOMPtr<nsIEventQueue> eq(do_QueryInterface(aSubject));
        if (eq) 
        {
            PRBool isNative = PR_TRUE;
            // we only remove native event queues from the appshell
            eq->IsQueueNative(&isNative);
            if (isNative)
                mAppShell->ListenToEventQueue(eq, PR_FALSE);
        }
    } 
    else if (!strcmp(aTopic, "xul-window-visible"))
    {
        KillSplashScreen();
    }
 
    else if (!strcmp(aTopic, "xul-window-registered"))
    {
        mWindowCount++;
    }
    else if (!strcmp(aTopic, "xul-window-destroyed"))
    {
        mWindowCount--;
        if (mWindowCount == 0)
            mAppShell->Exit();
    }

    return NS_OK;
}

nsresult StartupProfile()
{    
    NS_TIMELINE_MARK_FUNCTION("Profile Startup");

	nsCOMPtr<nsIFile> appDataDir;
	nsresult rv = NS_GetSpecialDirectory(NS_APP_APPLICATION_REGISTRY_DIR, getter_AddRefs(appDataDir));
	if (NS_FAILED(rv))
        return rv;
    
	rv = appDataDir->Append(NS_LITERAL_STRING("minimo"));
	if (NS_FAILED(rv))
        return rv;

	nsCOMPtr<nsILocalFile> localAppDataDir(do_QueryInterface(appDataDir));
    
	nsCOMPtr<nsProfileDirServiceProvider> locProvider;
    NS_NewProfileDirServiceProvider(PR_TRUE, getter_AddRefs(locProvider));
    if (!locProvider)
        return NS_ERROR_FAILURE;
    
	rv = locProvider->Register();
    if (NS_FAILED(rv))
        return rv;
    
	return locProvider->SetProfileDir(localAppDataDir);   
}

void DoPreferences()
{
    nsCOMPtr<nsIPrefBranch> prefBranch = do_GetService(NS_PREFSERVICE_CONTRACTID);
    if (!prefBranch)
        return;

    prefBranch->SetIntPref("snav.keyCode.modifier", 0);
    prefBranch->GetBoolPref("config.wince.dumpJSConsole", &gDumpJSConsole);
}

void OverrideComponents()
{
  static NS_DEFINE_CID(kBrowserStatusFilter, NS_BROWSERSTATUSFILTER_CID);

  nsCOMPtr<nsIComponentRegistrar> registrar;
  NS_GetComponentRegistrar(getter_AddRefs(registrar));

  nsBrowserStatusFilterFactory* factory = new nsBrowserStatusFilterFactory();

  if (!factory)
    return;

  if (registrar)
    registrar->RegisterFactory(kBrowserStatusFilter,
                               NS_BROWSERSTATUSFILTER_CLASSNAME,
                               NS_BROWSERSTATUSFILTER_CONTRACTID,
                               factory);
}

#ifdef WINCE
typedef struct FindAppStruct
{
  HWND hwnd;
} FindAppStruct;

BOOL CALLBACK FindApplicationWindowProc(HWND hwnd, LPARAM lParam)
{
  FindAppStruct* findApp = (FindAppStruct*) lParam;
  
  unsigned short windowName[MAX_PATH];
  GetWindowTextW(hwnd, windowName, MAX_PATH);
  
  if (wcsstr(windowName, L"Minimo"))
  {
    findApp->hwnd = hwnd;
    return FALSE;
  }
  return TRUE;
} 

PRBool DoesProcessAlreadyExist()
{
    const HANDLE hMutex = CreateMutexW(0, 0, L"_MINIMO_EXE_MUTEX_");
    
	if(hMutex)
    {
      if(ERROR_ALREADY_EXISTS == GetLastError()) 
      {
        FindAppStruct findApp;
        findApp.hwnd = NULL;
        
        EnumWindows(FindApplicationWindowProc, (LPARAM)&findApp);
        
        if (findApp.hwnd)
        {
          SetForegroundWindow(findApp.hwnd);
          return TRUE;
        }

        MessageBox(0, "Minimo is running, but can't be switched to.", "Unexpected Error", 0);
        return TRUE;
      }
      return FALSE;
    }
    MessageBox(0, "Can not start Minimo", "Unexpected Error", 0);
    return TRUE;
}
#else
PRBool DoesProcessAlreadyExist() {return PR_FALSE;}
#endif


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   Complete hack below.  We to ensure that the layout of all
 *   of the shared libaries are at tightly packed as possible.
 *   We do this by explictly loading all of the modules we
 *   know about at compile time.  This seams to work for our
 *   purposes.  Ultimately, we should statically link all of
 *   these libraries.
 *
 *   If you are building this to put on a ROM where XIP exists.
 *
 *   For more information:
 *   1) http://msdn.microsoft.com/library/default.asp?url=/ \
        library/en-us/dncenet/html/advmemmgmt.asp
 *
 * * * * * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * */
#ifdef WINCE
#define HACKY_PRE_LOAD_LIBRARY 1
#endif

#ifdef HACKY_PRE_LOAD_LIBRARY
typedef struct _library
{
	const unsigned short* name;
	HMODULE module;
} _library;

_library Libraries[] =
{
  {  L"js3250.dll",     NULL },
  {  L"nspr4.dll",      NULL },
  {  L"nss3.dll",       NULL },
  {  L"nssckbi.dll",    NULL },
  {  L"plc4.dll",       NULL },
  {  L"plds4.dll",      NULL },
  {  L"shunt.dll",      NULL },
  {  L"smime3.dll",     NULL },
  {  L"softokn3.dll",   NULL },
  {  L"ssl3.dll",       NULL },
  {  L"xpcom.dll",      NULL },
  {  L"xpcom_core.dll", NULL },
  {  NULL, NULL },
};

void LoadKnownLibs()
{
  for (int i=0; Libraries[i].name; i++)
    Libraries[i].module = LoadLibraryW(Libraries[i].name);
}

void UnloadKnownLibs()
{
  for (int i=0; Libraries[i].name; i++)
    if (Libraries[i].module)
      FreeLibrary(Libraries[i].module);
}
#endif // HACKY_PRE_LOAD_LIBRARY

int main(int argc, char *argv[])
{
    if (DoesProcessAlreadyExist())
        return 0;

    CreateSplashScreen();

#ifdef HACKY_PRE_LOAD_LIBRARY
    LoadKnownLibs();
#endif

#ifdef _BUILD_STATIC_BIN
    NS_InitEmbedding(nsnull, nsnull, kPStaticModules, kStaticModuleCount);
#else
    NS_InitEmbedding(nsnull, nsnull);
#endif
    // Choose the new profile
    if (NS_FAILED(StartupProfile()))
        return 1;
    
    DoPreferences();
    OverrideComponents();

    NS_TIMELINE_ENTER("appStartup");
    nsCOMPtr<nsIAppShell> appShell = do_CreateInstance(kAppShellCID);
    appShell->Create(nsnull, nsnull);
    
    ApplicationObserver *appObserver = new ApplicationObserver(appShell);
    if (!appObserver)
        return 1;
    NS_ADDREF(appObserver);
    
    WindowCreator *creatorCallback = new WindowCreator(appShell);
    if (!creatorCallback)
        return 1;

    nsCOMPtr<nsIWindowWatcher> wwatch(do_GetService(NS_WINDOWWATCHER_CONTRACTID));
    wwatch->SetWindowCreator(creatorCallback);

    nsCOMPtr<nsIDOMWindow> newWindow;
    wwatch->OpenWindow(nsnull, start_url, "_blank", "chrome,dialog=no,all", nsnull, getter_AddRefs(newWindow));

    appShell->Run();

    appShell = nsnull;
    wwatch = nsnull;
    newWindow = nsnull;

    delete appObserver;
    delete creatorCallback;

    if (gDumpJSConsole)
      WriteConsoleLog();

    // Close down Embedding APIs
    NS_TermEmbedding();

#ifdef HACKY_PRE_LOAD_LIBRARY
    UnloadKnownLibs();
#endif
    return NS_OK;
}
