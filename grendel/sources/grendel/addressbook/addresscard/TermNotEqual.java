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
 * Created: Lester Schueler <lesters@netscape.com>, 14 Nov 1997.
 *
 * Contributors: Jeff Galyan <talisman@anamorphic.com>
 *               Christoph Toshok <toshok@netscape.com>
 */

package grendel.addressbook.addresscard;

//************************
//************************
public class TermNotEqual implements ITerm {
    private IAttribute fAttribute;

    /**
     */
    public TermNotEqual (IAttribute anAttribute) {
        fAttribute = anAttribute;
    }

    /**
     */
    public String getExpression(IQueryString iqs) {
        return iqs.opNotEqual (fAttribute);
    }

    /**
     */
    public AC_IDSet getSet (IQuerySet iqs) {
        return iqs.opNotEqual (fAttribute);
    }

  /** No-op for now
   *  (Jeff)
   */
  public AC_IDSet evaluate_ACSP (IQuerySet iqs) { return null; }


}
