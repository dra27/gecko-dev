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
 * Olli Pettay.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Olli Pettay <Olli.Pettay@helsinki.fi> (original author)
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

#include "nsXFormsActionModuleBase.h"
#include "nsXFormsActionElement.h"
#include "nsIDOM3Node.h"
#include "nsIDOMXPathResult.h"
#include "nsIDOMDocument.h"
#include "nsIDOMEvent.h"
#include "nsIDOMElement.h"
#include "nsIXFormsModelElement.h"
#include "nsXFormsModelElement.h"
#include "nsXFormsUtils.h"

class nsXFormsSetValueElement : public nsXFormsActionModuleBase
{
public:
  nsXFormsSetValueElement();
  NS_DECL_NSIXFORMSACTIONMODULEELEMENT
};

nsXFormsSetValueElement::nsXFormsSetValueElement()
{
}

NS_IMETHODIMP
nsXFormsSetValueElement::HandleAction(nsIDOMEvent* aEvent,
                                      nsIXFormsActionElement *aParentAction)
{
  if (!mElement)
    return NS_OK;
  
  nsCOMPtr<nsIModelElementPrivate> modelPriv;
  nsCOMPtr<nsIDOMXPathResult> result;
  nsresult rv =
    nsXFormsUtils:: EvaluateNodeBinding(mElement,
                                        nsXFormsUtils::ELEMENT_WITH_MODEL_ATTR,
                                        NS_LITERAL_STRING("ref"),
                                        EmptyString(),
                                        nsIDOMXPathResult::FIRST_ORDERED_NODE_TYPE,
                                        getter_AddRefs(modelPriv),
                                        getter_AddRefs(result));
  NS_ENSURE_SUCCESS(rv, rv);

  if (!result | !modelPriv)
    return NS_OK;

  nsCOMPtr<nsIDOMNode> singleNode;
  result->GetSingleNodeValue(getter_AddRefs(singleNode));
  if (!singleNode)
    return NS_OK;

  nsAutoString value;
  nsAutoString valueAttr;
  mElement->GetAttribute(NS_LITERAL_STRING("value"), valueAttr);

  if(!valueAttr.IsEmpty()) {
    // According to the XForms Errata, the context node for the XPath expression
    //   stored in @value should be the node that setvalue is bound to.
    nsCOMPtr<nsIDOMXPathResult> xpResult =
      nsXFormsUtils::EvaluateXPath(valueAttr, singleNode, mElement,
                                   nsIDOMXPathResult::STRING_TYPE);
    if (!xpResult)
      return NS_OK;
    xpResult->GetStringValue(value);
  }
  else {
    nsCOMPtr<nsIDOM3Node> n3(do_QueryInterface(mElement));
    n3->GetTextContent(value);
  }

  PRBool changed;
  rv = modelPriv->SetNodeValue(singleNode, value, &changed);
  NS_ENSURE_SUCCESS(rv, rv);

  if (changed) {
    nsCOMPtr<nsIDOMNode> model = do_QueryInterface(modelPriv);
    NS_ENSURE_STATE(model);
    if (aParentAction) {
      aParentAction->SetRecalculate(model, PR_TRUE);
      aParentAction->SetRevalidate(model, PR_TRUE);
      aParentAction->SetRefresh(model, PR_TRUE);
    } else {
      nsXFormsUtils::DispatchEvent(model, eEvent_Recalculate);
      nsXFormsUtils::DispatchEvent(model, eEvent_Revalidate);
      nsXFormsUtils::DispatchEvent(model, eEvent_Refresh);
    }
  }

  return NS_OK;
}

NS_HIDDEN_(nsresult)
NS_NewXFormsSetValueElement(nsIXTFElement **aResult)
{
  *aResult = new nsXFormsSetValueElement();
  if (!*aResult)
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ADDREF(*aResult);
  return NS_OK;
}

