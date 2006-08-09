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

#include "nsMsgTxn.h"
#include "nsIMsgHdr.h"

NS_IMPL_ISUPPORTS_INHERITED1(nsMsgTxn, nsHashPropertyBag, nsITransaction)

nsMsgTxn::nsMsgTxn() 
{
  m_txnType = 0;
}

nsMsgTxn::~nsMsgTxn()
{
}

NS_IMETHODIMP nsMsgTxn::DoTransaction(void)
{
  return NS_OK;
}

NS_IMETHODIMP nsMsgTxn::GetIsTransient(PRBool *aIsTransient)
{
  if (nsnull!=aIsTransient)
    *aIsTransient = PR_FALSE;
  else
	  return NS_ERROR_NULL_POINTER;
  return NS_OK;
}

NS_IMETHODIMP nsMsgTxn::Merge(nsITransaction *aTransaction, PRBool *aDidMerge)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}


nsresult nsMsgTxn::GetMsgWindow(nsIMsgWindow **msgWindow)
{
    if (!msgWindow || !m_msgWindow)
        return NS_ERROR_NULL_POINTER;
    *msgWindow = m_msgWindow;
    NS_ADDREF (*msgWindow);
    return NS_OK;
}

nsresult nsMsgTxn::SetMsgWindow(nsIMsgWindow *msgWindow)
{
    m_msgWindow = msgWindow;
    return NS_OK;
}


nsresult
nsMsgTxn::SetTransactionType(PRUint32 txnType)
{
  return SetPropertyAsUint32(NS_LITERAL_STRING("type"), txnType);
}

/*none of the callers pass null aFolder, 
  we always initialize aResult (before we pass in) for the case where the key is not in the db*/
nsresult 
nsMsgTxn::CheckForToggleDelete(nsIMsgFolder *aFolder, const nsMsgKey &aMsgKey, PRBool *aResult)
{
  NS_ENSURE_ARG(aResult);
  nsCOMPtr<nsIMsgDBHdr> message;
  nsCOMPtr<nsIMsgDatabase> db;
  nsresult rv = aFolder->GetMsgDatabase(nsnull,getter_AddRefs(db));
  if (db)
  {
    PRBool containsKey;
    rv = db->ContainsKey(aMsgKey, &containsKey);
    if (NS_FAILED(rv) || !containsKey)   // the message has been deleted from db, so we cannot do toggle here
      return NS_OK;
    rv = db->GetMsgHdrForKey(aMsgKey, getter_AddRefs(message));
    PRUint32 flags;
    if (NS_SUCCEEDED(rv) && message)
    {
      message->GetFlags(&flags);
      *aResult = (flags & MSG_FLAG_IMAP_DELETED) != 0;
    }
  }
  return rv;
}
