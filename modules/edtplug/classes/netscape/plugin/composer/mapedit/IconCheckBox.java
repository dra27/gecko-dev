/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
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

package netscape.plugin.composer.mapedit;


import java.awt.*;


abstract public class IconCheckBox extends Canvas {
  public IconCheckBox(IconCheckBoxGroup g,boolean s,OkCallback cb) {
    state = s;
    grp = g;
    g.add(this);
    okCB = cb;
  }

  public boolean getState() {
    return state;
  }

  public Dimension preferredSize() {
    return new Dimension(WIDTH,HEIGHT);
  }

  public Dimension minimumSize() {
    return new Dimension(WIDTH,HEIGHT);
  }

  public void setState(boolean val) {
    if (val != state) {
      state = val;
      if (state) {
        // call callback.
        okCB.ok();
      }
      if (grp != null) {
        grp.stateChanged(this,val);
      }
      repaint();
    }
  }

  public boolean mouseDown(Event evt, int x, int y) {
    setState(!state);
    return true;
  }

  private boolean state;
  private IconCheckBoxGroup grp;
  private OkCallback okCB;

  protected final static int WIDTH = 20;
  protected final static int HEIGHT = 20;
}
