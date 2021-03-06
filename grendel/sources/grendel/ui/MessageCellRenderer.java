/* -*- Mode: java; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is the Grendel mail/news client.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are
 * Copyright (C) 1997 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 *
 * Created: Will Scullin <scullin@netscape.com>, 17 Dec 1997.
 */

package grendel.ui;

import java.awt.Font;
import java.awt.Graphics;

import javax.mail.Message;
import javax.mail.MessagingException;

import grendel.storage.MessageExtraFactory;
import grendel.view.ViewedMessage;
import grendel.widgets.DefaultCellRenderer;

//
// MessageCellRenderer Class
//

class MessageCellRenderer extends DefaultCellRenderer {
  Font          fPlain;
  Font          fBold;
  Font          fItalic;

  public MessageCellRenderer() {
    fPlain = Font.decode("SansSerif-12");
    fBold = Font.decode("SansSerif-bold-12");
    fItalic = Font.decode("SansSerif-italic-12");
  }

  public void paint(Graphics g) {
    Font font = fPlain;
    if (fObject != null) {
      Message m = ((ViewedMessage) fObject).getMessage();
      if (m == null) {
        font = fItalic;
      } else {
        try {
          font = MessageExtraFactory.Get(m).isRead() ? fPlain : fBold;
        } catch (MessagingException e) {
        }
      }
    }
    setFont(font);

    super.paint(g);
  }
}
