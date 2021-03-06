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
 * The Original Code is Mozilla Communicator client code.
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
#ifndef nsGfxScrollFrame_h___
#define nsGfxScrollFrame_h___

#include "nsHTMLContainerFrame.h"
#include "nsIAnonymousContentCreator.h"
#include "nsBoxFrame.h"
#include "nsIScrollableFrame.h"
#include "nsIScrollPositionListener.h"
#include "nsIStatefulFrame.h"
#include "nsGUIEvent.h"

class nsISupportsArray;
class nsIScrollableView;
class nsPresContext;
class nsIPresShell;
class nsIContent;
class nsIAtom;
class nsIDocument;
class nsIScrollFrameInternal;
class nsPresState;
struct ScrollReflowState;

class nsGfxScrollFrameInner : public nsIScrollPositionListener {
public:
  NS_IMETHOD QueryInterface(REFNSIID aIID, void** aInstancePtr);
  NS_IMETHOD_(nsrefcnt) AddRef(void);
  NS_IMETHOD_(nsrefcnt) Release(void);

  nsGfxScrollFrameInner(nsContainerFrame* aOuter, PRBool aIsRoot);

  typedef nsIScrollableFrame::ScrollbarStyles ScrollbarStyles;
  ScrollbarStyles GetScrollbarStylesFromFrame() const;

  // If a child frame was added or removed on the scrollframe,
  // reload our child frame list.
  // We need this if a scrollbar frame is recreated.
  void ReloadChildFrames();
  PRBool NeedsClipWidget() const;
  void CreateScrollableView();

  void CreateAnonymousContent(nsISupportsArray& aAnonymousChildren);
  void PostScrollPortEvent(PRBool aOverflow, nsScrollPortEvent::orientType aType);

  nsresult GetChildContentAndOffsetsFromPoint(nsPresContext* aCX,
                                              const nsPoint&  aPoint,
                                              nsIContent **   aNewContent,
                                              PRInt32&        aContentOffset,
                                              PRInt32&        aContentOffsetEnd,
                                              PRBool&         aBeginFrameContent);

  // nsIScrollPositionListener

  NS_IMETHOD ScrollPositionWillChange(nsIScrollableView* aScrollable, nscoord aX, nscoord aY);
  NS_IMETHOD ScrollPositionDidChange(nsIScrollableView* aScrollable, nscoord aX, nscoord aY);

  // This gets called when the 'curpos' attribute on one of the scrollbars changes
  void CurPosAttributeChanged(nsIContent* aChild, PRInt32 aModType);

  void SetScrollbarEnabled(nsIBox* aBox, nscoord aMaxPos, PRBool aReflow=PR_TRUE);
  PRBool SetAttribute(nsIBox* aBox, nsIAtom* aAtom, nscoord aSize, PRBool aReflow=PR_TRUE);
  PRInt32 GetIntegerAttribute(nsIBox* aFrame, nsIAtom* atom, PRInt32 defaultValue);

  /**
   * If RTL, then this will scroll to the right during initial layout.
   */
  void AdjustHorizontalScrollbar();
 
  // Like ScrollPositionDidChange, but initiated by this frame rather than from the
  // scrolling view
  void InternalScrollPositionDidChange(nscoord aX, nscoord aY);

  nsIScrollableView* GetScrollableView() const { return mScrollableView; }

  nsIView* GetParentViewForChildFrame(nsIFrame* aFrame) const;

  void ScrollToRestoredPosition();

  nsPresState* SaveState();
  void RestoreState(nsPresState* aState);

  nsIFrame* GetScrolledFrame() const { return mScrolledFrame; }

  void ScrollbarChanged(nsPresContext* aPresContext, nscoord aX, nscoord aY, PRUint32 aFlags);

  static void SetScrollbarVisibility(nsIBox* aScrollbar, PRBool aVisible);

  nsSize GetScrolledSize() const;
  nsMargin GetActualScrollbarSizes() const;
  nsMargin GetDesiredScrollbarSizes(nsBoxLayoutState* aState);
  PRBool IsScrollbarOnRight();
  void LayoutScrollbars(nsBoxLayoutState& aState,
                        const nsRect& aContentArea,
                        const nsRect& aOldScrollArea,
                        const nsRect& aScrollArea);

  nsIScrollableView* mScrollableView;
  nsIBox* mHScrollbarBox;
  nsIBox* mVScrollbarBox;
  nsIFrame* mScrolledFrame;
  nsIBox* mScrollCornerBox;
  nsContainerFrame* mOuter;
  nscoord mOnePixel;

  nsRect mRestoreRect;
  nsPoint mLastPos;

  // The last dir value we saw in AddHorizontalScrollbar.  Use PRInt16
  // so we can fit all the possible values of a PRUint8 and have a -1
  // value that indicates "not set")
  PRInt16     mLastDir;
  
  PRPackedBool mNeverHasVerticalScrollbar:1;
  PRPackedBool mNeverHasHorizontalScrollbar:1;
  PRPackedBool mHasVerticalScrollbar:1;
  PRPackedBool mHasHorizontalScrollbar:1;
  PRPackedBool mViewInitiatedScroll:1;
  PRPackedBool mFrameInitiatedScroll:1;
  PRPackedBool mDidHistoryRestore:1;
  // Is this the scrollframe for the document's viewport?
  PRPackedBool mIsRoot:1;
  PRPackedBool mSupppressScrollbarUpdate:1;
};

/**
 * The scroll frame creates and manages the scrolling view
 *
 * It only supports having a single child frame that typically is an area
 * frame, but doesn't have to be. The child frame must have a view, though
 *
 * Scroll frames don't support incremental changes, i.e. you can't replace
 * or remove the scrolled frame
 */
class nsHTMLScrollFrame : public nsHTMLContainerFrame,
                          public nsIScrollableFrame,
                          public nsIAnonymousContentCreator,
                          public nsIStatefulFrame {
public:
  friend nsresult NS_NewHTMLScrollFrame(nsIPresShell* aPresShell, nsIFrame** aNewFrame, 
                                        PRBool aIsRoot);

  NS_DECL_ISUPPORTS

  // Called to set the child frames. We typically have three: the scroll area,
  // the vertical scrollbar, and the horizontal scrollbar.
  NS_IMETHOD SetInitialChildList(nsPresContext* aPresContext,
                                 nsIAtom*        aListName,
                                 nsIFrame*       aChildList);

  PRBool TryLayout(ScrollReflowState* aState,
                   const nsHTMLReflowMetrics& aKidMetrics,
                   PRBool aAssumeVScroll, PRBool aAssumeHScroll,
                   PRBool aForce);
  nsresult ReflowScrolledFrame(const ScrollReflowState& aState,
                               PRBool aAssumeVScroll,
                               nsHTMLReflowMetrics* aMetrics,
                               PRBool aFirstPass);
  nsresult ReflowContents(ScrollReflowState* aState,
                          const nsHTMLReflowMetrics& aDesiredSize);
  PRBool IsRTLTextControl();
  void PlaceScrollArea(const ScrollReflowState& aState);

   NS_IMETHOD Reflow(nsPresContext*          aPresContext,
                  nsHTMLReflowMetrics&     aDesiredSize,
                  const nsHTMLReflowState& aReflowState,
                  nsReflowStatus&          aStatus);

  // Because there can be only one child frame, these two function return
  // NS_ERROR_FAILURE
  NS_IMETHOD AppendFrames(nsIAtom*        aListName,
                          nsIFrame*       aFrameList);
  NS_IMETHOD InsertFrames(nsIAtom*        aListName,
                          nsIFrame*       aPrevFrame,
                          nsIFrame*       aFrameList);

  NS_IMETHOD ReplaceFrame(nsIAtom*        aListName,
                          nsIFrame*       aOldFrame,
                          nsIFrame*       aNewFrame);

  NS_IMETHOD Destroy(nsPresContext* aPresContext);

  NS_IMETHOD RemoveFrame(nsIAtom*        aListName,
                         nsIFrame*       aOldFrame);


  NS_IMETHOD GetContentAndOffsetsFromPoint(nsPresContext* aCX,
                                           const nsPoint&  aPoint,
                                           nsIContent **   aNewContent,
                                           PRInt32&        aContentOffset,
                                           PRInt32&        aContentOffsetEnd,
                                           PRBool&         aBeginFrameContent) {
    return mInner.GetChildContentAndOffsetsFromPoint(aCX, aPoint, aNewContent, aContentOffset,
                                                     aContentOffsetEnd, aBeginFrameContent);
  }

  virtual nsIView* GetParentViewForChildFrame(nsIFrame* aFrame) const {
    return mInner.GetParentViewForChildFrame(aFrame);
  }

  virtual nsIFrame* GetContentInsertionFrame() {
    return mInner.GetScrolledFrame()->GetContentInsertionFrame();
  }

  virtual nsIView* GetMouseCapturer() const {
    return mInner.GetScrolledFrame()->GetView();
  }

  virtual PRBool NeedsView() { return PR_TRUE; }
  virtual PRBool DoesClipChildren() { return PR_TRUE; }

  // nsIAnonymousContentCreator
  NS_IMETHOD CreateAnonymousContent(nsPresContext* aPresContext,
                                    nsISupportsArray& aAnonymousItems);
  NS_IMETHOD CreateFrameFor(nsPresContext*   aPresContext,
                            nsIContent *      aContent,
                            nsIFrame**        aFrame) { if (aFrame) *aFrame = nsnull; return NS_ERROR_FAILURE; }

  // nsIScrollableFrame
  virtual nsIFrame* GetScrolledFrame() const;
  virtual nsIScrollableView* GetScrollableView();

  virtual nsPoint GetScrollPosition() const;
  virtual void ScrollTo(nsPoint aScrollPosition, PRUint32 aFlags);

  virtual void SetScrollbarVisibility(PRBool aVerticalVisible, PRBool aHorizontalVisible);

  virtual nsIBox* GetScrollbarBox(PRBool aVertical);

  virtual void CurPosAttributeChanged(nsIContent* aChild, PRInt32 aModType);

  // nsIStatefulFrame
  NS_IMETHOD SaveState(nsPresContext* aPresContext, nsPresState** aState) {
    NS_ENSURE_ARG_POINTER(aState);
    *aState = mInner.SaveState();
    return NS_OK;
  }
  NS_IMETHOD RestoreState(nsPresContext* aPresContext, nsPresState* aState) {
    NS_ENSURE_ARG_POINTER(aState);
    mInner.RestoreState(aState);
    return NS_OK;
  }

  virtual void ScrollToRestoredPosition() {
    mInner.ScrollToRestoredPosition();
  }

  virtual nsMargin GetActualScrollbarSizes() const {
    return mInner.GetActualScrollbarSizes();
  }
  virtual nsMargin GetDesiredScrollbarSizes(nsBoxLayoutState* aState);
  virtual nsGfxScrollFrameInner::ScrollbarStyles GetScrollbarStyles() const;

  /**
   * Get the "type" of the frame
   *
   * @see nsLayoutAtoms::scrollFrame
   */
  virtual nsIAtom* GetType() const;
  
#ifdef NS_DEBUG
  NS_IMETHOD GetFrameName(nsAString& aResult) const;
#endif

  virtual nsresult GetContentOf(nsIContent** aContent);

  PRBool DidHistoryRestore() { return mInner.mDidHistoryRestore; }

#ifdef ACCESSIBILITY
  NS_IMETHOD GetAccessible(nsIAccessible** aAccessible);
#endif

protected:
  nsHTMLScrollFrame(nsIPresShell* aShell, PRBool aIsRoot);
  virtual PRIntn GetSkipSides() const;
  
  void SetSuppressScrollbarUpdate(PRBool aSuppress) {
    mInner.mSupppressScrollbarUpdate = aSuppress;
  }

private:
  friend class nsGfxScrollFrameInner;
  nsGfxScrollFrameInner mInner;
};

/**
 * The scroll frame creates and manages the scrolling view
 *
 * It only supports having a single child frame that typically is an area
 * frame, but doesn't have to be. The child frame must have a view, though
 *
 * Scroll frames don't support incremental changes, i.e. you can't replace
 * or remove the scrolled frame
 */
class nsXULScrollFrame : public nsBoxFrame,
                         public nsIScrollableFrame,
                         public nsIAnonymousContentCreator,
                         public nsIStatefulFrame {
public:
  friend nsresult NS_NewXULScrollFrame(nsIPresShell* aPresShell, nsIFrame** aNewFrame, 
                                       PRBool aIsRoot);

  // Called to set the child frames. We typically have three: the scroll area,
  // the vertical scrollbar, and the horizontal scrollbar.
  NS_IMETHOD SetInitialChildList(nsPresContext* aPresContext,
                                 nsIAtom*        aListName,
                                 nsIFrame*       aChildList);

  NS_IMETHOD Reflow(nsPresContext*          aPresContext,
                  nsHTMLReflowMetrics&     aDesiredSize,
                  const nsHTMLReflowState& aReflowState,
                  nsReflowStatus&          aStatus);

  // Because there can be only one child frame, these two function return
  // NS_ERROR_FAILURE
  NS_IMETHOD AppendFrames(nsIAtom*        aListName,
                          nsIFrame*       aFrameList);
  NS_IMETHOD InsertFrames(nsIAtom*        aListName,
                          nsIFrame*       aPrevFrame,
                          nsIFrame*       aFrameList);

  NS_IMETHOD ReplaceFrame(nsIAtom*        aListName,
                          nsIFrame*       aOldFrame,
                          nsIFrame*       aNewFrame);

  NS_IMETHOD Destroy(nsPresContext* aPresContext);

  NS_IMETHOD RemoveFrame(nsIAtom*        aListName,
                         nsIFrame*       aOldFrame);


  NS_IMETHOD GetContentAndOffsetsFromPoint(nsPresContext* aCX,
                                           const nsPoint&  aPoint,
                                           nsIContent **   aNewContent,
                                           PRInt32&        aContentOffset,
                                           PRInt32&        aContentOffsetEnd,
                                           PRBool&         aBeginFrameContent) {
    return mInner.GetChildContentAndOffsetsFromPoint(aCX, aPoint, aNewContent, aContentOffset,
                                                     aContentOffsetEnd, aBeginFrameContent);
  }

  virtual nsIView* GetParentViewForChildFrame(nsIFrame* aFrame) const {
    return mInner.GetParentViewForChildFrame(aFrame);
  }

  virtual nsIFrame* GetContentInsertionFrame() {
    return mInner.GetScrolledFrame()->GetContentInsertionFrame();
  }

  virtual nsIView* GetMouseCapturer() const {
    return mInner.GetScrolledFrame()->GetView();
  }

  virtual PRBool NeedsView() { return PR_TRUE; }
  virtual PRBool DoesClipChildren() { return PR_TRUE; }

  // nsIAnonymousContentCreator
  NS_IMETHOD CreateAnonymousContent(nsPresContext* aPresContext,
                                    nsISupportsArray& aAnonymousItems);
  NS_IMETHOD CreateFrameFor(nsPresContext*   aPresContext,
                            nsIContent *      aContent,
                            nsIFrame**        aFrame) { if (aFrame) *aFrame = nsnull; return NS_ERROR_FAILURE; }

  // nsIBox methods
  NS_DECL_ISUPPORTS

  NS_IMETHOD GetPrefSize(nsBoxLayoutState& aBoxLayoutState, nsSize& aSize);
  NS_IMETHOD GetMinSize(nsBoxLayoutState& aBoxLayoutState, nsSize& aSize);
  NS_IMETHOD GetMaxSize(nsBoxLayoutState& aBoxLayoutState, nsSize& aSize);
  NS_IMETHOD GetAscent(nsBoxLayoutState& aBoxLayoutState, nscoord& aAscent);

  NS_IMETHOD DoLayout(nsBoxLayoutState& aBoxLayoutState);
  NS_IMETHOD GetPadding(nsMargin& aPadding);

  nsresult Layout(nsBoxLayoutState& aState);
  void LayoutScrollArea(nsBoxLayoutState& aState, const nsRect& aRect);

  static PRBool AddRemoveScrollbar(PRBool& aHasScrollbar, 
                                   nscoord& aXY, 
                                   nscoord& aSize, 
                                   nscoord aSbSize, 
                                   PRBool aOnRightOrBottom, 
                                   PRBool aAdd);
  
  PRBool AddRemoveScrollbar(nsBoxLayoutState& aState, 
                            nsRect& aScrollAreaSize, 
                            PRBool aOnTop, 
                            PRBool aHorizontal, 
                            PRBool aAdd);
  
  PRBool AddHorizontalScrollbar (nsBoxLayoutState& aState, nsRect& aScrollAreaSize, PRBool aOnBottom);
  PRBool AddVerticalScrollbar   (nsBoxLayoutState& aState, nsRect& aScrollAreaSize, PRBool aOnRight);
  void RemoveHorizontalScrollbar(nsBoxLayoutState& aState, nsRect& aScrollAreaSize, PRBool aOnBottom);
  void RemoveVerticalScrollbar  (nsBoxLayoutState& aState, nsRect& aScrollAreaSize, PRBool aOnRight);

  static void AdjustReflowStateForPrintPreview(nsBoxLayoutState& aState, PRBool& aSetBack);
  static void AdjustReflowStateBack(nsBoxLayoutState& aState, PRBool aSetBack);

  // nsIScrollableFrame
  virtual nsIFrame* GetScrolledFrame() const;
  virtual nsIScrollableView* GetScrollableView();

  virtual nsPoint GetScrollPosition() const;
  virtual void ScrollTo(nsPoint aScrollPosition, PRUint32 aFlags);

  virtual void SetScrollbarVisibility(PRBool aVerticalVisible, PRBool aHorizontalVisible);

  virtual nsIBox* GetScrollbarBox(PRBool aVertical);

  virtual void CurPosAttributeChanged(nsIContent* aChild, PRInt32 aModType);

  // nsIStatefulFrame
  NS_IMETHOD SaveState(nsPresContext* aPresContext, nsPresState** aState) {
    NS_ENSURE_ARG_POINTER(aState);
    *aState = mInner.SaveState();
    return NS_OK;
  }
  NS_IMETHOD RestoreState(nsPresContext* aPresContext, nsPresState* aState) {
    NS_ENSURE_ARG_POINTER(aState);
    mInner.RestoreState(aState);
    return NS_OK;
  }

  virtual void ScrollToRestoredPosition() {
    mInner.ScrollToRestoredPosition();
  }

  virtual nsMargin GetActualScrollbarSizes() const {
    return mInner.GetActualScrollbarSizes();
  }
  virtual nsMargin GetDesiredScrollbarSizes(nsBoxLayoutState* aState);
  virtual nsGfxScrollFrameInner::ScrollbarStyles GetScrollbarStyles() const;

  /**
   * Get the "type" of the frame
   *
   * @see nsLayoutAtoms::scrollFrame
   */
  virtual nsIAtom* GetType() const;
  
#ifdef NS_DEBUG
  NS_IMETHOD GetFrameName(nsAString& aResult) const;
#endif

  virtual nsresult GetContentOf(nsIContent** aContent);

protected:
  nsXULScrollFrame(nsIPresShell* aShell, PRBool aIsRoot);
  virtual PRIntn GetSkipSides() const;

private:
  friend class nsGfxScrollFrameInner;
  nsGfxScrollFrameInner mInner;
  nscoord mMaxElementWidth;
  PRPackedBool mHorizontalOverflow;
  PRPackedBool mVerticalOverflow;
};

#endif /* nsGfxScrollFrame_h___ */
