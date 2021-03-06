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
 *   David Hyatt <hyatt@mozilla.org> (Original Author)
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

#import "NSString+Utils.h"
#import "CHBrowserView.h"

#include "nsCOMPtr.h"
#include "nsString.h"
#include "CHClickListener.h"
#include "nsIDOMEventTarget.h"
#include "nsIContent.h"
#include "nsIDOMElement.h"
#include "nsIDOMMouseEvent.h"
#include "nsEmbedAPI.h"
#include "nsIDOMDocument.h"
#include "nsIDOMDocumentEvent.h"
#include "nsIDOMEventTarget.h"
#include "nsIDOMNSHTMLElement.h"
#include "nsIDOMHTMLSelectElement.h"
#include "nsIDOMHTMLOptionElement.h"
#include "nsIDOMHTMLOptionsCollection.h"
#include "nsIDOMHTMLOptGroupElement.h"
#include "nsIDOMWindow.h"
#include "nsIScriptGlobalObject.h"
#include "nsIDocument.h"


@interface CHOptionSelector : NSObject
{
  nsIDOMHTMLSelectElement* mSelectElt;
}

-(id)initWithSelect:(nsIDOMHTMLSelectElement*)aSel;
-(IBAction)selectOption:(id)aSender;

@end

@implementation CHOptionSelector

-(id)initWithSelect:(nsIDOMHTMLSelectElement*)aSel
{
  if ( (self = [super init]) ) {
    mSelectElt = aSel;
  }
  return self;
}

-(IBAction)selectOption:(id)aSender
{
  nsIDOMHTMLOptionElement* optionElt = (nsIDOMHTMLOptionElement*)[[aSender representedObject] pointerValue];
  NS_ASSERTION(optionElt, "Missing option element");
  if (!optionElt) return;

  optionElt->SetSelected(PR_TRUE);

  // Fire a DOM event for the title change.
  nsCOMPtr<nsIDOMEvent> event;
  nsCOMPtr<nsIDOMDocument> domDocument;
  mSelectElt->GetOwnerDocument(getter_AddRefs(domDocument));
  nsCOMPtr<nsIDOMDocumentEvent> docEvent(do_QueryInterface(domDocument));
  
  docEvent->CreateEvent(NS_LITERAL_STRING("Events"), getter_AddRefs(event));
  if (event) {
    event->InitEvent(NS_LITERAL_STRING("change"), PR_TRUE, PR_TRUE);
    PRBool noDefault;
    nsCOMPtr<nsIDOMEventTarget> target(do_QueryInterface(mSelectElt));
    target->DispatchEvent(event, &noDefault);
  }  
}

@end


NS_IMPL_ISUPPORTS2(CHClickListener, nsIDOMMouseListener, nsIDOMEventListener)

CHClickListener::CHClickListener()
{
}

CHClickListener::~CHClickListener()
{
}

NS_IMETHODIMP
CHClickListener::MouseDown(nsIDOMEvent* aEvent)
{
  nsCOMPtr<nsIDOMMouseEvent> mouseEvent(do_QueryInterface(aEvent));
  if (!mouseEvent) return NS_ERROR_FAILURE;
  
  PRUint16 button;
  mouseEvent->GetButton(&button);
  // only show popup on left button
  if (button != 0)
    return NS_OK;

  nsCOMPtr<nsIDOMEventTarget> target;
  mouseEvent->GetTarget(getter_AddRefs(target));
  if (!target)
    return NS_OK;

  nsCOMPtr<nsIDOMHTMLSelectElement> sel(do_QueryInterface(target));
  if (!sel)
    return NS_OK;

  PRInt32 size = 0;
  sel->GetSize(&size);
  PRBool multiple = PR_FALSE;
  sel->GetMultiple(&multiple);
  if(size > 1 || multiple)
    return NS_OK;

  // the call to popUpContextMenu: is synchronous so we don't need to
  // worry about retaining the menu for later.
  NSMenu* menu = [[[NSMenu alloc] init] autorelease];

  // We'll set the disabled state as the options are created, so disable
  // auto-enabling via NSMenuValidation.
  [menu setAutoenablesItems: NO];

  nsCOMPtr<nsIDOMHTMLOptionsCollection> options;
  sel->GetOptions(getter_AddRefs(options));
  PRUint32 count;
  options->GetLength(&count);
  PRInt32 selIndex = 0;   // currently unused

  nsCOMPtr<nsIDOMHTMLOptGroupElement> curOptGroup;

  for (PRUint32 i = 0; i < count; i++) {
    nsAutoString itemLabel;

    nsCOMPtr<nsIDOMNode> node;
    options->Item(i, getter_AddRefs(node));

    nsCOMPtr<nsIDOMHTMLOptionElement> option(do_QueryInterface(node));

    nsCOMPtr<nsIDOMNode> parentNode;
    option->GetParentNode(getter_AddRefs(parentNode));
    nsCOMPtr<nsIDOMHTMLOptGroupElement> parentOptGroup = do_QueryInterface(parentNode);
    if (parentOptGroup && (parentOptGroup != curOptGroup))
    {
      // insert optgroup item
      parentOptGroup->GetLabel(itemLabel);
      NSString* title = [[NSString stringWith_nsAString: itemLabel] stringByTruncatingTo:75 at:kTruncateAtMiddle];
      NSMenuItem* menuItem = [[[NSMenuItem alloc] initWithTitle: title action: NULL keyEquivalent: @""] autorelease];
      [menu addItem: menuItem];
      [menuItem setEnabled: NO];

      curOptGroup = parentOptGroup;
    }

    option->GetLabel(itemLabel);
    if (itemLabel.IsEmpty())
      option->GetText(itemLabel);

    NSString* title = [[NSString stringWith_nsAString: itemLabel] stringByTruncatingTo:75 at:kTruncateAtMiddle];
    
    // indent items in optgroup
    if (parentOptGroup)
      title = [@"  " stringByAppendingString:title];

    NSMenuItem* menuItem = [[[NSMenuItem alloc] initWithTitle: title action: NULL keyEquivalent: @""] autorelease];
    [menu addItem: menuItem];
    [menuItem setRepresentedObject:[NSValue valueWithPointer:option.get()]];

    PRBool selected;
    option->GetSelected(&selected);
    if (selected) {
      [menuItem setState: NSOnState];
      selIndex = i;
    }
    PRBool disabled;
    option->GetDisabled(&disabled);
    if (disabled)
      [menuItem setEnabled: NO];
    CHOptionSelector* optSelector = [[[CHOptionSelector alloc] initWithSelect: sel] autorelease];
    [menuItem setTarget: optSelector];									// retains
    if (!selected)
      [menuItem setAction:@selector(selectOption:)];
  }

  nsCOMPtr<nsIDOMNSHTMLElement> nsSel(do_QueryInterface(sel));
  PRInt32 left, top, height;
  PRInt32 clientX, clientY;
  nsSel->GetOffsetLeft(&left);
  nsSel->GetOffsetTop(&top);
  nsSel->GetOffsetHeight(&height);

  nsCOMPtr<nsIDOMElement> currOffsetParent;
  nsSel->GetOffsetParent(getter_AddRefs(currOffsetParent));
  while (currOffsetParent) {
    nsCOMPtr<nsIDOMNSHTMLElement> currNS(do_QueryInterface(currOffsetParent));
    PRInt32 currLeft, currTop;
    currNS->GetOffsetLeft(&currLeft);
    currNS->GetOffsetTop(&currTop);
    left += currLeft;
    top += currTop;
    currNS->GetOffsetParent(getter_AddRefs(currOffsetParent));
  }
  
  mouseEvent->GetClientX(&clientX);
  mouseEvent->GetClientY(&clientY);

  PRInt32 xDelta = clientX - left;
  PRInt32 yDelta = top + height - clientY;

  nsCOMPtr<nsIContent> selContent = do_QueryInterface(sel);
  nsCOMPtr<nsIDocument> doc = selContent->GetDocument();

  // I'm going to assume that if we got a mousedown for a content node,
  // it's actually in a document.

  nsIScriptGlobalObject* sgo = doc->GetScriptGlobalObject();
  nsCOMPtr<nsIDOMWindow> window = do_QueryInterface(sgo);
  if (!window)
    return NS_OK;

  PRInt32 scrollX, scrollY;
  window->GetScrollX(&scrollX);
  window->GetScrollY(&scrollY);
  xDelta += scrollX; // Normal direction.
  yDelta -= scrollY; // Remember, y is flipped.
  
  const float kMenuWidth = 20.0;               // specify something small so it sizes to fit
  const float kMenuPopupHeight = 20.0;         // height of a popup in aqua
  NSEvent* event = [NSApp currentEvent];
  NSPoint point = [event locationInWindow];
  NSRect bounds = { {point.x - xDelta, point.y - yDelta}, {kMenuWidth, kMenuPopupHeight} };

  NSPopUpButtonCell *cell = [[NSPopUpButtonCell alloc] initTextCell: @"" pullsDown: NO];
  [cell setMenu: menu];
  [cell setFont:[NSFont systemFontOfSize:[NSFont systemFontSize]]];
  [cell trackMouse: event inRect: bounds ofView: [[event window] contentView] untilMouseUp: YES];
  [cell release];

  return NS_OK;
}
