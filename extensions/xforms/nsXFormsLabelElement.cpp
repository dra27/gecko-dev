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
 * The Original Code is Mozilla XForms support.
 *
 * The Initial Developer of the Original Code is
 * IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Brian Ryner <bryner@brianryner.com>
 *  Olli Pettay <Olli.Pettay@helsinki.fi>
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

/**
 * Implementation of the XForms \<label\> element.
 */

#include "nsXFormsUtils.h"
#include "nsXFormsControlStub.h"
#include "nsXFormsDelegateStub.h"
#include "nsXFormsAtoms.h"
#include "nsCOMPtr.h"
#include "nsIDOMElement.h"
#include "nsIDOMXPathResult.h"
#include "nsIDOM3Node.h"
#include "nsIDOMDocument.h"
#include "nsIDOMText.h"
#include "nsIXTFXMLVisualWrapper.h"
#include "nsString.h"
#include "nsIXFormsUIWidget.h"
#include "nsIDocument.h"
#include "nsNetUtil.h"
#include "nsIXFormsLabelElement.h"
#include "nsIXFormsItemElement.h"

class nsXFormsLabelElement : public nsXFormsDelegateStub,
                             public nsIStreamListener,
                             public nsIInterfaceRequestor,
                             public nsIXFormsLabelElement
{
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_NSIREQUESTOBSERVER
  NS_DECL_NSISTREAMLISTENER
  NS_DECL_NSIINTERFACEREQUESTOR
  NS_DECL_NSIXFORMSLABELELEMENT

  // nsIXFormsDelegate
  NS_IMETHOD GetValue(nsAString& aValue);

  // nsIXFormsControl
  NS_IMETHOD IsEventTarget(PRBool *aOK);
  NS_IMETHOD Refresh();

  NS_IMETHOD OnCreated(nsIXTFBindableElementWrapper *aWrapper);

  // nsIXTFElement overrides
  NS_IMETHOD ChildInserted(nsIDOMNode *aChild, PRUint32 aIndex);
  NS_IMETHOD ChildAppended(nsIDOMNode *aChild);
  NS_IMETHOD ChildRemoved(PRUint32 aIndex);
  NS_IMETHOD AttributeSet(nsIAtom *aName, const nsAString &aSrc);
  NS_IMETHOD AttributeRemoved(nsIAtom *aName);

#ifdef DEBUG_smaug
  virtual const char* Name() { return "label"; }
#endif
private:
  NS_HIDDEN_(void) LoadExternalLabel(const nsAString& aValue);

  nsCString            mSrcAttrText;
  nsCOMPtr<nsIChannel> mChannel;
};

NS_IMPL_ISUPPORTS_INHERITED4(nsXFormsLabelElement,
                             nsXFormsDelegateStub,
                             nsIRequestObserver,
                             nsIStreamListener,
                             nsIInterfaceRequestor,
                             nsIXFormsLabelElement)

NS_IMETHODIMP
nsXFormsLabelElement::OnCreated(nsIXTFBindableElementWrapper *aWrapper)
{
  nsresult rv = nsXFormsDelegateStub::OnCreated(aWrapper);
  NS_ENSURE_SUCCESS(rv, rv);

  aWrapper->SetNotificationMask(kStandardNotificationMask |
                                nsIXTFElement::NOTIFY_CHILD_INSERTED |
                                nsIXTFElement::NOTIFY_CHILD_APPENDED |
                                nsIXTFElement::NOTIFY_CHILD_REMOVED);
  return NS_OK;
}

NS_IMETHODIMP
nsXFormsLabelElement::ChildInserted(nsIDOMNode *aChild, PRUint32 aIndex)
{
  Refresh();
  return NS_OK;
}

NS_IMETHODIMP
nsXFormsLabelElement::ChildAppended(nsIDOMNode *aChild)
{
  Refresh();
  return NS_OK;
}

NS_IMETHODIMP
nsXFormsLabelElement::ChildRemoved(PRUint32 aIndex)
{
  Refresh();
  return NS_OK;
}

NS_IMETHODIMP
nsXFormsLabelElement::AttributeSet(nsIAtom *aName, const nsAString &aValue)
{
  if (aName == nsXFormsAtoms::src) {
    // If we are currently trying to load an external label, cancel the request.
    if (mChannel) {
      mChannel->Cancel(NS_BINDING_ABORTED);
    }

    LoadExternalLabel(aValue);

    // No need to call Refresh() here, since it is called once the link
    // target has been read in, during OnStopRequest()

    return NS_OK;
  }

  return nsXFormsDelegateStub::AttributeSet(aName, aValue);
}

NS_IMETHODIMP
nsXFormsLabelElement::AttributeRemoved(nsIAtom *aName)
{
  if (aName == nsXFormsAtoms::src) {
    // If we are currently trying to load an external label, cancel the request.
    if (mChannel) {
      mChannel->Cancel(NS_BINDING_ABORTED);
    }

    mSrcAttrText.Truncate();
    Refresh();
    return NS_OK;
  }

  return nsXFormsDelegateStub::AttributeRemoved(aName);
}

NS_IMETHODIMP
nsXFormsLabelElement::GetValue(nsAString& aValue)
{
  // The order of precedence for determining the label is:
  //   single node binding, linking, inline text (8.3.3)

  nsXFormsDelegateStub::GetValue(aValue);
  if (aValue.IsVoid() && !mSrcAttrText.IsEmpty()) {
    // handle linking ('src') attribute
    aValue = NS_ConvertUTF8toUTF16(mSrcAttrText);
  }

  return NS_OK;
}

void
nsXFormsLabelElement::LoadExternalLabel(const nsAString& aSrc)
{
  nsresult rv = NS_ERROR_FAILURE;

  nsCOMPtr<nsIDOMDocument> domDoc;
  mElement->GetOwnerDocument(getter_AddRefs(domDoc));
  nsCOMPtr<nsIDocument> doc(do_QueryInterface(domDoc));
  if (doc) {
    nsCOMPtr<nsIURI> uri;
    NS_NewURI(getter_AddRefs(uri), aSrc, doc->GetDocumentCharacterSet().get(),
              doc->GetDocumentURI());
    if (uri) {
      if (nsXFormsUtils::CheckSameOrigin(doc->GetDocumentURI(), uri)) {
        nsCOMPtr<nsILoadGroup> loadGroup;
        loadGroup = doc->GetDocumentLoadGroup();
        NS_WARN_IF_FALSE(loadGroup, "No load group!");

        // Using the same load group as the main document and creating
        // the channel with LOAD_NORMAL flag delays the dispatching of
        // the 'load' event until label data document has been loaded.
        NS_NewChannel(getter_AddRefs(mChannel), uri, nsnull, loadGroup,
                      this, nsIRequest::LOAD_NORMAL);

        if (mChannel) {
          rv = mChannel->AsyncOpen(this, nsnull);
          if (NS_FAILED(rv)) {
            // URI doesn't exist; report error.
            mChannel = nsnull;

            // XXX Passing |mElement| as |aContext| param to ReportError leads
            //     to an infinite loop.  Avoid for now.
            const nsPromiseFlatString& flat = PromiseFlatString(aSrc);
            const PRUnichar *strings[] = { flat.get() };
            nsXFormsUtils::ReportError(NS_LITERAL_STRING("labelLink1Error"),
                                       strings, 1, mElement, nsnull);

            nsCOMPtr<nsIModelElementPrivate> modelPriv =
                                              nsXFormsUtils::GetModel(mElement);
            nsCOMPtr<nsIDOMNode> model = do_QueryInterface(modelPriv);
            nsXFormsUtils::DispatchEvent(model, eEvent_LinkError);
          }
        }
      } else {
        nsXFormsUtils::ReportError(NS_LITERAL_STRING("labelLinkLoadOrigin"),
                                   domDoc);
      }
    }
  }
}

// nsIXFormsControl

NS_IMETHODIMP
nsXFormsLabelElement::Refresh()
{
  nsXFormsDelegateStub::Refresh();
  nsCOMPtr<nsIDOMNode> parent;
  mElement->GetParentNode(getter_AddRefs(parent));

  // If <label> is inside <select1> its parent is <item>
  // or <contextcontainer> (which parent is <item>).
  nsCOMPtr<nsIXFormsItemElement> item(do_QueryInterface(parent));
  if (item) {
    item->LabelRefreshed();
  } else if (parent) {
    nsCOMPtr<nsIDOMNode> grandparent;
    parent->GetParentNode(getter_AddRefs(grandparent));
    item = do_QueryInterface(grandparent);
    if (item) {
      item->LabelRefreshed();
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsXFormsLabelElement::IsEventTarget(PRBool *aOK)
{
  *aOK = PR_FALSE;
  return NS_OK;
}

// nsIInterfaceRequestor

NS_IMETHODIMP
nsXFormsLabelElement::GetInterface(const nsIID &aIID, void **aResult)
{
  *aResult = nsnull;
  return QueryInterface(aIID, aResult);
}

// nsIStreamListener

NS_IMETHODIMP
nsXFormsLabelElement::OnStartRequest(nsIRequest *aRequest,
                                     nsISupports *aContext)
{
  // Only handle data from text files for now.  Cancel any other requests.
  nsCOMPtr<nsIChannel> channel(do_QueryInterface(aRequest));
  if (channel) {
    nsCAutoString type;
    channel->GetContentType(type);
    if (!type.EqualsLiteral("text/plain"))
      return NS_ERROR_ILLEGAL_VALUE;
  }

  mSrcAttrText.Truncate();
  return NS_OK;
}

NS_IMETHODIMP
nsXFormsLabelElement::OnDataAvailable(nsIRequest *aRequest,
                                      nsISupports *aContext,
                                      nsIInputStream *aInputStream,
                                      PRUint32 aOffset,
                                      PRUint32 aCount)
{
  nsresult rv;
  PRUint32 size, bytesRead;
  char buffer[256];

  while (aCount) {
    size = PR_MIN(aCount, sizeof(buffer));
    rv = aInputStream->Read(buffer, size, &bytesRead);
    if (NS_FAILED(rv))
      return rv;
    mSrcAttrText.Append(buffer, bytesRead);
    aCount -= bytesRead;
  }

  return NS_OK;
}

NS_IMETHODIMP
nsXFormsLabelElement::OnStopRequest(nsIRequest *aRequest,
                                    nsISupports *aContext,
                                    nsresult aStatusCode)
{
  // Done with load request, so null out channel member
  mChannel = nsnull;

  if (NS_FAILED(aStatusCode)) {
    // If we received NS_BINDING_ABORTED, then we were cancelled by a later
    // AttributeSet() call.  Don't do anything and return.
    if (aStatusCode == NS_BINDING_ABORTED)
      return NS_OK;

    // XXX Passing |mElement| as |aContext| param to ReportError leads
    //     to an infinite loop.  Avoid for now.
    nsAutoString src;
    mElement->GetAttribute(NS_LITERAL_STRING("src"), src);
    const PRUnichar *strings[] = { src.get() };
    nsXFormsUtils::ReportError(NS_LITERAL_STRING("labelLink2Error"),
                               strings, 1, mElement, nsnull);

    nsCOMPtr<nsIModelElementPrivate> modelPriv =
      nsXFormsUtils::GetModel(mElement);
    nsCOMPtr<nsIDOMNode> model = do_QueryInterface(modelPriv);
    nsXFormsUtils::DispatchEvent(model, eEvent_LinkError);

    mSrcAttrText.Truncate();
  }

  nsCOMPtr<nsIXFormsUIWidget> widget = do_QueryInterface(mElement);
  if (widget)
    widget->Refresh();

  return NS_OK;
}

NS_IMETHODIMP
nsXFormsLabelElement::GetTextValue(nsAString& aValue)
{
  NS_ENSURE_STATE(mElement);
  GetValue(aValue);

  if (aValue.IsVoid()) {
    nsCOMPtr<nsIDOM3Node> inner(do_QueryInterface(mElement));
    if (inner) {
      inner->GetTextContent(aValue);
    }
  }

  return NS_OK;
}

NS_HIDDEN_(nsresult)
NS_NewXFormsLabelElement(nsIXTFElement **aResult)
{
  *aResult = new nsXFormsLabelElement();
  if (!*aResult)
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ADDREF(*aResult);
  return NS_OK;
}
