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

#include "nsIXFormsSelectChild.h"
#include "nsXFormsStubElement.h"
#include "nsIDOMHTMLOptGroupElement.h"
#include "nsString.h"
#include "nsCOMPtr.h"
#include "nsIXTFBindableElementWrapper.h"
#include "nsIDOMDocument.h"
#include "nsIDOMNodeList.h"
#include "nsIDocument.h"
#include "nsXFormsUtils.h"
#include "nsXFormsModelElement.h"
#include "nsArray.h"
#include "nsIXFormsControlBase.h"
#include "nsIXFormsControl.h"
#include "nsIXFormsItemSetUIElement.h"

class nsXFormsItemSetElement : public nsXFormsBindableStub,
                               public nsIXFormsSelectChild,
                               public nsIXFormsControlBase
{
public:
  nsXFormsItemSetElement() : mElement(nsnull) {}

  NS_DECL_ISUPPORTS_INHERITED

  NS_IMETHOD OnCreated(nsIXTFBindableElementWrapper *aWrapper);

  // nsIXTFElement overrides
  NS_IMETHOD ParentChanged(nsIDOMElement *aNewParent);
  NS_IMETHOD ChildInserted(nsIDOMNode *aChild, PRUint32 aIndex);
  NS_IMETHOD ChildAppended(nsIDOMNode *aChild);
  NS_IMETHOD WillRemoveChild(PRUint32 aIndex);
  NS_IMETHOD BeginAddingChildren();
  NS_IMETHOD DoneAddingChildren();

  // nsIXFormsControlBase overrides
  NS_IMETHOD Bind();
  NS_IMETHOD Refresh();

  // nsIXFormsSelectChild
  NS_DECL_NSIXFORMSSELECTCHILD

private:
  nsIDOMElement *mElement;
  nsCOMArray<nsIXFormsSelectChild> mItems;
};

NS_IMPL_ISUPPORTS_INHERITED2(nsXFormsItemSetElement,
                             nsXFormsBindableStub,
                             nsIXFormsSelectChild,
                             nsIXFormsControlBase)

NS_IMETHODIMP
nsXFormsItemSetElement::OnCreated(nsIXTFBindableElementWrapper *aWrapper)
{
  nsresult rv = nsXFormsBindableStub::OnCreated(aWrapper);
  NS_ENSURE_SUCCESS(rv, rv);

  aWrapper->SetNotificationMask(nsIXTFElement::NOTIFY_PARENT_CHANGED |
                                nsIXTFElement::NOTIFY_CHILD_INSERTED |
                                nsIXTFElement::NOTIFY_CHILD_APPENDED |
                                nsIXTFElement::NOTIFY_WILL_REMOVE_CHILD |
                                nsIXTFElement::NOTIFY_BEGIN_ADDING_CHILDREN);

  nsCOMPtr<nsIDOMElement> node;
  aWrapper->GetElementNode(getter_AddRefs(node));

  // It's ok to keep a pointer to mElement.  mElement will have an
  // owning reference to this object, so as long as we null out mElement in
  // OnDestroyed, it will always be valid.

  mElement = node;
  NS_ASSERTION(mElement, "Wrapper is not an nsIDOMElement, we'll crash soon");

  return NS_OK;
}

NS_IMETHODIMP
nsXFormsItemSetElement::ParentChanged(nsIDOMElement *aNewParent)
{
  if (aNewParent)
    Refresh();

  return NS_OK;
}

NS_IMETHODIMP
nsXFormsItemSetElement::ChildInserted(nsIDOMNode *aChild, PRUint32 aIndex)
{
  Refresh();
  return NS_OK;
}

NS_IMETHODIMP
nsXFormsItemSetElement::ChildAppended(nsIDOMNode *aChild)
{
  Refresh();
  return NS_OK;
}

NS_IMETHODIMP
nsXFormsItemSetElement::WillRemoveChild(PRUint32 aIndex)
{
  Refresh();
  return NS_OK;
}

NS_IMETHODIMP
nsXFormsItemSetElement::BeginAddingChildren()
{
  // Suppress child notifications until we're done getting children.
  nsCOMPtr<nsIXTFElementWrapper> wrapper = do_QueryInterface(mElement);
  NS_ASSERTION(wrapper, "huh? our element must be an xtf wrapper");

  wrapper->SetNotificationMask(nsIXTFElement::NOTIFY_PARENT_CHANGED |
                               nsIXTFElement::NOTIFY_DONE_ADDING_CHILDREN);

  return NS_OK;
}

NS_IMETHODIMP
nsXFormsItemSetElement::DoneAddingChildren()
{
  // Unsuppress child notifications until we're done getting children.
  nsCOMPtr<nsIXTFElementWrapper> wrapper = do_QueryInterface(mElement);
  NS_ASSERTION(wrapper, "huh? our element must be an xtf wrapper");

  wrapper->SetNotificationMask(nsIXTFElement::NOTIFY_PARENT_CHANGED |
                               nsIXTFElement::NOTIFY_CHILD_INSERTED |
                               nsIXTFElement::NOTIFY_CHILD_APPENDED |
                               nsIXTFElement::NOTIFY_WILL_REMOVE_CHILD);

  // Walk our children and get their anonymous content.
  Refresh();
  return NS_OK;
}

// nsIXFormsSelectChild

NS_IMETHODIMP
nsXFormsItemSetElement::GetAnonymousNodes(nsIArray **aNodes)
{
  nsCOMPtr<nsIMutableArray> array;
  nsresult rv = NS_NewArray(getter_AddRefs(array));
  NS_ENSURE_SUCCESS(rv, rv);

  PRInt32 itemCount = mItems.Count();
  nsCOMPtr<nsIArray> childNodes;
  nsCOMPtr<nsIDOMNode> childOption;

  for (PRInt32 i = 0; i < itemCount; ++i) {
    mItems[i]->GetAnonymousNodes(getter_AddRefs(childNodes));
    if (!childNodes)
      continue;

    PRUint32 anonLength = 0;
    childNodes->GetLength(&anonLength);

    for (PRUint32 j = 0; j < anonLength; ++j) {
      childOption = do_QueryElementAt(childNodes, j);
      NS_ASSERTION(childOption, "invalid nodelist");

      array->AppendElement(childOption, PR_FALSE);
    }
  }

  NS_ADDREF(*aNodes = array);
  return NS_OK;
}

NS_IMETHODIMP
nsXFormsItemSetElement::SelectItemByValue(const nsAString &aValue, nsIDOMNode **aSelected)
{
  NS_ENSURE_ARG_POINTER(aSelected);
  NS_ENSURE_STATE(mElement);
  *aSelected = nsnull;
  // nsIXFormsItemSetUIElement is implemented by the XBL binding.
  nsCOMPtr<nsIXFormsItemSetUIElement> uiItemSet(do_QueryInterface(mElement));
  NS_ENSURE_STATE(uiItemSet);

  nsCOMPtr<nsIDOMElement> anonContent;
  uiItemSet->GetAnonymousItemSetContent(getter_AddRefs(anonContent));
  NS_ENSURE_STATE(anonContent);

  nsCOMPtr<nsIDOMNode> child, tmp;
  anonContent->GetFirstChild(getter_AddRefs(child));
  // Trying to select the first possible (generated) \<item\> element.
  while (child) {
    nsCOMPtr<nsIXFormsSelectChild> selectChild(do_QueryInterface(tmp));
    if (selectChild) {
      selectChild->SelectItemByValue(aValue, aSelected);
      if (*aSelected) {
        return NS_OK;
      }
    }
    tmp.swap(child);
    tmp->GetNextSibling(getter_AddRefs(child));
  }
  return NS_OK;
}

NS_IMETHODIMP
nsXFormsItemSetElement::SelectItemsByValue(const nsStringArray &aValueList)
{
  return NS_OK;
}

NS_IMETHODIMP
nsXFormsItemSetElement::SelectItemsByContent(nsIDOMNode *aNode)
{
  return NS_OK;
}

NS_IMETHODIMP
nsXFormsItemSetElement::WriteSelectedItems(nsIDOMNode *aContainer, 
                                           nsAString  &aStringBuffer)
{
  return NS_OK;
}

NS_IMETHODIMP
nsXFormsItemSetElement::SetContext(nsIDOMElement *aContextNode,
                                   PRInt32 aContextPosition,
                                   PRInt32 aContextSize)
{
  // itemset can't be inside another itemset, so we don't need to worry about
  // this.
  return NS_OK;
}

// internal methods

NS_IMETHODIMP
nsXFormsItemSetElement::Bind()
{
  return NS_OK;
}

NS_IMETHODIMP
nsXFormsItemSetElement::Refresh()
{
  mItems.Clear();
  // We need to create item elements for each element referenced by the
  // nodeset.  Each of these items will create an anonymous HTML option element
  // which will return from GetAnonymousNodes.  We then clone our template
  // content and insert the cloned content as children of the HTML option.

  nsCOMPtr<nsIDOMDocument> domDoc;
  mElement->GetOwnerDocument(getter_AddRefs(domDoc));

  if (!nsXFormsUtils::IsDocumentReadyForBind(domDoc)) {
    // not ready to bind yet, defer
    nsXFormsModelElement::DeferElementBind(domDoc, this);
    return NS_OK;
  }

  nsCOMPtr<nsIModelElementPrivate> model;
  nsCOMPtr<nsIDOMXPathResult> result;
  nsXFormsUtils::EvaluateNodeBinding(mElement,
                                     nsXFormsUtils::ELEMENT_WITH_MODEL_ATTR,
                                     NS_LITERAL_STRING("nodeset"),
                                     EmptyString(),
                                     nsIDOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
                                     getter_AddRefs(model),
                                     getter_AddRefs(result));

  if (!result)
    return NS_OK;

  nsCOMPtr<nsIDOMNode> node, templateNode, cloneNode, tmpNode;
  nsCOMPtr<nsIDOMElement> itemNode, itemWrapperNode, contextContainer;
  nsCOMPtr<nsIDOMNodeList> templateNodes;
  mElement->GetChildNodes(getter_AddRefs(templateNodes));
  PRUint32 templateNodeCount = 0;
  if (templateNodes)
    templateNodes->GetLength(&templateNodeCount);
  
  nsCOMPtr<nsIDOMDocument> document;
  mElement->GetOwnerDocument(getter_AddRefs(document));
  if (!document)
    return NS_OK;

  PRUint32 nodeCount;
  result->GetSnapshotLength(&nodeCount);

  // XXX Possibly cleanup this when XBLizing <select>.
  nsCOMPtr<nsIDOMNode> parent, tmp;
  mElement->GetParentNode(getter_AddRefs(parent));
  PRBool parentIsSelect1 = PR_FALSE;
  while (parent &&
    !nsXFormsUtils::IsXFormsElement(parent, NS_LITERAL_STRING("select"))) {
    if (nsXFormsUtils::IsXFormsElement(parent, NS_LITERAL_STRING("select1"))) {
      parentIsSelect1 = PR_TRUE;
      break;
    }
    tmp.swap(parent);
    tmp->GetParentNode(getter_AddRefs(parent));
  }
  
  if (parentIsSelect1) {
    // For select1
    nsCOMPtr<nsIXFormsItemSetUIElement> uiItemSet(do_QueryInterface(mElement));
    nsCOMPtr<nsIDOMElement> anonContent;
    if (uiItemSet) {
      uiItemSet->GetAnonymousItemSetContent(getter_AddRefs(anonContent));
    }

    NS_ENSURE_STATE(anonContent);

    nsCOMPtr<nsIDOMNode> childNode, nodeReturn;
    while (NS_SUCCEEDED(anonContent->GetFirstChild(getter_AddRefs(childNode))) &&
         childNode) {
      anonContent->RemoveChild(childNode, getter_AddRefs(nodeReturn));
    }

    for (PRUint32 i = 0; i < nodeCount; ++i) {
      result->SnapshotItem(i, getter_AddRefs(node));
      NS_ASSERTION(node, "incorrect snapshot length");

      nsresult rv = document->CreateElementNS(NS_LITERAL_STRING(NS_NAMESPACE_XFORMS),
                                              NS_LITERAL_STRING("item"),
                                              getter_AddRefs(itemNode));
      NS_ENSURE_SUCCESS(rv, rv);

      // XXX Could we get rid of the <contextcontainer>?
      rv = document->CreateElementNS(NS_LITERAL_STRING(NS_NAMESPACE_XFORMS),
                                     NS_LITERAL_STRING("contextcontainer"),
                                     getter_AddRefs(contextContainer));

      NS_ENSURE_SUCCESS(rv, rv);

      nsCOMPtr<nsIDOMElement> modelElement = do_QueryInterface(model);
      nsAutoString modelID;
      modelElement->GetAttribute(NS_LITERAL_STRING("id"), modelID);

      contextContainer->SetAttribute(NS_LITERAL_STRING("model"), modelID);

      nsCOMPtr<nsIXFormsContextControl> ctx(do_QueryInterface(contextContainer));
      if (ctx) {
        ctx->SetContext(nsCOMPtr<nsIDOMElement>(do_QueryInterface(node)),
                        i + 1, nodeCount);
      }
      // Clone the template content under the item
      for (PRUint32 j = 0; j < templateNodeCount; ++j) {
        templateNodes->Item(j, getter_AddRefs(templateNode));
        templateNode->CloneNode(PR_TRUE, getter_AddRefs(cloneNode));
        contextContainer->AppendChild(cloneNode, getter_AddRefs(templateNode));
      }

      itemNode->AppendChild(contextContainer, getter_AddRefs(tmpNode));
      anonContent->AppendChild(itemNode, getter_AddRefs(tmpNode));
    }
  } else {
    for (PRUint32 i = 0; i < nodeCount; ++i) {
      result->SnapshotItem(i, getter_AddRefs(node));
      NS_ASSERTION(node, "incorrect snapshot length");

      document->CreateElementNS(NS_LITERAL_STRING(NS_NAMESPACE_XFORMS),
                                NS_LITERAL_STRING("item"),
                                getter_AddRefs(itemNode));

      if (!itemNode)
        return NS_OK;

      nsCOMPtr<nsIDOMElement> modelElement = do_QueryInterface(model);
      nsAutoString modelID;
      modelElement->GetAttribute(NS_LITERAL_STRING("id"), modelID);

      itemNode->SetAttribute(NS_LITERAL_STRING("model"), modelID);

      nsCOMPtr<nsIXFormsSelectChild> item = do_QueryInterface(itemNode);
      NS_ASSERTION(item, "item must be a SelectChild!");

      item->SetContext(nsCOMPtr<nsIDOMElement>(do_QueryInterface(node)),
                       i + 1, nodeCount);

      // Clone the template content under the item
      for (PRUint32 j = 0; j < templateNodeCount; ++j) {
        templateNodes->Item(j, getter_AddRefs(templateNode));
        templateNode->CloneNode(PR_TRUE, getter_AddRefs(cloneNode));
        itemNode->AppendChild(cloneNode, getter_AddRefs(templateNode));
      }

      mItems.AppendObject(item);
    }
  }

  // refresh parent control, since we are being defered
  nsCOMPtr<nsIDOMNode> parentNode;
  mElement->GetParentNode(getter_AddRefs(parentNode));
  if (parentNode) {

    nsCOMPtr<nsIXFormsControlBase> stub = do_QueryInterface(parentNode);
    if (stub) {
      stub->Refresh();
    }
  }

  return NS_OK;
}

NS_HIDDEN_(nsresult)
NS_NewXFormsItemSetElement(nsIXTFElement **aResult)
{
  *aResult = new nsXFormsItemSetElement();
  if (!*aResult)
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ADDREF(*aResult);
  return NS_OK;
}

