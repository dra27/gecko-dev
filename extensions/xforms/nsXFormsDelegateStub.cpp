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
 * Novell, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Allan Beaufour <abeaufour@novell.com>
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


#include "nsIDOMDocument.h"
#include "nsIDOM3Node.h"
#include "nsIDOMEventTarget.h"
#include "nsIDOMEvent.h"
#include "nsIDOMUIEvent.h"
#include "nsIDOMDocumentView.h"
#include "nsIDOMAbstractView.h"
#include "nsIDOMDocumentEvent.h"
#include "nsDOMString.h"
#include "nsIModelElementPrivate.h"
#include "nsIXFormsUIWidget.h"
#include "nsXFormsAtoms.h"
#include "nsXFormsDelegateStub.h"
#include "nsXFormsUtils.h"
#include "nsIServiceManager.h"
#include "nsXFormsModelElement.h"

NS_IMPL_ISUPPORTS_INHERITED1(nsXFormsDelegateStub,
                             nsXFormsBindableControlStub,
                             nsIXFormsDelegate)


NS_IMETHODIMP
nsXFormsDelegateStub::WillChangeDocument(nsIDOMDocument *aNewDocument)
{
  mRepeatState = eType_Unknown;
  return nsXFormsBindableControlStub::WillChangeDocument(aNewDocument);
}

NS_IMETHODIMP
nsXFormsDelegateStub::WillChangeParent(nsIDOMElement *aNewParent)
{
  mRepeatState = eType_Unknown;
  return nsXFormsBindableControlStub::WillChangeParent(aNewParent);
}

NS_IMETHODIMP
nsXFormsDelegateStub::OnCreated(nsIXTFBindableElementWrapper *aWrapper)
{
  nsresult rv = nsXFormsBindableControlStub::OnCreated(aWrapper);
  NS_ENSURE_SUCCESS(rv, rv);
  aWrapper->SetNotificationMask(kStandardNotificationMask |
                                nsIXTFElement::NOTIFY_WILL_CHANGE_DOCUMENT |
                                nsIXTFElement::NOTIFY_WILL_CHANGE_PARENT);
  return rv;
}

NS_IMETHODIMP
nsXFormsDelegateStub::OnDestroyed()
{
  nsXFormsModelElement::CancelPostRefresh(this);
  return nsXFormsBindableControlStub::OnDestroyed();
}

// nsIXFormsControl

NS_IMETHODIMP
nsXFormsDelegateStub::Refresh()
{
  if (mRepeatState == eType_Template)
    return NS_OK;

  const nsVoidArray* list = nsPostRefresh::PostRefreshList();
  if (list && list->IndexOf(this) >= 0) {
    // This control will be refreshed later.
    return NS_OK;
  }

  SetMozTypeAttribute();

  nsCOMPtr<nsIXFormsUIWidget> widget = do_QueryInterface(mElement);
  if (!widget)
    return NS_OK;

  return widget->Refresh();
}

NS_IMETHODIMP
nsXFormsDelegateStub::TryFocus(PRBool* aOK)
{
  *aOK = PR_FALSE;
  if (GetRelevantState()) {
    nsCOMPtr<nsIXFormsUIWidget> widget = do_QueryInterface(mElement);
    if (widget) {
      widget->Focus(aOK);
    }
  }

  return NS_OK;
}

// nsIXFormsDelegate

NS_IMETHODIMP
nsXFormsDelegateStub::GetValue(nsAString& aValue)
{
  SetDOMStringToNull(aValue);
  if (mBoundNode) {
    nsXFormsUtils::GetNodeValue(mBoundNode, aValue);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsXFormsDelegateStub::SetValue(const nsAString& aValue)
{
  if (!mBoundNode || !mModel)
    return NS_OK;

  PRBool changed;
  nsresult rv = mModel->SetNodeValue(mBoundNode, aValue, &changed);
  NS_ENSURE_SUCCESS(rv, rv);
  if (changed) {
    nsCOMPtr<nsIDOMNode> model = do_QueryInterface(mModel);
 
    if (model) {
      rv = nsXFormsUtils::DispatchEvent(model, eEvent_Recalculate);
      NS_ENSURE_SUCCESS(rv, rv);
      rv = nsXFormsUtils::DispatchEvent(model, eEvent_Revalidate);
      NS_ENSURE_SUCCESS(rv, rv);
      rv = nsXFormsUtils::DispatchEvent(model, eEvent_Refresh);
      NS_ENSURE_SUCCESS(rv, rv);
    }
  }

  return NS_OK;
}

nsresult
nsXFormsDelegateStub::GetState(const nsAString &aState, PRBool *aStateVal)
{
  NS_ENSURE_ARG_POINTER(aStateVal);

  if (mElement) {
    mElement->HasAttribute(aState, aStateVal);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsXFormsDelegateStub::GetIsReadonly(PRBool *aStateVal)
{
  return GetState(NS_LITERAL_STRING("read-only"), aStateVal);
}

NS_IMETHODIMP
nsXFormsDelegateStub::GetIsEnabled(PRBool *aStateVal)
{
  return GetState(NS_LITERAL_STRING("enabled"), aStateVal);
}

NS_IMETHODIMP
nsXFormsDelegateStub::GetIsRequired(PRBool *aStateVal)
{
  return GetState(NS_LITERAL_STRING("required"), aStateVal);
}

NS_IMETHODIMP
nsXFormsDelegateStub::GetIsValid(PRBool *aStateVal)
{
  return GetState(NS_LITERAL_STRING("valid"), aStateVal);
}

NS_IMETHODIMP
nsXFormsDelegateStub::GetHasBoundNode(PRBool *aHasBoundNode)
{
  *aHasBoundNode = mBoundNode ? PR_TRUE : PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP
nsXFormsDelegateStub::WidgetAttached()
{
  if (UpdateRepeatState() != eType_Template)
    nsXFormsModelElement::NeedsPostRefresh(this);

  return NS_OK;
}

nsRepeatState
nsXFormsDelegateStub::UpdateRepeatState()
{
  mRepeatState = eType_NotApplicable;
  nsCOMPtr<nsIDOMNode> parent;
  mElement->GetParentNode(getter_AddRefs(parent));
  while (parent) {
    if (nsXFormsUtils::IsXFormsElement(parent, NS_LITERAL_STRING("contextcontainer"))) {
      mRepeatState = eType_GeneratedContent;
      break;
    }
    if (nsXFormsUtils::IsXFormsElement(parent, NS_LITERAL_STRING("repeat"))) {
      mRepeatState = eType_Template;
      break;
    }
    nsCOMPtr<nsIDOMNode> tmp;
    parent->GetParentNode(getter_AddRefs(tmp));
    parent = tmp;
  }
  return mRepeatState;
}

void
nsXFormsDelegateStub::SetMozTypeAttribute()
{
  NS_NAMED_LITERAL_STRING(mozTypeNs, NS_NAMESPACE_MOZ_XFORMS_TYPE);
  NS_NAMED_LITERAL_STRING(mozType, "type");

  if (mModel && mBoundNode) {
    nsAutoString type, ns;
    if (NS_FAILED(mModel->GetTypeAndNSFromNode(mBoundNode, type, ns))) {
      mElement->RemoveAttributeNS(mozTypeNs, mozType);
      return;
    }

    ns.AppendLiteral("#");
    ns.Append(type);
    mElement->SetAttributeNS(mozTypeNs, mozType, ns);
  } else {
    mElement->RemoveAttributeNS(mozTypeNs, mozType);
  }
}
