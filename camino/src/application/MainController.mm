/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Josh Aas - josha@mac.com
 *   Nate Weaver (Wevah) - wevah@derailer.org
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

#import <Carbon/Carbon.h>

#import "NSString+Utils.h"
#import "NSResponder+Utils.h"
#import "NSMenu+Utils.h"
#import "NSURL+Utils.h"

#import "ChimeraUIConstants.h"
#import "MainController.h"
#import "BrowserWindow.h"
#import "BrowserWindowController.h"
#import "BookmarkMenu.h"
#import "Bookmark.h"
#import "BookmarkFolder.h"
#import "BookmarkInfoController.h"
#import "BookmarkManager.h"
#import "BookmarkToolbar.h"
#import "BrowserTabView.h"
#import "CHBrowserService.h"
#import "UserDefaults.h"
#import "KeychainService.h"
#import "RemoteDataProvider.h"
#import "ProgressDlgController.h"
#import "JSConsole.h"
#import "NetworkServices.h"
#import "MVPreferencesController.h"
#import "FindDlgController.h"
#import "PreferenceManager.h"
#import "SharedMenusObj.h"
#import "SiteIconProvider.h"

#include "nsBuildID.h"
#include "nsCOMPtr.h"
#include "nsEmbedAPI.h"
#include "nsString.h"
#include "nsStaticComponents.h"

#include "nsIWebBrowserChrome.h"
#include "nsIServiceManager.h"
#include "nsIIOService.h"
#include "nsIPref.h"
#include "nsIChromeRegistry.h"
#include "nsIObserverService.h"
#include "nsIGenericFactory.h"
#include "nsIEventQueueService.h"
#include "nsNetCID.h"
#include "nsIPermissionManager.h"
#include "nsICookieManager.h"
#include "nsIBrowserHistory.h"
#include "nsICacheService.h"

extern const nsModuleComponentInfo* GetAppComponents(unsigned int * outNumComponents);

static const char* ioServiceContractID = "@mozilla.org/network/io-service;1";

// Constants on how to behave when we are asked to open a URL from
// another application. These are values of the "browser.reuse_window" pref.
const int kOpenNewWindowOnAE = 0;
const int kOpenNewTabOnAE = 1;
const int kReuseWindowOnAE = 2;

@interface MainController(Private)<NetworkServicesClient>

- (void)setupStartpage;
- (void)setupRendezvous;
- (NSMenu*)bookmarksMenu;
- (BOOL)bookmarksItemsEnabled;
- (void)adjustBookmarkMenuItems;
- (void)updateDockMenuBookmarkFolder;
- (void)doBookmarksMenuEnabling;
- (void)adjustTextEncodingMenu;

- (void)windowLayeringDidChange:(NSNotification*)inNotification;
- (void)bookmarkLoadingCompleted:(NSNotification*)inNotification;
- (void)dockMenuBookmarkFolderChanged:(NSNotification*)inNotification;
- (void)menuWillDisplay:(NSNotification*)inNotification;
- (void)openPanelDidEnd:(NSOpenPanel*)inOpenPanel returnCode:(int)inReturnCode contextInfo:(void*)inContextInfo;

@end

#pragma mark -

@implementation MainController

//
// +isRunningOnTiger
//
// returns YES if we're on 10.4 or better
//
+ (BOOL)isTigerOrHigher
{
  static BOOL sInitTigerOrHigher = NO;
  static BOOL sRunningOnTiger = NO;
  if (!sInitTigerOrHigher) {
    long version = 0;
    ::Gestalt(gestaltSystemVersion, &version);
    if (version >= 0x00001040) 
      sRunningOnTiger = YES;
    sInitTigerOrHigher = YES;
  }
  return sRunningOnTiger;
}

//
// +supportsSpotlight
//
// returns YES if we're running on a machine that supports spotlight (tiger or higher)
//
+ (BOOL)supportsSpotlight
{
  return [self isTigerOrHigher];
}

//
// +supportsBonjour
//
// returns YES if we're running on a machine that supports bonjour (formerly rendezvous, 10.2
// or higher)
//
+ (BOOL)supportsBonjour
{
  // are we on 10.2.3 or higher? The DNS resolution stuff is broken before 10.2.3
  BOOL supportsBonjour = NO;
  long version = 0;
  ::Gestalt(gestaltSystemVersion, &version);
  if (version >= 0x00001023) 
    supportsBonjour = YES;
  return supportsBonjour;
}

-(id)init
{
  if ( (self = [super init]) ) {
    // ensure that we're at least on 10.2 as lower OS versions are not supported any more
    long version = 0;
    ::Gestalt(gestaltSystemVersion, &version);
    if (version < 0x00001020) {
      NSString* appName = NSLocalizedStringFromTable(@"CFBundleName", @"InfoPlist", nil);
      NSString* alert = [NSString stringWithFormat: NSLocalizedString(@"RequiredVersionNotMetTitle", @""), appName];
      NSString* message = [NSString stringWithFormat: NSLocalizedString(@"RequiredVersionNotMet", @""), appName];
      NSString* quit = NSLocalizedString(@"AlreadyRunningButton",@"");
      NSRunAlertPanel(alert, message, quit, nil, nil);
      [NSApp terminate:self];
    }
    
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    if ([defaults boolForKey:USER_DEFAULTS_AUTOREGISTER_KEY]) {
      // This option causes us to simply initialize embedding and exit.
      NSString *path = [[[NSBundle mainBundle] executablePath] stringByDeletingLastPathComponent];
      setenv("MOZILLA_FIVE_HOME", [path fileSystemRepresentation], 1);

      if (NS_SUCCEEDED(NS_InitEmbedding(nsnull, nsnull,
                                        kPStaticModules, kStaticModuleCount))) {
        // Register new chrome
        nsCOMPtr<nsIChromeRegistry> chromeReg =
          do_GetService("@mozilla.org/chrome/chrome-registry;1");
        if (chromeReg) {
          chromeReg->CheckForNewChrome();
          chromeReg = 0;
        }
        NS_TermEmbedding();
      }

      [NSApp terminate:self];
      return self;
    }

    NSString* url = [defaults stringForKey:USER_DEFAULTS_URL_KEY];
    mStartURL = url ? [url retain] : nil;
    
    mFindDialog = nil;
    mMenuBookmarks = nil;
    
    [NSApp setServicesProvider:self];
    // Initialize shared menu support
    mSharedMenusObj = [[SharedMenusObj alloc] init];
  }
  return self;
}

-(void)dealloc
{
  [mCharsets release];
  
  // Terminate shared menus
  [mSharedMenusObj release];

  [mFindDialog release];
  [mKeychainService release];
  
  [super dealloc];
#if DEBUG
  NSLog(@"Main controller died");
#endif
}

- (void)awakeFromNib
{
  // Be aware that we load a secondary nib for the accessory views, so this
  // will get called more than once.
}

- (void)ensureGeckoInitted
{
  if (mGeckoInitted)
    return;

  // bring up prefs manager (which inits gecko)
  [PreferenceManager sharedInstance];

  // register our app components with the embed layer
  unsigned int numComps = 0;
  const nsModuleComponentInfo* comps = GetAppComponents(&numComps);
  CHBrowserService::RegisterAppComponents(comps, numComps);

  // To work around a bug on Tiger where the view hookup order has been changed from postfix to prefix
  // order, we need to set a user default to return to the old behavior.
  [[NSUserDefaults standardUserDefaults] setBool:NO forKey:@"NSViewSetAncestorsWindowFirst"];

  mGeckoInitted = YES;
}

-(void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  [self ensureGeckoInitted];

  // previous versions would keep the cache in the profile folder. If we find it there, remove it so
  // that backup apps can more easily back up our profile. This will mean if anyone goes back to 
  // 0.8.x, they'll lose their favicons and cache, but that's ok.
  NSString* cacheDir = [[[PreferenceManager sharedInstance] newProfilePath] stringByAppendingPathComponent:@"Cache"];
  [[NSFileManager defaultManager] removeFileAtPath:cacheDir handler:nil];

  NSNotificationCenter* notificationCenter = [NSNotificationCenter defaultCenter];
  // turn on menu display notifications
  [NSMenu setupMenuWillDisplayNotifications];
  
  // register for them for bookmarks
  [notificationCenter addObserver:self selector:@selector(menuWillDisplay:) name:NSMenuWillDisplayNotification object:nil];
  
  // register for various window layering changes
  [notificationCenter addObserver:self selector:@selector(windowLayeringDidChange:) name:NSWindowDidBecomeKeyNotification object:nil];
  [notificationCenter addObserver:self selector:@selector(windowLayeringDidChange:) name:NSWindowDidResignKeyNotification object:nil];
  [notificationCenter addObserver:self selector:@selector(windowLayeringDidChange:) name:NSWindowDidBecomeMainNotification object:nil];
  [notificationCenter addObserver:self selector:@selector(windowLayeringDidChange:) name:NSWindowDidResignMainNotification object:nil];
  
  // listen for bookmark loading completion
  [notificationCenter addObserver:self selector:@selector(bookmarkLoadingCompleted:) name:kBookmarkManagerStartedNotification object:nil];
  // listen for changes to the dock menu
  [notificationCenter addObserver:self selector:@selector(dockMenuBookmarkFolderChanged:) name:BookmarkFolderDockMenuChangeNotificaton object:nil];

  // and fire up bookmarks (they will be loaded on a thread)
  [[BookmarkManager sharedBookmarkManager] loadBookmarksLoadingSynchronously:NO];

  // register some special favicon images
  [[SiteIconProvider sharedFavoriteIconProvider] registerFaviconImage:[NSImage imageNamed:@"smallDocument"] forPageURI:@"about:blank"];
  [[SiteIconProvider sharedFavoriteIconProvider] registerFaviconImage:[NSImage imageNamed:@"bm_favicon"]    forPageURI:@"about:bookmarks"];
  [[SiteIconProvider sharedFavoriteIconProvider] registerFaviconImage:[NSImage imageNamed:@"historyicon"]   forPageURI:@"about:history"];
  
  [self setupStartpage];

  // Initialize offline mode.
  mOffline = NO;
  nsCOMPtr<nsIIOService> ioService(do_GetService(ioServiceContractID));
  if (!ioService)
    return;
  PRBool offline = PR_FALSE;
  ioService->GetOffline(&offline);
  mOffline = offline;
  
  // Initialize the keychain service.
  mKeychainService = [KeychainService instance];
  
  // bring up the JS console service
  BOOL success;
  if ([[PreferenceManager sharedInstance] getBooleanPref:"chimera.log_js_to_console" withSuccess:&success])
    [JSConsole sharedJSConsole];

  [self setupRendezvous];
  
  // load up the charset dictionary with keys and menu titles.
  NSString* charsetPath = [NSBundle pathForResource:@"Charset" ofType:@"dict" inDirectory:[[NSBundle mainBundle] bundlePath]];
  mCharsets = [[NSDictionary dictionaryWithContentsOfFile:charsetPath] retain];

  // open a new browser window if we don't already have one
  // (for example, from an GetURL Apple Event)
  NSWindow* browserWindow = [self getFrontmostBrowserWindow];
  if (!browserWindow)
    [self newWindow:self];
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
  return [[ProgressDlgController sharedDownloadController] allowTerminate];
}

-(void)applicationWillTerminate: (NSNotification*)aNotification
{
#if DEBUG
  NSLog(@"App will terminate notification");
#endif
  
  [NetworkServices shutdownNetworkServices];

  [[BookmarkManager sharedBookmarkManager] shutdown];

  // Autosave one of the windows.
  [[[mApplication mainWindow] windowController] autosaveWindowFrame];
  
  // Cancel outstanding site icon loads
  [[RemoteDataProvider sharedRemoteDataProvider] cancelOutstandingRequests];
  
  // make sure the info window is closed
  [BookmarkInfoController closeBookmarkInfoController];
    
  // Release before calling TermEmbedding since we need to access XPCOM
  // to save preferences
  [MVPreferencesController clearSharedInstance];
  
  CHBrowserService::TermEmbedding();
  
  [self autorelease];
}

- (void)windowLayeringDidChange:(NSNotification*)inNotification
{
  [self performSelectorOnMainThread:@selector(doBookmarksMenuEnabling) withObject:nil waitUntilDone:NO];
}

- (NSMenu *)applicationDockMenu:(NSApplication *)sender
{
  // the dock menu doesn't get the usual show notifications, so we rebuild it explicitly here
  [mDockMenu rebuildMenuIncludingSubmenus:YES];
  return mDockMenu;
}

- (void)setupStartpage
{
  // only do this if no url was specified in the command-line
  if (mStartURL) return;
  // for non-nightly builds, show a special start page
  PreferenceManager* prefManager = [PreferenceManager sharedInstance];
  NSString* vendorSubString = [prefManager getStringPref:"general.useragent.vendorSub" withSuccess:NULL];
  if (![vendorSubString hasSuffix:@"+"]) {
    // has the user seen this already?
    NSString* startPageRev = [prefManager getStringPref:"browser.startup_page_override.version" withSuccess:NULL];
    if (![vendorSubString isEqualToString:startPageRev]) {
      NSString* startPage = NSLocalizedStringFromTable( @"StartPageDefault", @"WebsiteDefaults", nil);
      if ([startPage length] && ![startPage isEqualToString:@"StartPageDefault"]) {
        [mStartURL release];
        mStartURL = [startPage retain];
      }
      // set the pref to say they've seen it
      [prefManager setPref:"browser.startup_page_override.version" toString:vendorSubString];
    }
  }
}

- (void)setupRendezvous // aka "Bonjour"
{
  BOOL doingRendezvous = NO;
  
  if ([[PreferenceManager sharedInstance] getBooleanPref:"chimera.enable_rendezvous" withSuccess:NULL]) {
    if ([MainController supportsBonjour]) {
      NSNotificationCenter* notificationCenter = [NSNotificationCenter defaultCenter];
      [notificationCenter addObserver:self selector:@selector(availableServicesChanged:) name:NetworkServicesAvailableServicesChanged object:nil];
      [notificationCenter addObserver:self selector:@selector(serviceResolved:) name:NetworkServicesResolutionSuccess object:nil];
      [notificationCenter addObserver:self selector:@selector(serviceResolutionFailed:) name:NetworkServicesResolutionFailure object:nil];
      doingRendezvous = YES;
    }
  }
  
  if (!doingRendezvous) {
    // remove rendezvous items
    int itemIndex;
    while ((itemIndex = [mGoMenu indexOfItemWithTag:kRendezvousRelatedItemTag]) != -1)
      [mGoMenu removeItemAtIndex:itemIndex];
  }
}

//
// bookmarkLoadingCompleted:
//
- (void)bookmarkLoadingCompleted:(NSNotification*)inNotification
{
  // the bookmarks menus get built lazily (by BookmarkMenu)
  [mBookmarksMenu setBookmarkFolder:[[BookmarkManager sharedBookmarkManager] bookmarkMenuFolder]];

  // dock bookmarks
  [self updateDockMenuBookmarkFolder];
}

- (void)dockMenuBookmarkFolderChanged:(NSNotification*)inNotification
{
  [self updateDockMenuBookmarkFolder];
}

- (void)updateDockMenuBookmarkFolder
{
  [mDockMenu setBookmarkFolder:[[BookmarkManager sharedBookmarkManager] dockMenuFolder]];
}

// a central place for bookmark opening logic.
- (void)loadBookmark:(BookmarkItem*)item withWindowController:(BrowserWindowController*)browserWindowController openBehavior:(EBookmarkOpenBehavior)behavior
{
  if (!browserWindowController)
    browserWindowController = [self getMainWindowBrowserController];
  
  BOOL openInNewWindow = (browserWindowController == nil);
  BOOL openInNewTab = NO;
  BOOL newTabInBackground = NO;
  
  BOOL loadNewTabsInBackgroundPref = [[PreferenceManager sharedInstance] getBooleanPref:"browser.tabs.loadInBackground" withSuccess:NULL];
  
  NSWindow* behindWindow = nil;

  switch (behavior)
  {
    case eBookmarkOpenBehaviorDefault:
      {
        BOOL cmdKeyDown = (GetCurrentKeyModifiers() & cmdKey) != 0;
        if (cmdKeyDown)
          if ([[PreferenceManager sharedInstance] getBooleanPref:"browser.tabs.opentabfor.middleclick" withSuccess:NULL]) {
            openInNewTab = YES;
            newTabInBackground = loadNewTabsInBackgroundPref;
          }
          else {
            openInNewWindow = YES;
            if (loadNewTabsInBackgroundPref)
              behindWindow = [browserWindowController window];
          }
      }
      break;

    case eBookmarkOpenBehaviorNewTabDefault:
      openInNewTab = YES;
      newTabInBackground = browserWindowController && loadNewTabsInBackgroundPref;
      break;

    case eBookmarkOpenBehaviorNewTabForeground:
      openInNewTab = YES;
      break;

    case eBookmarkOpenBehaviorNewTabBackground:
      openInNewTab = YES;
      newTabInBackground = YES;
      break;

    case eBookmarkOpenBehaviorNewWindowDefault:
      openInNewWindow = YES;
      if (loadNewTabsInBackgroundPref)
        behindWindow = [browserWindowController window];
      break;

    case eBookmarkOpenBehaviorNewWindowForeground:
      openInNewWindow = YES;
      break;

    case eBookmarkOpenBehaviorNewWindowBackground:
      openInNewWindow = YES;
      behindWindow = [browserWindowController window];
      break;
  }

  // we allow popups for the load that fires off a bookmark. Subsequent page loads, however, will
  // not allow popups (if blocked).
  if ([item isKindOfClass:[Bookmark class]]) {
    if (openInNewWindow)
      [self openBrowserWindowWithURL:[(Bookmark *)item url] andReferrer:nil behind:behindWindow allowPopups:YES];
    else if (openInNewTab)
      [browserWindowController openNewTabWithURL:[(Bookmark *)item url] referrer:nil loadInBackground:newTabInBackground allowPopups:YES];
    else
      [browserWindowController loadURL:[(Bookmark *)item url] referrer:nil activate:YES allowPopups:YES];
  }
  else if ([item isKindOfClass:[BookmarkFolder class]]) {
    if (openInNewWindow)
      [self openBrowserWindowWithURLs:[(BookmarkFolder *)item childURLs] behind:behindWindow allowPopups:YES];
    else if (openInNewTab)
      [browserWindowController openURLArray:[(BookmarkFolder *)item childURLs] tabOpenPolicy:eAppendTabs allowPopups:YES];
    else
      [browserWindowController openURLArrayReplacingTabs:[(BookmarkFolder *)item childURLs] closeExtraTabs:[(BookmarkFolder *)item isGroup] allowPopups:YES];
  }
}

/*
This takes an NSURL to a local file, and if that file is a file that contains
a URL we want and isn't the content itself, we return the URL it contains.
Otherwise, we return the URL we originally got. Right now this supports .url and
.webloc files.
*/
+(NSURL*) decodeLocalFileURL:(NSURL*)url
{  
  NSString *urlPathString = [url path];

  if ([[urlPathString pathExtension] isEqualToString:@"url"])
    url = [NSURL urlFromIEURLFile:urlPathString];
  else if ([[urlPathString pathExtension] isEqualToString:@"webloc"])
    url = [NSURL urlFromWebloc:urlPathString];

  return url;
}

+(BOOL) isBlankURL:(NSString*)inURL
{
  BOOL isBlank = NO;
  if (!inURL || [inURL isEqualToString: @"about:blank"] || [inURL isEqualToString: @""])
    isBlank = YES;
  return isBlank;
}

-(IBAction)newWindow:(id)aSender
{
  // If we have a key window, have it autosave its dimensions before
  // we open a new window.  That ensures the size ends up matching.
  NSWindow* mainWindow = [mApplication mainWindow];
  if (mainWindow && [[mainWindow windowController] respondsToSelector:@selector(autosaveWindowFrame)])
    [[mainWindow windowController] autosaveWindowFrame];

  // Now open the new window.
  NSString* homePage = mStartURL ? mStartURL : [[PreferenceManager sharedInstance] homePage:YES];
  BrowserWindowController* controller = [self openBrowserWindowWithURL:homePage andReferrer:nil behind:nil allowPopups:NO];

  if ([MainController isBlankURL:homePage])
    [controller focusURLBar];
  else
    [[[controller getBrowserWrapper] getBrowserView] setActive: YES];

  // Only load the command-line specified URL for the first window we open
  if (mStartURL) {
    [mStartURL release];
    mStartURL = nil;
  }
}	

-(IBAction)newTab:(id)aSender
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
    [browserController newTab:aSender];
  else
  {
    // follow the pref about what to load in a new tab (even though we're making a new window)
    int newTabPage = [[PreferenceManager sharedInstance] getIntPref:"browser.tabs.startPage" withSuccess:NULL];
    BOOL loadHomepage = (newTabPage == 1);

    NSString* urlToLoad = @"about:blank";
    if (loadHomepage)
      urlToLoad = [[PreferenceManager sharedInstance] homePage:NO];

    [self openBrowserWindowWithURL:urlToLoad andReferrer:nil behind:nil allowPopups:NO];
  }
}

-(IBAction)closeTab:(id)aSender
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
    [browserController closeCurrentTab:aSender];
}

-(IBAction) previousTab:(id)aSender
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
    [browserController previousTab:aSender];
}

-(IBAction) nextTab:(id)aSender
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
    [browserController nextTab:aSender];
}

-(IBAction) openFile:(id)aSender
{
  NSOpenPanel* openPanel = [NSOpenPanel openPanel];
  [openPanel setCanChooseFiles:YES];
  [openPanel setCanChooseDirectories:NO];
  [openPanel setAllowsMultipleSelection:YES];
  NSArray* fileTypes = [NSArray arrayWithObjects: @"htm",@"html",@"shtml",@"xhtml",@"xml",
                                                  @"txt",@"text",
                                                  @"gif",@"jpg",@"jpeg",@"png",@"bmp",
                                                  @"webloc",@"url",
                                                  nil];

  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
  {
    [openPanel beginSheetForDirectory:nil
                                 file:nil
                                types:fileTypes
                       modalForWindow:[browserController window]
                        modalDelegate:self
                       didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
                          contextInfo:browserController];
  }
  else
  {
    int result = [openPanel runModalForTypes:fileTypes];
    [self openPanelDidEnd:openPanel returnCode:result contextInfo:nil];
  }
}

- (void)openPanelDidEnd:(NSOpenPanel*)inOpenPanel returnCode:(int)inReturnCode contextInfo:(void*)inContextInfo
{
  if (inReturnCode != NSOKButton)
    return;

  BrowserWindowController* browserController = (BrowserWindowController*)inContextInfo;

  NSArray* urlArray = [inOpenPanel URLs];
  if ([urlArray count] == 0)
      return;

  NSMutableArray* urlStringsArray = [NSMutableArray arrayWithCapacity:[urlArray count]];
  
  // fix them up
  NSEnumerator* urlsEnum = [urlArray objectEnumerator];
  NSURL* curURL;
  while ((curURL = [urlsEnum nextObject]))
  {
    [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:curURL];
    curURL = [MainController decodeLocalFileURL:curURL];
    [urlStringsArray addObject:[curURL path]];
  }

  if (!browserController)
  {
    [self openBrowserWindowWithURLs:urlStringsArray behind:nil allowPopups:YES];
  }
  else
  {
    [browserController openURLArray:urlStringsArray tabOpenPolicy:eAppendFromCurrentTab allowPopups:YES];
  }
}

-(IBAction) openLocation:(id)aSender
{
    NSWindow* browserWindow = [self getFrontmostBrowserWindow];
    if (!browserWindow) {
      [self openBrowserWindowWithURL: @"about:blank" andReferrer:nil behind:nil allowPopups:NO];
      browserWindow = [mApplication mainWindow];
    }
    else if (![browserWindow isMainWindow] || ![browserWindow isKeyWindow]) {
      [browserWindow makeKeyAndOrderFront:self];
    }
    
    [[browserWindow windowController] performAppropriateLocationAction];
}

-(IBAction) savePage:(id)aSender
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
    [browserController saveDocument:NO filterView:[self getSavePanelView]];
}

-(IBAction) pageSetup:(id)aSender
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
    [browserController pageSetup:aSender];
}

-(IBAction) printPage:(id)aSender
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
    [browserController printDocument:aSender];
}

-(IBAction) toggleOfflineMode:(id)aSender
{
    nsCOMPtr<nsIIOService> ioService(do_GetService(ioServiceContractID));
    if (!ioService)
        return;
    PRBool offline = PR_FALSE;
    ioService->GetOffline(&offline);
    ioService->SetOffline(!offline);
    mOffline = !offline;
    
    // Update the menu item text.
    // Set the menu item's text to "Go Online" if we're currently
    // offline.
/*
    if (mOffline)
        [mOfflineMenuItem setTitle: @"Go Online"];
    else
        [mOfflineMenuItem setTitle: @"Work Offline"];
*/
        
    // Indicate that we are working offline.
    [[NSNotificationCenter defaultCenter] postNotificationName:@"offlineModeChanged" object:nil];
}

- (IBAction)sendURL:(id)aSender
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
    [browserController sendURL:aSender];
}


// Edit menu actions.


//
// -findInPage
//
// Called in response to "Find" in edit menu. Opens the find dialog. We only keep
// one around for the whole app to use, showing/hiding as we see fit.
//
-(IBAction) findInPage:(id)aSender
{
  if ( !mFindDialog )
     mFindDialog = [[FindDlgController alloc] initWithWindowNibName: @"FindDialog"];
  [mFindDialog showWindow:self];
}


-(IBAction) getInfo:(id)aSender
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
    [browserController getInfo: aSender]; 
}

-(IBAction) goBack:(id)aSender
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
    [browserController back: aSender]; 
}

-(IBAction) goForward:(id)aSender
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
    [browserController forward: aSender]; 
}

-(IBAction) doReload:(id)aSender
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
    [browserController reload: aSender]; 
}

-(IBAction) reloadWithCharset:(id)aSender
{
  // Figure out which charset to tell gecko to load based on the sender's tag. There
  // is guaranteed to only be 1 key that matches this tag, so we just take the first one.
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController) {
    NSArray* charsetList = [mCharsets allKeysForObject:[NSNumber numberWithInt:[aSender tag]]];
    NS_ASSERTION([charsetList count] == 1, "OOPS, multiply defined charsets in plist");
    [browserController reloadWithNewCharset:[charsetList objectAtIndex:0]];
  }
}

-(IBAction) toggleAutoCharsetDetection:(id)aSender
{
  NSString* detectorValue = [[PreferenceManager sharedInstance] getStringPref:"intl.charset.detector" withSuccess:NULL];
  BOOL universalChardetOn = [detectorValue isEqualToString:@"universal_charset_detector"];
  NSString* newValue = universalChardetOn ? @"" : @"universal_charset_detector";
  [[PreferenceManager sharedInstance] setPref:"intl.charset.detector" toString:newValue];
  // and reload
  [self doReload:nil];
}

-(IBAction) doStop:(id)aSender
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
    [browserController stop: aSender]; 
}

-(IBAction) goHome:(id)aSender
{
  NSWindow* browserWindow = [self getFrontmostBrowserWindow];
  if (browserWindow) {
    if (![browserWindow isMainWindow])
      [browserWindow makeKeyAndOrderFront:self];
    
    [[browserWindow windowController] home:aSender];
  }
  else {
    // explicity open the home page to work around "load home page in new window" pref
    [self openBrowserWindowWithURL:(mStartURL ? mStartURL : [[PreferenceManager sharedInstance] homePage:NO]) andReferrer:nil behind:nil allowPopups:NO];
  }
}

-(IBAction) doSearch:(id)aSender
{
  NSWindow* browserWindow = [self getFrontmostBrowserWindow];

  if (browserWindow) {
    if (![browserWindow isMainWindow])
      [browserWindow makeKeyAndOrderFront:self];
  } else {
      [self newWindow:self];
      browserWindow = [self getFrontmostBrowserWindow];
  }

  [[browserWindow windowController] performAppropriateSearchAction];
}

-(IBAction) downloadsWindow:(id)aSender
{
	[[ProgressDlgController sharedDownloadController] showWindow:aSender];
}

- (void)menuWillDisplay:(NSNotification*)inNotification
{
  if ([mBookmarksMenu isTargetOfWillDisplayNotification:[inNotification object]])
  {
    [self adjustBookmarkMenuItems];
  }
  else if ([mTextEncodingsMenu isTargetOfWillDisplayNotification:[inNotification object]])
  {
    [self adjustTextEncodingMenu];
  }
}

- (void)adjustBookmarksMenuItemsEnabling
 {
   // we do this after a delay to ensure that window layer state has been set by the time
   // we do the enabling.
  [self performSelectorOnMainThread:@selector(doBookmarksMenuEnabling) withObject:nil waitUntilDone:NO];
}

//
// -doBookmarksMenuEnabling
//
// We've turned off auto-enabling for the bookmarks menu because of the unknown
// number of bookmarks in the list so we have to manage it manually. This routine
// should be called whenever a window goes away, becomes main, or is no longer main.
//
- (void)doBookmarksMenuEnabling
{
  // update our stand-in menu by hand (because it doesn't get autoupdated)
  [mBookmarksHelperMenu update];

  [mAddBookmarkMenuItem               takeStateFromItem:[mBookmarksHelperMenu itemWithTarget:[mAddBookmarkMenuItem target]
                                                                                   andAction:[mAddBookmarkMenuItem action]]];
  [mCreateBookmarksFolderMenuItem     takeStateFromItem:[mBookmarksHelperMenu itemWithTarget:[mCreateBookmarksFolderMenuItem target]
                                                                                   andAction:[mCreateBookmarksFolderMenuItem action]]];
  [mCreateBookmarksSeparatorMenuItem  takeStateFromItem:[mBookmarksHelperMenu itemWithTarget:[mCreateBookmarksSeparatorMenuItem target]
                                                                                   andAction:[mCreateBookmarksSeparatorMenuItem action]]];
  [mShowAllBookmarksMenuItem          takeStateFromItem:[mBookmarksHelperMenu itemWithTarget:[mShowAllBookmarksMenuItem target]
                                                                                   andAction:[mShowAllBookmarksMenuItem action]]];

  // We enable bookmark items themselves from the carbon event handler that fires before the menu is shown.
}

- (NSMenu*)bookmarksMenu
{
  return mBookmarksMenu;
}

- (BOOL)bookmarksItemsEnabled
{
  // since this menu is not in the menu bar, we have to update it by hand
  [mBookmarksHelperMenu update];
  return [[mBookmarksHelperMenu itemWithTarget:self andAction:@selector(openMenuBookmark:)] isEnabled];
}

- (void)adjustBookmarkMenuItems
{
  BOOL enableItems = [self bookmarksItemsEnabled];

  int firstBookmarkItem = [mBookmarksMenu indexOfItemWithTag:kBookmarksDividerTag] + 1;
  [mBookmarksMenu setAllItemsEnabled:enableItems startingWithItemAtIndex:firstBookmarkItem includingSubmenus:YES];
}

- (NSView*)getSavePanelView
{
  if (!mFilterView)
    [NSBundle loadNibNamed:@"AccessoryViews" owner:self];
  return mFilterView;
}

-(NSWindow*)getFrontmostBrowserWindow
{
  // for some reason, [NSApp mainWindow] doesn't always work, so we have to
  // do this manually
  NSArray *windowList = [NSApp orderedWindows];
  NSWindow *foundWindow = nil;

  for (unsigned int i = 0; i < [windowList count]; i ++) {
    NSWindow*	thisWindow = [windowList objectAtIndex:i];
    
    // not all browser windows are created equal. We only consider those with
    // an empty chrome mask, or ones with a toolbar, status bar, and resize control
    // to be real top-level browser windows for purposes of saving size and 
    // loading urls in. Others are popups and are transient.
    if ([[thisWindow windowController] isMemberOfClass:[BrowserWindowController class]]) {
      unsigned int chromeMask = [[thisWindow windowController] chromeMask];
      if (chromeMask == 0 || 
            (chromeMask & nsIWebBrowserChrome::CHROME_TOOLBAR &&
             chromeMask & nsIWebBrowserChrome::CHROME_STATUSBAR &&
             chromeMask & nsIWebBrowserChrome::CHROME_WINDOW_RESIZE)) {
        foundWindow = thisWindow;
        break;
      }
    }
  }

  return foundWindow;
}

- (BrowserWindowController*)openBrowserWindowWithURL:(NSString*)aURL andReferrer:(NSString*)aReferrer behind:(NSWindow*)window allowPopups:(BOOL)inAllowPopups
{
  BrowserWindowController* browser = [[BrowserWindowController alloc] initWithWindowNibName: @"BrowserWindow"];

  if (window)
  {
    BrowserWindow* browserWin = [browser window];
    [browserWin setSuppressMakeKeyFront:YES];	// prevent gecko focus bringing the window to the front
    [browserWin orderWindow: NSWindowBelow relativeTo: [window windowNumber]];
    [browserWin setSuppressMakeKeyFront:NO];
  }
  else
  {
    [browser showWindow: self];
  }

  // The process of creating a new tab in this brand new window loads about:blank for us as a 
  // side effect of calling GetDocument(). We don't need to do it again.
  if ([MainController isBlankURL:aURL])
    [browser disableLoadPage];
  else
    [browser loadURL: aURL referrer:aReferrer activate:YES allowPopups:inAllowPopups];

  return browser;
}

- (BrowserWindowController*)openBrowserWindowWithURLs:(NSArray*)urlArray behind:(NSWindow*)window allowPopups:(BOOL)inAllowPopups
{
  BrowserWindowController* browser = [[BrowserWindowController alloc] initWithWindowNibName: @"BrowserWindow"];

  if (window)
  {
    BrowserWindow* browserWin = [browser window];
    [browserWin setSuppressMakeKeyFront:YES];	// prevent gecko focus bringing the window to the front
    [browserWin orderWindow: NSWindowBelow relativeTo: [window windowNumber]];
    [browserWin setSuppressMakeKeyFront:NO];
  }
  else
  {
    [browser showWindow: self];
  }

  [browser openURLArray:urlArray tabOpenPolicy:eReplaceTabs allowPopups:inAllowPopups];
  return browser;
}

// open a new URL, observing the prefs on how to behave
- (void)openNewWindowOrTabWithURL:(NSString*)inURLString andReferrer: (NSString*)aReferrer
{
  // make sure we're initted
  [PreferenceManager sharedInstance];
  
  PRInt32 reuseWindow = 0;
  PRBool loadInBackground = PR_FALSE;
  
  nsCOMPtr<nsIPref> prefService ( do_GetService(NS_PREF_CONTRACTID) );
  if ( prefService ) {
    prefService->GetIntPref("browser.reuse_window", &reuseWindow);
    prefService->GetBoolPref("browser.tabs.loadInBackground", &loadInBackground);
  }
  
  // reuse the main window (if there is one) based on the pref in the 
  // tabbed browsing panel. The user may have closed all of 
  // them or we may get this event at startup before we've had time to load
  // our window.
  BrowserWindowController* controller = (BrowserWindowController*)[[self getFrontmostBrowserWindow] windowController];
  if (reuseWindow > kOpenNewWindowOnAE && controller) {
    if (reuseWindow == kOpenNewTabOnAE) {
      [controller openNewTabWithURL:inURLString referrer:aReferrer loadInBackground:loadInBackground allowPopups:NO];
    }
    else
      [controller loadURL: inURLString referrer:nil activate:YES allowPopups:NO];
  }
  else {
    // should use BrowserWindowController openNewWindowWithURL, but that method
    // really needs to be on the MainController
    controller = [self openBrowserWindowWithURL: inURLString andReferrer:aReferrer behind:nil allowPopups:NO];
  }
  
  [[[controller getBrowserWrapper] getBrowserView] setActive: YES];
}

// Bookmarks menu actions.
-(IBAction) importBookmarks:(id)aSender
{
  [[BookmarkManager sharedBookmarkManager] startImportBookmarks];
}

// helper for exportBookmarks function
-(IBAction) setFileExtension:(id)aSender
{
  if ([[aSender title] isEqualToString:@"HTML"])
    [[aSender representedObject] setRequiredFileType:@"html"];
  else
    [[aSender representedObject] setRequiredFileType:@"plist"];
}

-(IBAction) exportBookmarks:(id)aSender
{
  NSSavePanel* savePanel = [NSSavePanel savePanel];
  [savePanel setPrompt:NSLocalizedString(@"Export", @"Export")];
  [savePanel setRequiredFileType:@"html"];
  
  // get an accessory view for HTML or Safari .plist output
  if (!mExportPanelView)
    [NSBundle loadNibNamed:@"AccessoryViews" owner:self];
  NSPopUpButton *button = [mExportPanelView viewWithTag:1001];
  [[button itemAtIndex:0] setRepresentedObject:savePanel];
  [[button itemAtIndex:1] setRepresentedObject:savePanel];
  [savePanel setAccessoryView:mExportPanelView];
  
  // start the save panel
  int saveResult = [savePanel runModalForDirectory:nil file:NSLocalizedString(@"ExportedBookmarkFile", @"Exported Bookmarks")];
  int selectedButton = [button indexOfSelectedItem];
  if (saveResult != NSFileHandlingPanelOKButton)
    return;
  if (0 == selectedButton)
    [[BookmarkManager sharedBookmarkManager] writeHTMLFile:[savePanel filename]];
  else
    [[BookmarkManager sharedBookmarkManager] writeSafariFile:[savePanel filename]];
}

-(IBAction) openMenuBookmark:(id)aSender
{
  BookmarkItem*  item = [aSender representedObject];
  [self loadBookmark:item withWindowController:[self getMainWindowBrowserController] openBehavior:eBookmarkOpenBehaviorDefault];
}

//
// -showHistory:
//
// show the history in the bookmark manager. Creates a new window if
// one isn't already there. history isn't a toggle, hence the name.
//
-(IBAction) showHistory:(id)aSender
{
  NSWindow* browserWindow = [self getFrontmostBrowserWindow];
  if (!browserWindow) {
    [self newWindow:self];
    browserWindow = [mApplication mainWindow];
  }

  [[browserWindow windowController] manageHistory: aSender];
}

//
// -clearHistory:
//
// clear the global history, after showing a warning
//
-(IBAction)clearHistory:(id)aSender
{
  if (NSRunCriticalAlertPanel(NSLocalizedString(@"ClearHistoryTitle", nil),
                              NSLocalizedString(@"ClearHistoryMessage", nil),
                              NSLocalizedString(@"ClearHistoryButton", nil),
                              NSLocalizedString(@"CancelButtonText", nil),
                              nil) == NSAlertDefaultReturn)
  {
    // clear history
    nsCOMPtr<nsIBrowserHistory> hist = do_GetService("@mozilla.org/browser/global-history;2");
    if (hist)
      hist->RemoveAllPages();
  }
}

//
// manageBookmarks:
//
// toggle the bookmark manager (creating a new window if needed)
//
-(IBAction)manageBookmarks:(id)aSender
{
  NSWindow* browserWindow = [self getFrontmostBrowserWindow];
  if (!browserWindow) {
    [self newWindow:self];
    browserWindow = [mApplication mainWindow];
  }

  [[browserWindow windowController] manageBookmarks: aSender];
}

- (void)displayPreferencesWindow:(id)sender
{
  [[MVPreferencesController sharedInstance] showPreferences:nil];
}

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
  // We can get called before -applicationDidFinishLaunching, so make sure gecko
  // has been initted
  [self ensureGeckoInitted];
  
  NSURL* urlToOpen = [MainController decodeLocalFileURL:[NSURL fileURLWithPath:filename]];
  [self openNewWindowOrTabWithURL:[urlToOpen absoluteString] andReferrer:nil];
  return YES;    
}

- (IBAction)biggerTextSize:(id)aSender
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
    [browserController biggerTextSize:aSender];
}

- (IBAction)smallerTextSize:(id)aSender
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
    [browserController smallerTextSize:aSender];
}

-(IBAction) viewSource:(id)aSender
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
    [browserController viewPageSource: aSender];		// top-level page, not focussed frame
}

-(BOOL)isMainWindowABrowserWindow
{
  // see also getFrontmostBrowserWindow. That will always return a browser
  // window if one exists. This will only return one if it is frontmost.
  return [[[mApplication mainWindow] windowController] isMemberOfClass:[BrowserWindowController class]];
}

- (BrowserWindowController*)getMainWindowBrowserController
{
  // note that [NSApp mainWindow] will return NULL if we are not
  // frontmost
  NSWindowController* mainWindowController = [[mApplication mainWindow] windowController];
  if (mainWindowController && [mainWindowController isMemberOfClass:[BrowserWindowController class]])
    return (BrowserWindowController*)mainWindowController;
  
  return nil;
}

- (void)adjustCloseWindowMenuItemKeyEquivalent:(BOOL)inHaveTabs
{
  // capitalization of the key equivalent affects whether the shift modifer is used.
  [mCloseWindowMenuItem setKeyEquivalent: inHaveTabs ? @"W" : @"w"];
}

- (void)adjustCloseTabMenuItemKeyEquivalent:(BOOL)inHaveTabs
{
  if (inHaveTabs) {
    [mCloseTabMenuItem setKeyEquivalent:@"w"];
    [mCloseTabMenuItem setKeyEquivalentModifierMask:NSCommandKeyMask];
  }
  else {
    [mCloseTabMenuItem setKeyEquivalent:@""];
    [mCloseTabMenuItem setKeyEquivalentModifierMask:0];
  }
}

// see if we have a window with tabs open, and adjust the key equivalents for
// Close Tab/Close Window accordingly
- (void)fixCloseMenuItemKeyEquivalents
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController) {
    BOOL windowWithMultipleTabs = ([[browserController getTabBrowser] numberOfTabViewItems] > 1);    
    [self adjustCloseWindowMenuItemKeyEquivalent:windowWithMultipleTabs];
    [self adjustCloseTabMenuItemKeyEquivalent:windowWithMultipleTabs];
  }
}

- (void)adjustTextEncodingMenu
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (browserController)
  {
    // enable all items
    [mTextEncodingsMenu setAllItemsEnabled:YES startingWithItemAtIndex:0 includingSubmenus:YES];

    NSString* charset = [browserController currentCharset];
#if DEBUG_CHARSET
    NSLog(@"charset is %@", charset);
#endif
    NSNumber* tag = [mCharsets objectForKey:[charset lowercaseString]];
    [mTextEncodingsMenu checkItemWithTag:[tag intValue] uncheckingOtherItems:YES];
  }
  else
  {
    [mTextEncodingsMenu setAllItemsEnabled:NO startingWithItemAtIndex:0 includingSubmenus:YES];
    // always enable the autodetect item
    [[mTextEncodingsMenu itemWithTag:kEncodingMenuAutodetectItemTag] setEnabled:YES];
  }
  
  // update the state of the autodetect item
  NSString* detectorValue = [[PreferenceManager sharedInstance] getStringPref:"intl.charset.detector" withSuccess:NULL];
  BOOL universalChardetOn = [detectorValue isEqualToString:@"universal_charset_detector"];
  [[mTextEncodingsMenu itemWithTag:kEncodingMenuAutodetectItemTag] setState:universalChardetOn ? NSOnState : NSOffState];
}

-(BOOL)validateMenuItem:(NSMenuItem*)aMenuItem
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];

  // disable items that aren't relevant if there's no main browser window open
  // or the bookmark/history manager is open
  SEL action = [aMenuItem action];

  // NSLog(@"MainController validateMenuItem for %@ (%s)", [aMenuItem title], action);
  
  // XXX some of these should be wired to the First Responder (like reload:).
  if (action == @selector(printPage:) ||
        /* ... many more items go here ... */
        /* action == @selector(goHome:) || */			// always enabled
        /* action == @selector(doSearch:) || */		// always enabled
        action == @selector(pageSetup:) ||
        action == @selector(findInPage:) ||
        action == @selector(doReload:) ||
        action == @selector(savePage:))
  {
    if (browserController != nil)
      return (![browserController bookmarkManagerIsVisible]);
    return NO;
  }
    
  // check what the state of the personal toolbar should be, but only if there is a browser
  // window open. Popup windows that have the personal toolbar removed should always gray
  // out this menu.
  if (action == @selector(toggleBookmarksToolbar:)) {
    if (browserController) {
      NSView* bookmarkToolbar = [browserController bookmarkToolbar];
      if ( bookmarkToolbar ) {
        float height = [bookmarkToolbar frame].size.height;
        BOOL toolbarShowing = (height > 0);
        if (toolbarShowing)
          [mBookmarksToolbarMenuItem setTitle: NSLocalizedString(@"Hide Bookmarks Toolbar",@"")];
        else
          [mBookmarksToolbarMenuItem setTitle: NSLocalizedString(@"Show Bookmarks Toolbar",@"")];
        return YES;
      }
      else
        return NO;
    }
    else
      return NO;
  }

  if ( action == @selector(getInfo:) )
    return (browserController && [browserController canGetInfo]);

  // only enable newTab if there is a browser window frontmost, or if there is no window
  // (i.e. disable it for non-browser windows).
  if (action == @selector(newTab:))
  {
    return (browserController != nil) || ([NSApp mainWindow] == nil);
  }
  
  // only activate if we've got multiple tabs open.
  if ((action == @selector(closeTab:) ||
       action == @selector(nextTab:) ||
       action == @selector(previousTab:)))
  {
    return (browserController && [[browserController getTabBrowser] numberOfTabViewItems] > 1);
  }
  
#if 0
  if (action == @selector(addBookmark:))
    return (browserController && ![[browserController getBrowserWrapper] isEmpty]);
#endif
  
  if (action == @selector(biggerTextSize:))
    return (browserController &&
            ![[browserController getBrowserWrapper] isEmpty] &&
            [[[browserController getBrowserWrapper] getBrowserView] canMakeTextBigger]);

  if (action == @selector(smallerTextSize:))
    return (browserController &&
            ![[browserController getBrowserWrapper] isEmpty] &&
            [[[browserController getBrowserWrapper] getBrowserView] canMakeTextSmaller]);
  
  if (action == @selector(viewSource:))
    return (browserController &&
            ![browserController bookmarkManagerIsVisible]);

  if (action == @selector(doStop:))
    return (browserController && [[browserController getBrowserWrapper] isBusy]);

  if (action == @selector(goBack:) || action == @selector(goForward:)) {
    if (browserController) {
      CHBrowserView* browserView = [[browserController getBrowserWrapper] getBrowserView];
      if (action == @selector(goBack:))
        return [browserView canGoBack];
      if (action == @selector(goForward:))
        return [browserView canGoForward];
    }
    else
      return NO;
  }
  
  if (action == @selector(manageBookmarks:))
  {
    BOOL showingBookmarks = (browserController && [browserController bookmarkManagerIsVisible]);
    NSString* showBMLabel = showingBookmarks ? NSLocalizedString(@"Hide All Bookmarks", @"")
                                             : NSLocalizedString(@"Show All Bookmarks", @"");
    [aMenuItem setTitle:showBMLabel];
    return showingBookmarks ? [browserController canHideBookmarks] : YES;
  }
  
  if (action == @selector(sendURL:))
  {
    NSString* urlString = [[[self getMainWindowBrowserController] getBrowserWrapper] location];
    return ![MainController isBlankURL:urlString];
  }

  // default return
  return YES;
}

-(IBAction) toggleBookmarksToolbar:(id)aSender
{
  BrowserWindowController* browserController = [self getMainWindowBrowserController];
  if (!browserController) return;

  float height = [[browserController bookmarkToolbar] frame].size.height;
  BOOL showToolbar = (BOOL)(!(height > 0));
  
  [[browserController bookmarkToolbar] showBookmarksToolbar: showToolbar];
  
  // save prefs here
  NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
  [defaults setInteger: ((showToolbar) ? 0 : 1) forKey: USER_DEFAULTS_HIDE_PERS_TOOLBAR_KEY];
}

-(IBAction) supportLink:(id)aSender;
{
  NSString* pageToLoad = NSLocalizedStringFromTable(@"SupportPageDefault", @"WebsiteDefaults", nil);
  if (![pageToLoad isEqualToString:@"SupportPageDefault"])
    [self openNewWindowOrTabWithURL:pageToLoad andReferrer:nil];
}

-(IBAction) infoLink:(id)aSender
{
  NSString* pageToLoad = NSLocalizedStringFromTable(@"InfoPageDefault", @"WebsiteDefaults", nil);
  if (![pageToLoad isEqualToString:@"InfoPageDefault"])
    [self openNewWindowOrTabWithURL:pageToLoad andReferrer:nil];
}

-(IBAction) feedbackLink:(id)aSender
{
  NSString *pageToLoad = NSLocalizedStringFromTable(@"FeedbackPageDefault", @"WebsiteDefaults", nil);
  if (![pageToLoad isEqualToString:@"FeedbackPageDefault"])
    [self openNewWindowOrTabWithURL:pageToLoad andReferrer:nil];
}

-(IBAction) releaseNoteLink:(id)aSender
{
  NSString *pageToLoad = NSLocalizedStringFromTable(@"ReleaseNotesDefault", @"WebsiteDefaults", nil);
  if (![pageToLoad isEqualToString:@"ReleaseNotesDefault"])
    [self openNewWindowOrTabWithURL:pageToLoad andReferrer:nil];
}

-(IBAction) tipsTricksLink:(id)aSender
{
  NSString *pageToLoad = NSLocalizedStringFromTable(@"TipsTricksPageDefault", @"WebsiteDefaults", nil);
  if (![pageToLoad isEqualToString:@"TipsTricksPageDefault"])
    [self openNewWindowOrTabWithURL:pageToLoad andReferrer:nil];
}

-(IBAction) searchCustomizeLink:(id)aSender
{
  NSString *pageToLoad = NSLocalizedStringFromTable(@"SearchCustomPageDefault", @"WebsiteDefaults", nil);
  if (![pageToLoad isEqualToString:@"SearchCustomPageDefault"])
    [self openNewWindowOrTabWithURL:pageToLoad andReferrer:nil];
}

-(IBAction) aboutPlugins:(id)aSender
{
  [self openNewWindowOrTabWithURL:@"about:plugins" andReferrer:nil];
}

+ (NSImage*)createImageForDragging:(NSImage*)aIcon title:(NSString*)aTitle
{
  NSImage* image;
  NSSize titleSize, imageSize;
  NSRect imageRect;
  NSDictionary* stringAttrs;
  
  // get the size of the new image we are creating
  titleSize = [aTitle sizeWithAttributes:nil];
  imageSize = NSMakeSize(titleSize.width + [aIcon size].width,
                         titleSize.height > [aIcon size].height ? 
                            titleSize.height : [aIcon size].height);
                        
  // create the image and lock drawing focus on it
  image = [[[NSImage alloc] initWithSize:imageSize] autorelease];
  [image lockFocus];
  
  // draw the image and title in image with translucency
  imageRect = NSMakeRect(0,0, [aIcon size].width, [aIcon size].height);
  [aIcon drawAtPoint: NSMakePoint(0,0) fromRect: imageRect operation:NSCompositeCopy fraction:0.8];
  
  stringAttrs = [NSDictionary dictionaryWithObject: [[NSColor textColor] colorWithAlphaComponent:0.8]
                  forKey: NSForegroundColorAttributeName];
  [aTitle drawAtPoint: NSMakePoint([aIcon size].width, 0) withAttributes: stringAttrs];
  
  [image unlockFocus]; 
  
  return image;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)theApp hasVisibleWindows:(BOOL)flag
{
  // we might be sitting there with the "there is another copy of camino running" dialog up
  // (which means we're in a modal loop in [PreferenceManager init]). So if we haven't
  // finished initting prefs yet, just bail.
  if (![PreferenceManager sharedInstanceDontCreate])
    return NO;

  // ignore |hasVisibleWindows| because we always want to show a browser window when
  // the user clicks on the app icon, even if, say, prefs or the d/l window are open.
  // If there is no browser, create one. If there is one, unminimize it if it's in the dock.
  NSWindow* frontBrowser = [self getFrontmostBrowserWindow];
  if ( !frontBrowser )
    [self newWindow:self];
  else if ([frontBrowser isMiniaturized])
    [frontBrowser deminiaturize:self];
  
  return NO;
}

- (void) applicationDidChangeScreenParameters:(NSNotification *)aNotification
{
  [NSApp makeWindowsPerform:@selector(display) inOrder:YES];
}

- (void)applicationDidBecomeActive:(NSNotification *)aNotification
{
  [mFindDialog applicationWasActivated];
}

//
// -aboutWindow:
//
// Show the (slightly modified) standard about window, with the build id instead of the
// typical application version. It'll display like "2003120403 (v0.7+)". 
//
-(IBAction)aboutWindow:(id)sender
{
  NSString* version = [NSString stringWithFormat:@"%010u", NS_BUILD_ID];
  NSDictionary* d = [NSDictionary dictionaryWithObject:version forKey:@"ApplicationVersion"];
  [NSApp orderFrontStandardAboutPanelWithOptions:d];
}

// services

- (void)openURL:(NSPasteboard *) pboard userData:(NSString *) userData error:(NSString **) error
{
  NSMutableString *urlString = [[[NSMutableString alloc] init] autorelease];
  if (!urlString)
    return;

  NSArray* types = [pboard types];
  if (![types containsObject:NSStringPboardType]) {
    *error = NSLocalizedString(@"Error: couldn't open URL.",
                    @"pboard couldn't give URL string.");
    return;
  }
  NSString* pboardString = [pboard stringForType:NSStringPboardType];
  if (!pboardString) {
    *error = NSLocalizedString(@"Error: couldn't open URL.",
                    @"pboard couldn't give URL string.");
    return;
  }
  NSScanner* scanner = [NSScanner scannerWithString:pboardString];
  [scanner scanCharactersFromSet:[[NSCharacterSet whitespaceAndNewlineCharacterSet] invertedSet] intoString:&urlString];
  while (![scanner isAtEnd]) {
    NSString *tmpString;
    [scanner scanCharactersFromSet:[[NSCharacterSet whitespaceAndNewlineCharacterSet] invertedSet] intoString:&tmpString];
    [urlString appendString:tmpString];
  }
  
  [self openNewWindowOrTabWithURL:urlString andReferrer:nil];
}

//
// -emptyCache:
//
// Puts up a modal panel and if the user gives the go-ahead, emtpies the disk and memory 
// caches. We keep this separate from |-resetBrowser:| so the user can just clear the cache
// and not have to delete everything (such as their keychain passwords).
//
- (IBAction)emptyCache:(id)sender
{
  if (NSRunCriticalAlertPanel(NSLocalizedString(@"EmptyCacheTitle", nil), NSLocalizedString(@"EmptyCacheMessage", nil),
         NSLocalizedString(@"EmptyButton", nil), NSLocalizedString(@"CancelButtonText", nil), nil) == NSAlertDefaultReturn) {
    // remove cache
    nsCOMPtr<nsICacheService> cacheServ (do_GetService("@mozilla.org/network/cache-service;1"));
    if (cacheServ)
      cacheServ->EvictEntries(nsICache::STORE_ANYWHERE);
  }
}

//
// -resetBrowser:
//
// Here we need to:
// - warn user about what is going to happen
// - if its OK...
// - close all open windows, delete cache, history, cookies, site permissions,
//    downloads, saved names and passwords
//
- (IBAction)resetBrowser:(id)sender
{
  if (NSRunCriticalAlertPanel(NSLocalizedString(@"Reset Camino Title", @"Are you sure you want to reset Camino?"),
                      NSLocalizedString(@"Reset Warning Message",
                      @"Resetting Camino will erase your browsing history, empty the cache, clear downloads, clear all cookies, clear all site permissions, and remove all remembered usernames and passwords. This action cannot be undone."),
                      NSLocalizedString(@"Reset Camino", @"Reset Camino"),
                      NSLocalizedString(@"CancelButtonText", @"Cancel"),
                      nil) == NSAlertDefaultReturn) {
    
    // close all windows
    NSArray *windows = [NSApp orderedWindows];
    for (unsigned int i = 0; i < [windows count]; i++) {
      [[windows objectAtIndex:i] performClose:self];
    }
    
    // remove cache
    nsCOMPtr<nsICacheService> cacheServ (do_GetService("@mozilla.org/network/cache-service;1"));
    if (cacheServ)
      cacheServ->EvictEntries(nsICache::STORE_ANYWHERE);

    // remove cookies
    nsCOMPtr<nsICookieManager> cm(do_GetService(NS_COOKIEMANAGER_CONTRACTID));
    nsICookieManager* mCookieManager = cm.get();
    if (mCookieManager)
      mCookieManager->RemoveAll();

    // remove site permissions
    nsCOMPtr<nsIPermissionManager> pm(do_GetService(NS_PERMISSIONMANAGER_CONTRACTID));
    nsIPermissionManager* mPermissionManager = pm.get();
    if (mPermissionManager)
      mPermissionManager->RemoveAll();

    // remove history
    nsCOMPtr<nsIBrowserHistory> hist (do_GetService("@mozilla.org/browser/global-history;2"));
    if (hist)
      hist->RemoveAllPages();

    // remove downloads
    [[ProgressDlgController sharedDownloadController] clearAllDownloads];

    // remove saved names and passwords
    [[KeychainService instance] removeAllUsernamesAndPasswords];
    
    // open a new window
    [self newWindow:self];
  }
}

#pragma mark -


static int SortByProtocolAndName(NSDictionary* item1, NSDictionary* item2, void *context)
{
  NSComparisonResult protocolCompare = [[item1 objectForKey:@"name"] compare:[item2 objectForKey:@"name"] options:NSCaseInsensitiveSearch];
  if (protocolCompare != NSOrderedSame)
    return protocolCompare;
    
  return [[item1 objectForKey:@"protocol"] compare:[item2 objectForKey:@"protocol"] options:NSCaseInsensitiveSearch];
}

// NetworkServicesClient implementation

- (void)availableServicesChanged:(NSNotification *)note
{
  // rebuild the submenu, leaving the first item
  while ([mServersSubmenu numberOfItems] > 1)
    [mServersSubmenu removeItemAtIndex:[mServersSubmenu numberOfItems] - 1];

  NetworkServices *netserv = [note object];

  NSEnumerator* keysEnumerator = [netserv serviceEnumerator];
  // build an array of dictionaries, so we can sort it

  NSMutableArray* servicesArray = [[NSMutableArray alloc] initWithCapacity:10];

  id key;
  while ((key = [keysEnumerator nextObject])) {
    NSDictionary* serviceDict = [NSDictionary dictionaryWithObjectsAndKeys:
      key, @"id",
      [netserv serviceName:[key intValue]], @"name",
      [netserv serviceProtocol:[key intValue]], @"protocol", nil];

    [servicesArray addObject:serviceDict];
  }

  if ([servicesArray count] == 0) {
    // add a separator
    [mServersSubmenu addItem:[NSMenuItem separatorItem]];

    NSMenuItem* newItem = [mServersSubmenu addItemWithTitle:NSLocalizedString(@"NoServicesFound", @"") action:nil keyEquivalent:@""];
    [newItem setTag:-1];
    [newItem setTarget:self];
  }
  else {
    // add a separator
    [mServersSubmenu addItem:[NSMenuItem separatorItem]];
    
    // sort on protocol, then name
    [servicesArray sortUsingFunction:SortByProtocolAndName context:NULL];
    
    unsigned count = [servicesArray count];
    for (unsigned int i = 0; i < count; i++) {
      NSDictionary* serviceDict = [servicesArray objectAtIndex:i];
      NSString* itemName = [[serviceDict objectForKey:@"name"] stringByAppendingString:NSLocalizedString([serviceDict objectForKey:@"protocol"], @"")];
      
      id newItem = [mServersSubmenu addItemWithTitle:itemName action:@selector(connectToServer:) keyEquivalent:@""];
      [newItem setTag:[[serviceDict objectForKey:@"id"] intValue]];
      [newItem setTarget:self];
    }
  }
  // when you alloc, you've got to release . . .
  [servicesArray release];
}

- (void)serviceResolved:(NSNotification *)note
{
  NSDictionary *dict = [note userInfo];
  if ([dict objectForKey:NetworkServicesClientKey] == self)
    [self openNewWindowOrTabWithURL:[dict objectForKey:NetworkServicesResolvedURLKey] andReferrer:nil];
}

//
// handles resolution failure for everybody else
//
- (void)serviceResolutionFailed:(NSNotification *)note
{
  NSDictionary *dict = [note userInfo];
  NSString* serviceName = [dict objectForKey:NetworkServicesServiceKey];
  NSBeginAlertSheet(NSLocalizedString(@"ServiceResolutionFailedTitle", @""),
                    @"",               // default button
                    nil,               // cancel buttton
                    nil,               // other button
                    [NSApp mainWindow],                // window
                    nil,               // delegate
                    nil,               // end sel
                    nil,               // dismiss sel
                    NULL,              // context
                    [NSString stringWithFormat:NSLocalizedString(@"ServiceResolutionFailedMsgFormat", @""), serviceName]
                    );
}

-(IBAction) connectToServer:(id)aSender
{
  [[NetworkServices sharedNetworkServices] attemptResolveService:[aSender tag] forSender:self];
}

-(IBAction) aboutServers:(id)aSender
{
  NSString *pageToLoad = NSLocalizedStringFromTable(@"RendezvousPageDefault", @"WebsiteDefaults", nil);
  if (![pageToLoad isEqualToString:@"RendezvousPageDefault"])
    [self openNewWindowOrTabWithURL:pageToLoad andReferrer:nil];
}

@end
