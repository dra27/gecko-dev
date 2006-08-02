/* -*- Mode: javascript; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Sun Microsystems, Inc.
 * Portions created by Sun Microsystems are Copyright (C) 2006 Sun
 * Microsystems, Inc. All Rights Reserved.
 *
 * Original Author: Daniel Boelzle (daniel.boelzle@sun.com)
 *
 * Contributor(s):
 *
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

// xxx todo: just to avoid registration errors, how to do better?
var calWcapCalendar;
if (!calWcapCalendar) {
    calWcapCalendar = {};
    calWcapCalendar.prototype = {};
}

calWcapCalendar.prototype.encodeAttendee = function(
    /* calIAttendee */ attendee, forceRSVP )
{
    this.log( "\tattendee.icalProperty.icalString=" +
              attendee.icalProperty.icalString +
              "\n\tattendee.id=" + attendee.id +
              "\n\tattendee.commonName=" + attendee.commonName +
              "\n\tattendee.role=" + attendee.role +
              "\n\tattendee.participationStatus=" +
              attendee.participationStatus );
    var ret = ("RSVP=" + (forceRSVP || attendee.rsvp ? "TRUE" : "FALSE"));
    if (attendee.participationStatus != null)
        ret += ("^PARTSTAT=" + attendee.participationStatus);
    if (attendee.role != null)
        ret += ("^ROLE=" + attendee.role);
    if (attendee.commonName != null)
        ret += ("^CN=" + encodeURIComponent(attendee.commonName));
    ret += ("^" + attendee.id);
    return ret;
};

calWcapCalendar.prototype.encodeRecurrenceParams = function( item, oldItem )
{
    var rrules = {};
    var rdates = {};
    var exrules = {};
    var exdates = {};
    this.getRecurrenceParams( item, rrules, rdates, exrules, exdates );
    if (oldItem) {
        // actually only write changes if an old item has been changed, because
        // cs recreates the whole series if a rule has changed.
        // xxx todo: one problem is left when a master only holds EXDATEs,
        //           and effectively no item is present anymore.
        //           cs seems not to clean up automatically, but it does when
        //           when deleting an occurrence {id, rec-id}!
        //           So this still leaves the question open why deleteOccurrence
        //           does not directly call deleteItem rather than modifyItem,
        //           which leads to a much cleaner usage.
        //           I assume this mimic has been chosen for easier undo/redo
        //           support (Undo would then have to distinguish whether
        //           it has previously deleted an occurrence or ordinary item:
        //            - entering an exception again
        //            - or adding an item)
        //           Currently it can just modifyItem(newItem/oldItem) back.
        var rrules_ = {};
        var rdates_ = {};
        var exrules_ = {};
        var exdates_ = {};
        this.getRecurrenceParams(oldItem, rrules_, rdates_, exrules_, exdates_);
        
        function sameSet( list, list_ ) {
            return (list.length == list_.length &&
                    list.every( function(x) {
                                    return list_.some(
                                        function(y) { return x == y; } );
                                }
                        ));
        }
        if (sameSet( rrules.value, rrules_.value ))
            rrules.value = null; // don't write
        if (sameSet( rdates.value, rdates_.value ))
            rdates.value = null; // don't write
        if (sameSet( exrules.value, exrules.value ))
            exrules.value = null; // don't write
        if (sameSet( exdates.value, exdates_.value ))
            exdates.value = null; // don't write
    }
    
    function encodeList( list ) {
        var ret = "";
        for each ( var str in list ) {
            if (ret.length > 0)
                ret += ";";
            ret += str;
        }
        return ret;
    }
    var ret = "";
    if (rrules.value != null)
        ret += ("&rrules=" + encodeList(rrules.value));
    if (rdates.value != null)
        ret += ("&rdates=" + encodeList(rdates.value));
    if (exrules.value != null)
        ret += ("&exrules=" + encodeList(exrules.value));
    if (exdates.value != null)
        ret += ("&exdates=" + encodeList(exdates.value));
    return ret;
    // xxx todo:
    // rchange=1: expand recurrences,
    // or whether to replace the rrule, ambiguous documentation!!!
    // check with store(with no uid) upon adoptItem() which behaves strange
    // if rchange=0 is set!
};

calWcapCalendar.prototype.getRecurrenceParams = function(
    item, out_rrules, out_rdates, out_exrules, out_exdates )
{
    // recurrences:
    out_rrules.value = [];
    out_rdates.value = [];
    out_exrules.value = [];
    out_exdates.value = [];
    if (item.recurrenceInfo != null) {
        var rItems = item.recurrenceInfo.getRecurrenceItems( {} );
        for each ( var rItem in rItems ) {
            var isNeg = rItem.isNegative;
            // xxx todo: need to QueryInterface() here?
            if (rItem instanceof Components.interfaces.calIRecurrenceRule)
            {
                var rule = ("\"" + encodeURIComponent(
                                rItem.icalProperty.valueAsIcalString) +
                            "\"");
                if (isNeg)
                    out_exrules.value.push( rule );
                else
                    out_rrules.value.push( rule );
            }
            // xxx todo: need to QueryInterface() here?
            else if (
                rItem instanceof Components.interfaces.calIRecurrenceDateSet)
            {
                var d = rItem.getDates({});
                for each ( var d in rdates ) {
                    if (isNeg)
                        out_exdates.value.push( getIcalUTC(d.date) );
                    else
                        out_rdates.value.push( getIcalUTC(d.date) );
                }
            }
            // xxx todo: need to QueryInterface() here?
            else if (
                rItem instanceof Components.interfaces.calIRecurrenceDate)
            {
                if (isNeg)
                    out_exdates.value.push( getIcalUTC(rItem.date) );
                else
                    out_rdates.value.push( getIcalUTC(rItem.date) );
            }
            else {
                this.notifyError(
                    "don\'t know how t handle this recurrence item: " +
                    rItem.valueAsIcalString );
            }
        }
    }
};

calWcapCalendar.prototype.getStoreUrl = function( item )
{
    var bIsEvent = isEvent(item);
    var url = this.session.getCommandUrl( bIsEvent ? "storeevents"
                                                   : "storetodos" );
    url += "&fetch=1&compressed=1&recurring=1";
    url += ("&calid=" + encodeURIComponent(this.calId));
    
    // it is always safe to use orgCalId,
    // because every user has a calId == userId
    var orgCalid = ((item.organizer == null || item.organizer.id == null)
                    ? this.calId // sensible default
                    : item.organizer.id);
    url += ("&orgCalid=" + encodeURIComponent(orgCalid));
    
    // xxx todo: default prio is 0 (5 in sjs cs)
    url += ("&priority=" + item.priority);
    url += "&replace=1"; // (update) don't append to any lists
    url += ("&icsClass="+ ((item.privacy != null && item.privacy != "")
                           ? item.privacy : "PUBLIC"));
    if (!bIsEvent && item.isCompleted) {
        url += "&status=4"; // force to COMPLETED
    }
    else {
        switch (item.status) {
        case "CONFIRMED":    url += "&status=0"; break;
        case "CANCELLED":    url += "&status=1"; break;
        case "TENTATIVE":    url += "&status=2"; break;
        case "NEEDS-ACTION": url += "&status=3"; break;
        case "COMPLETED":    url += "&status=4"; break;
        case "IN-PROCESS":   url += "&status=5"; break;
        case "DRAFT":        url += "&status=6"; break;
        case "FINAL":        url += "&status=7"; break;
        default:
            url += "&status=3"; // NEEDS-ACTION
//             this.logError( "getStoreUrl(): unexpected item status=" +
//                            item.status );
            break;
        }
    }
    
    // attachment urls:
    url += "&attachments=";
    var attachments = item.attachments;
    if (attachments != null) {
        for ( var i = 0; i < attachments.length; ++i ) {
            if (i > 0)
                url += ";";
            var obj = attachments[i];
            if (obj instanceof String) {
                url += encodeURIComponent(obj);
            }
            else {
                this.notifyError(
                    "only URLs supported as attachment, not: " + obj );
            }
        }
    }
    
    // categories:
    // xxx todo: check whether ;-separated:
    url += "&categories=";
    if (item.hasProperty( "CATEGORIES" )) {
        url += encodeURIComponent(
            item.getProperty( "CATEGORIES" ).replace(/,/g, ";") );
    }
    
    // xxx todo: contacts, resources missing in calAPI
    // xxx todo: missing relatedTos= in cal api
    
    url += ("&summary=" + encodeURIComponent(item.title));
    // desc: xxx todo attribute "description" not impl in calItemBase.js
    url += "&desc=";
    if (item.hasProperty( "DESCRIPTION" )) {
        url += encodeURIComponent( item.getProperty( "DESCRIPTION" ) );
    }
    // location: xxx todo currently not impl in calItemBase.js
    url += "&location=";
    if (item.hasProperty( "LOCATION" )) {
        url += encodeURIComponent( item.getProperty( "LOCATION" ) );
    }
    url += "&icsUrl=";
    if (item.hasProperty( "URL" )) {
        url += encodeURIComponent( item.getProperty( "URL" ) );
    }
    
    // attendees:
    var attendees =  item.getAttendees({});
    var forceRSVP = false;
    
    var dtstart = null;
    var dtend = null; // for alarmRelated
    
    if (bIsEvent) {
        if (attendees != null && attendees.length > 0) {
            // ORGANIZER is this cal?
            if (orgCalid == this.calId) {
                url += "&method=2"; // REQUEST
                forceRSVP = true;
            }
            else {
                var userId = this.session.userId;
                if (userId == null)
                    userId = this.calId; // fallback
                var i = 0;
                for ( ; i < attendees.length; ++i ) {
                    if (attendees[i].id == userId) {
                        // REPLY for just this user:
                        url += "&method=4";
                        attendees = [ attendees[i] ];
                        break;
                    }
                }
                if (i >= attendees.length) {
                    // user not in list, don't write attendee list:
                    attendees = null;
                }
            }
        } // else just PUBLISH (default)
        
        dtstart = item.startDate;
        var dtend = item.endDate;
        url += ("&dtend=" + getIcalUTC(dtend));
//         url += ("&X-NSCP-DTEND-TZID=" + 
//                 encodeURIComponent(this.getAlignedTimezone(dtend.timezone)));
        if (dtstart.isDate && dtend.isDate)
            url += "&isAllDay=1";
    }
    else { // calITodo:
        // xxx todo: dtstart is mandatory for cs, so if this is
        //           undefined, assume an allDay todo???
        dtstart = item.entryDate;
        dtend = item.dueDate;
        if (dtend) {
            url += ("&due=" + getIcalUTC(dtend));
//         url += ("&X-NSCP-DUE-TZID=" + encodeURIComponent(
//                     this.getAlignedTimezone(dtend.timezone)));
        }
        // xxx todo: missing duration
        if (dtstart && dtstart.isDate)
            url += "&isAllDay=1";
        if (item.isCompleted)
            url += "&percent=100";
        else
            url += ("&percent=" +
                    (item.percentComplete ? item.percentComplete : "0"));
        url += "&completed=";
        if (item.completedDate != null)
            url += getIcalUTC(item.completedDate);
        else if (item.isCompleted)
            url += getIcalUTC(getTime()); // repair missing completedDate
        else
            url += "0"; // not yet completed
        // xxx todo: sentBy sentUID fields in cs: missing in cal api
    }
    
    if (dtstart) {
        // important to provide tz info with entry date for proper
        // occurrence calculation (daylight savings)
        url += ("&dtstart=" + getIcalUTC(dtstart));
        // xxx todo: setting X-NSCP- does not work.
        //           i.e. no separate tz for start/end. WTF.
//     url += ("&X-NSCP-DTSTART-TZID=" +
//             encodeURIComponent(this.getAlignedTimezone(dtstart.timezone)));
        // currently the only way to influence X-NSCP-DTSTART-TZID:
        url += ("&tzid=" + encodeURIComponent(
                    this.getAlignedTimezone(dtstart.timezone)));
    }
    
    // alarm support:
    var alarmOffset = item.alarmOffset;
    if (alarmOffset) {
        var alarmStart = null;
        if (item.alarmRelated ==
            Components.interfaces.calIItemBase.ALARM_RELATED_END) {
            if (!dtend)
                this.logError("no end date (no end nor due) for alarm!");
            else {
                alarmStart = dtend.clone();
                alarmOffset = alarmOffset.clone();
                alarmOffset.isNegative = !alarmOffset.isNegative;
            }
        }
        else if (dtstart)
            alarmStart = dtstart.clone(); // default
        
        if (alarmStart != null) {
            alarmStart.addDuration(alarmOffset);
            var zalarmStart = getIcalUTC(alarmStart);
            url += ("&alarmStart=" + zalarmStart);
            // xxx todo: verify ;-separated addresses
            url += "&alarmEmails=";
            if (item.hasProperty( "alarmEmailAddress" )) {
                url += encodeURIComponent(
                    item.getProperty("alarmEmailAddress") );
            }
            else {
                // xxx todo: popup exor emails can be currently specified...
                url += ("&alarmPopup=" + zalarmStart);
            }
            // xxx todo: missing: alarm triggers for flashing, etc.
        }
    }
    else {
        // clear alarm:
        url += "&alarmStart=&alarmPopup=&alarmEmails=";
    }
    // xxx todo: currently no support to store this at VALARM component...
    var alarmLastAck = item.alarmLastAck;
    if (alarmLastAck) {
        url += ("&X-MOZ-LASTACK=" +
                "X-NSCP-ORIGINAL-OPERATION=X-NSCP-WCAP-PROPERTY-REPLACE^" +
                getIcalUTC(alarmLastAck));
    }
    
    // attendees:
    url += "&attendees=";
    if (attendees != null) {
        for ( var i = 0; i < attendees.length; ++i ) {
            if (i > 0)
                url += ";";
            url += this.encodeAttendee( attendees[i], forceRSVP );
        }
    }
    
    return url;
};

calWcapCalendar.prototype.adoptItem_resp = function( wcapResponse, iListener )
{
    var item = null;
    try {
        var icalRootComp = wcapResponse.data; // first statement, may throw
        
        var items = this.parseItems(
            icalRootComp,
            Components.interfaces.calICalendar.ITEM_FILTER_ALL_ITEMS,
            0, null, null );
        if (items.length < 1)
            throw new Error("empty VCALENDAR returned!");
        if (items.length > 1)
            this.notifyError( "unexpected number of items: " + items.length );
        item = items[0];
        
        this.log( "item.id=" + item.id );        
        if (iListener != null) {
            iListener.onOperationComplete(
                this.superCalendar, Components.results.NS_OK,
                Components.interfaces.calIOperationListener.ADD,
                item.id, item );
        }
        this.notifyObservers( "onAddItem", [item] );
        // xxx todo: maybe log request status
    }
    catch (exc) {
        if (iListener != null) {
            iListener.onOperationComplete(
                this.superCalendar, Components.results.NS_ERROR_FAILURE,
                Components.interfaces.calIOperationListener.ADD,
                item == null ? null : item.id, exc );
        }
        this.notifyError( exc );
    }
};

calWcapCalendar.prototype.adoptItem = function( item, iListener )
{
    this.log( "adoptItem() call: " + item.title );
    if (this.readOnly)
        throw Components.interfaces.calIErrors.CAL_IS_READONLY;
    try {
        // xxx todo: really necessary when adding?
        if (item.parentItem != item) {
            this.logError( "adoptItem(): unexpected proxy!" );
            debugger;
            item.parentItem.recurrenceInfo.modifyException( item );
            // ensure that we're looking at the base item
            // if we were given an occurrence.
            item = item.parentItem;
            // will most probably lead to error => existing parent
        }
        
        var url = this.getStoreUrl( item );
        url += this.encodeRecurrenceParams( item );
        // (WCAP_STORE_TYPE_CREATE) error if existing item:
        url += "&storetype=1";
        
        var this_ = this;
        this.session.issueAsyncRequest(
            url + "&fmt-out=text%2Fcalendar", stringToIcal,
            function( wcapResponse ) {
                this_.adoptItem_resp( wcapResponse, iListener );
            } );
    }
    catch (exc) {
        if (iListener != null) {
            iListener.onOperationComplete(
                this.superCalendar, Components.results.NS_ERROR_FAILURE,
                Components.interfaces.calIOperationListener.ADD,
                item.id, exc );
        }
        this.notifyError( exc );
    }
    this.log( "adoptItem() returning." );
};

calWcapCalendar.prototype.addItem = function( item, iListener )
{
    this.adoptItem( item.clone(), iListener );
};

calWcapCalendar.prototype.modifyItem_resp = function(
    wcapResponse, newItem_, oldItem, iListener )
{
    var newItem = null;
    try {
        var icalRootComp = wcapResponse.data; // first statement, may throw
        
        var items = this.parseItems(
            icalRootComp,
            Components.interfaces.calICalendar.ITEM_FILTER_ALL_ITEMS,
            0, null, null );
        if (items.length > 1)
            this.notifyError( "unexpected number of items: " + items.length );
        if (items.length < 1) {
            // however, sometimes socs just returns an empty calendar when
            // nothing has changed, although fetch=1.
            // This also occurs when the event organizer is in the attendees
            // list and switches its PARTSTAT too often... WTF.
            this.log( "empty VCALENDAR returned!" );
            newItem = newItem_; // fallback, assuming item has been written
        }
        else
            newItem = items[0];
        
        if (iListener != null) {
            iListener.onOperationComplete(
                this.superCalendar, Components.results.NS_OK,
                Components.interfaces.calIOperationListener.MODIFY,
                newItem.id, newItem );
        }
        this.notifyObservers( "onModifyItem", [newItem, oldItem] );
        // xxx todo: maybe log request status
    }
    catch (exc) {
        if (iListener != null) {
            iListener.onOperationComplete(
                this.superCalendar, Components.results.NS_ERROR_FAILURE,
                Components.interfaces.calIOperationListener.MODIFY,
                newItem == null ? null : newItem.id, exc );
        }
        this.notifyError( exc );
    }
};

calWcapCalendar.prototype.modifyItem = function(
    newItem, oldItem, iListener )
{
    this.log( "modifyItem() call: " + newItem.id );
    if (this.readOnly)
        throw Components.interfaces.calIErrors.CAL_IS_READONLY;
    
    try {
        if (!newItem.id)
            throw new Error("new item has no id!");
        
        var url = this.getStoreUrl( newItem );
        url += ("&uid=" + newItem.id);
        if (newItem.parentItem == newItem) { // is master
            // (WCAP_STORE_TYPE_MODIFY) error if not existing:
            url += "&storetype=2";
            url += "&mod=4"; // THIS AND ALL INSTANCES
            url += this.encodeRecurrenceParams( newItem, oldItem );
        }
        else {
            // modifying occurences lands here: occurence may not exist
            url += "&mod=1"; // THIS INSTANCE
            // if not set, whole series of events is modified:
            var rid = getIcalUTC(newItem.recurrenceId);
            url += ("&rid=" + rid);
        }
        
        var this_ = this;
        this.session.issueAsyncRequest(
            url + "&fmt-out=text%2Fcalendar", stringToIcal,
            function( wcapResponse ) {
                this_.modifyItem_resp( wcapResponse,
                                       newItem, oldItem, iListener );
            } );
    }
    catch (exc) {
        if (iListener != null) {
            iListener.onOperationComplete(
                this.superCalendar, Components.results.NS_ERROR_FAILURE,
                Components.interfaces.calIOperationListener.MODIFY,
                newItem.id, exc );
        }
        this.notifyError( exc );
    }
    this.log( "modifyItem() returning." );
};

calWcapCalendar.prototype.deleteItem_resp = function(
    wcapResponse, item, iListener )
{
    try {
        var xml = wcapResponse.data; // first statement, may throw
        
        // xxx todo: need to notify about each deleted item if multiple?
        if (item.isMutable) {
            item.makeImmutable();
        }
        if (iListener != null) {
            iListener.onOperationComplete(
                this.superCalendar, Components.results.NS_OK,
                Components.interfaces.calIOperationListener.DELETE,
                item.id, item );
        }
        this.notifyObservers( "onDeleteItem", [item] );
        if (LOG_LEVEL > 0) {
            this.log( "deleteItem_resp(): " +
                      getWcapRequestStatusString(xml) );
        }
    }
    catch (exc) {
        if (iListener != null) {
            iListener.onOperationComplete(
                this.superCalendar, Components.results.NS_ERROR_FAILURE,
                Components.interfaces.calIOperationListener.DELETE,
                item.id, exc );
        }
        this.notifyError( exc );
    }
};

calWcapCalendar.prototype.deleteItem = function( item, iListener )
{
    this.log( "deleteItem() call: " + item.id );
    if (this.readOnly)
        throw Components.interfaces.calIErrors.CAL_IS_READONLY;
    try {
        if (item.id == null)
            throw new Error("no item id!");
        
        var url = this.session.getCommandUrl(
            isEvent(item) ? "deleteevents_by_id" : "deletetodos_by_id" );
        url += ("&calid=" + encodeURIComponent(this.calId));
        url += ("&uid=" + item.id);
        
        var rid = item.recurrenceId;
        if (rid == null) {
            if (item != item.parentItem)
                throw new Error("proxy has no recurrenceId!");
            url += "&mod=4&rid=0"; // deleting THIS AND ALL
        }
        else {
            rid = getIcalUTC(rid);
            if (item == item.parentItem)
                this.notifyError( ">>>>>>>>>> non-proxy with rid: " + rid );
            url += ("&mod=1&rid=" + rid); // delete THIS INSTANCE
        }
        
        var this_ = this;
        this.session.issueAsyncRequest(
            url + "&fmt-out=text%2Fxml", stringToXml,
            function( wcapResponse ) {
                this_.deleteItem_resp( wcapResponse, item, iListener );
            } );
    }
    catch (exc) {
        if (iListener != null) {
            iListener.onOperationComplete(
                this.superCalendar, Components.results.NS_ERROR_FAILURE,
                Components.interfaces.calIOperationListener.DELETE,
                item.id, exc );
        }
        this.notifyError( exc );
    }
    this.log( "deleteItem() returning." );
};

calWcapCalendar.prototype.parseItems = function(
    icalRootComp, itemFilter, maxResult, rangeStart, rangeEnd )
{
    var unexpandedItems = [];
    var uid2parent = {};
    var excItems = [];
    
    var componentType = "ANY";
    switch (itemFilter &Components.interfaces.calICalendar.ITEM_FILTER_TYPE_ALL)
    {
    case Components.interfaces.calICalendar.ITEM_FILTER_TYPE_TODO:
        componentType = "VTODO"; break;
    case Components.interfaces.calICalendar.ITEM_FILTER_TYPE_EVENT:
        componentType = "VEVENT"; break;
    }
    
    var this_ = this;
    forEachIcalComponent(
        icalRootComp, componentType,
        function( subComp )
        {
            function patchTimezone( subComp, attr, xprop ) {
                var dt = subComp[attr];
                if (dt != null) {
                    if (LOG_LEVEL > 2) {
                        this_.log( attr + " is " + dt );
                    }
                    var tzid = subComp.getFirstProperty( xprop );
                    if (tzid != null) {
                        subComp[attr] = dt.getInTimezone(tzid.value);
                        if (LOG_LEVEL > 2) {
                            this_.log( "patching " + xprop + " from " +
                                       dt + " to " + subComp[attr] );
                        }
                    }
                }
            }

            patchTimezone( subComp, "startTime", "X-NSCP-DTSTART-TZID" );
            var item = null;
            switch (subComp.componentType) {
            case "VEVENT": {
                patchTimezone( subComp, "endTime", "X-NSCP-DTEND-TZID" );
                item = new CalEvent();
                item.icalComponent = subComp;
                break;
            }
            case "VTODO": {
                patchTimezone( subComp, "dueTime", "X-NSCP-DUE-TZID" );
                item = new CalTodo();
                item.icalComponent = subComp;
                switch (itemFilter & Components.interfaces.calICalendar
                                     .ITEM_FILTER_COMPLETED_ALL)
                {
                    case Components.interfaces.calICalendar
                        .ITEM_FILTER_COMPLETED_YES:
                        if (!item.isCompleted) {
                            delete item;
                            item = null;
                        }
                    break;
                    case Components.interfaces.calICalendar
                        .ITEM_FILTER_COMPLETED_NO:
                        if (item.isCompleted) {
                            delete item;
                            item = null;
                        }
                    break;
                }
                // assure correct TRIGGER;RELATED if VALARM,
                // cs does not constrain start/due with correct RELATED:
                if (item != null && item.alarmOffset &&
                    item.alarmRelated == Components.interfaces
                                         .calIItemBase.ALARM_RELATED_START &&
                    !item.entryDate) {
                    if (item.dueDate) {
                        // patch missing RELATED=END:
                        item.alarmRelated = Components.interfaces
                                            .calIItemBase.ALARM_RELATED_END;
                    }
                    else {
                        this_.log( "todo has no entryDate nor dueDate, " +
                                   "but VALARM => removing alarm!" );
                        item.alarmOffset = null;
                    }
                }
                break;
            }
            }
            if (item != null) {
                var lastAckProp = subComp.getFirstProperty("X-MOZ-LASTACK");
                if (lastAckProp) { // shift to alarm comp:
                    // TZID is UTC:
                    item.alarmLastAck = getDatetimeFromIcalProp( lastAckProp );
                }
                
                item.calendar = this_.superCalendar;
                var rid = subComp.getFirstProperty("RECURRENCE-ID");
                if (rid == null) {
                    unexpandedItems.push( item );
                    if (item.recurrenceInfo != null)
                        uid2parent[item.id] = item;
                }
                else {
                    // xxx todo: IMO ought not be needed here: TZID is UTC
                    var dtrid = getDatetimeFromIcalProp( rid );
                    if (LOG_LEVEL > 1) {
                        this_.log( "exception item: rid=" + dtrid.icalString );
                    }
                    item.recurrenceId = dtrid;
                    // force no recurrence info:
                    item.recurrenceInfo = null;
                    excItems.push( item );
                }
                
                if (item.title == null &&
                    (item.privacy == "PRIVATE" ||
                     item.privacy == "CONFIDENTIAL")) {
                    // assumed to look at a subscribed calendar,
                    // so patch title for private items:
                    item.title = g_privateItemTitle;
                }
            }
        },
        maxResult );
    
    // tag "exceptions", i.e. items with rid:
    for each ( var item in excItems ) {
        var parent = uid2parent[item.id];
        if (parent == null) {
            this.logError( "getItems_resp(): no parent item for rid=" +
                           item.recurrenceId );
        }
        else {
            item.parentItem = parent;
            item.makeImmutable();
            parent.recurrenceInfo.modifyException( item );
        }
    }
    
    var items = [];
    for ( var i = 0;
          (i < unexpandedItems.length) &&
              (maxResult == 0 || items.length < maxResult);
          ++i )
    {
        var item = unexpandedItems[i];
        item.makeImmutable();
        if (item.recurrenceInfo != null &&
            (itemFilter & Components.interfaces.calICalendar
             .ITEM_FILTER_CLASS_OCCURRENCES))
        {
            var occurrences = item.recurrenceInfo.getOccurrences(
                rangeStart, rangeEnd,
                maxResult == 0 ? 0 : maxResult - items.length,
                {} );
            if (LOG_LEVEL > 1) {
                this.log( "item: " + item.title + " has " +
                          occurrences.length.toString() + " occurrences." );
            }
            // only proxies returned:
            items = items.concat( occurrences );
        }
        else {
            items.push( item );
        }
    }
    
    if (LOG_LEVEL > 1) {
        this.log( "parseItems(): returned " + items.length + " items" );
        if (LOG_LEVEL > 2) {
            for each ( var item in items ) {
                this.log( "item: " + item.title + "\n" + item.icalString );
            }
        }
    }
    return items;
};

calWcapCalendar.prototype.getItem = function( id, iListener )
{
    // xxx todo: test
    // xxx todo: howto detect whether to call
    //           fetchevents_by_id ot fetchtodos_by_id?
    //           currently drag/drop is implemented for events only,
    //           try events first, fallback to todos... in the future...
    this.log( ">>>>>>>>>>>>>>>> getItem() call!");
    try {
        var this_ = this;
        var syncResponseFunc = function( wcapResponse ) {
            var icalRootComp = wcapResponse.data; // first statement, may throw
            var items = this_.parseItems(
                icalRootComp,
                Components.interfaces.calICalendar.ITEM_FILTER_ALL_ITEMS,
                1, null, null );
            if (items.length < 1)
                throw new Error("no such item!");
            if (items.length > 1) {
                this_.notifyError(
                    "unexpected number of items: " + items.length );
            }
            item = items[0];
            if (iListener != null) {
                iListener.onGetResult(
                    this_.superCalendar, Components.results.NS_OK,
                    Components.interfaces.calIItemBase,
                    this_.log( "getItems_resp(): success." ),
                    items.length, items );
                iListener.onOperationComplete(
                    this_.superCalendar, Components.results.NS_OK,
                    Components.interfaces.calIOperationListener.GET,
                    items.length == 1 ? items[0].id : null, null );
            }
            this_.log( "item delivered." );
        };
        
        var params = "&compressed=1&recurring=1&fmt-out=text%2Fcalendar";
        params += ("&calid=" + encodeURIComponent(this.calId));
        params += ("&uid=" + id);
        try {
            // most common: event
            this.session.issueSyncRequest(
                this.session.getCommandUrl( "fetchevents_by_id" ) + params,
                stringToIcal, syncResponseFunc );
        }
        catch (exc) {
            // try again, may be a task:
            this.session.issueSyncRequest(
                this.session.getCommandUrl( "fetchtodos_by_id" ) + params,
                stringToIcal, syncResponseFunc );
        }
    }
    catch (exc) {
        if (iListener != null) {
            iListener.onOperationComplete(
                this.superCalendar, Components.results.NS_ERROR_FAILURE,
                Components.interfaces.calIOperationListener.GET,
                null, exc );
        }
        this.notifyError( exc );
    }
    this.log( "getItem() returning." );
};

calWcapCalendar.prototype.getItems_resp = function(
    wcapResponse,
    itemFilter, maxResult, rangeStart, rangeEnd, iListener )
{
    try {
        var exc = wcapResponse.exception;
        // check  whether access is denied,
        // then show free-busy information instead:
        if (exc && (exc == Components.interfaces.
                           calIWcapErrors.WCAP_ACCESS_DENIED_TO_CALENDAR)) {
            if (iListener != null) {
                var this_ = this;
                var freeBusyListener = { // calIWcapFreeBusyListener:
                    onGetFreeBusyTimes:
                    function( rc, requestId, calId, count, entries )
                    {
                        if (rc == Components.results.NS_OK) {
                            var items = [];
                            for each ( var entry in entries ) {
                                var item = new CalEvent();
                                item.id = (g_busyItemUuidPrefix +
                                           entry.dtRangeStart.icalString);
                                item.calendar = this_.superCalendar;
                                item.title = g_busyItemTitle;
                                item.startDate = entry.dtRangeStart;
                                item.endDate = entry.dtRangeEnd;
                                item.makeImmutable();
                                items.push(item);
                            }
                            iListener.onGetResult(
                                this_.superCalendar, Components.results.NS_OK,
                                Components.interfaces.calIItemBase,
                                this_.log( "getItems_resp() using free-busy " +
                                           "information: success." ),
                                items.length, items );
                            iListener.onOperationComplete(
                                this_.superCalendar, Components.results.NS_OK,
                                Components.interfaces.calIOperationListener.GET,
                                items.length == 1 ? items[0].id : null, null );
                            this_.log(
                                items.length.toString() + " items delivered." );
                        }
                        else {
                            // if even availability is denied:
                            iListener.onOperationComplete(
                                this_.superCalendar,
                                Components.results.NS_ERROR_FAILURE,
                                Components.interfaces.calIOperationListener.GET,
                                null, rc );
                        }
                    }
                };
                this.session.getFreeBusyTimes(
                    this.calId, rangeStart, rangeEnd, true /*bBusyOnly*/,
                    freeBusyListener,
                    true /*bAsync*/, 0 /*requestId*/ );
            }
            return;
        }
        
        var icalRootComp = wcapResponse.data; // first statement, may throw
        
        var items = this.parseItems(
            icalRootComp, itemFilter, maxResult, rangeStart, rangeEnd );
        
        if (iListener != null) {
            iListener.onGetResult( this.superCalendar, Components.results.NS_OK,
                                   Components.interfaces.calIItemBase,
                                   this.log( "getItems_resp(): success." ),
                                   items.length, items );
            iListener.onOperationComplete(
                this.superCalendar, Components.results.NS_OK,
                Components.interfaces.calIOperationListener.GET,
                items.length == 1 ? items[0].id : null, null );
        }
        this.log( items.length.toString() + " items delivered." );
    }
    catch (exc) {
        if (iListener != null) {
            iListener.onOperationComplete(
                this.superCalendar, Components.results.NS_ERROR_FAILURE,
                Components.interfaces.calIOperationListener.GET,
                null, exc );
        }
        this.notifyError( exc );
    }
};

calWcapCalendar.prototype.getItems = function(
    itemFilter, maxResult, rangeStart, rangeEnd, iListener )
{
    // assure DATETIMEs:
    if (rangeStart != null && rangeStart.isDate) {
        rangeStart = rangeStart.clone();
        rangeStart.isDate = false;
    }
    if (rangeEnd != null && rangeEnd.isDate) {
        rangeEnd = rangeEnd.clone();
        rangeEnd.isDate = false;
    }
    var zRangeStart = getIcalUTC(rangeStart);
    var zRangeEnd = getIcalUTC(rangeEnd);
    this.log( "getItems():\n\titemFilter=" + itemFilter +
              ",\n\tmaxResult=" + maxResult +
              ",\n\trangeStart=" + zRangeStart +
              ",\n\trangeEnd=" + zRangeEnd );
    try {
        var url = this.session.getCommandUrl( "fetchcomponents_by_range" );
        url += ("&calid=" + encodeURIComponent(this.calId));
        url += "&compressed=1&recurring=1";
        
        switch (itemFilter &
                Components.interfaces.calICalendar.ITEM_FILTER_TYPE_ALL) {
        case Components.interfaces.calICalendar.ITEM_FILTER_TYPE_TODO:
            url += "&component-type=todo"; break;
        case Components.interfaces.calICalendar.ITEM_FILTER_TYPE_EVENT:
            url += "&component-type=event"; break;
        }
        
        const calIWcapCalendar = Components.interfaces.calIWcapCalendar;
        var compstate = "";
        if (itemFilter & calIWcapCalendar.ITEM_FILTER_REPLY_DECLINED)
            compstate += ";REPLY-DECLINED";
        if (itemFilter & calIWcapCalendar.ITEM_FILTER_REPLY_ACCEPTED)
            compstate += ";REPLY-ACCEPTED";
        if (itemFilter & calIWcapCalendar.ITEM_FILTER_REQUEST_COMPLETED)
            compstate += ";REQUEST-COMPLETED";
        if (itemFilter & calIWcapCalendar.ITEM_FILTER_REQUEST_NEEDS_ACTION)
            compstate += ";REQUEST-NEEDS-ACTION";
        if (itemFilter & calIWcapCalendar.ITEM_FILTER_REQUEST_NEEDSNOACTION)
            compstate += ";REQUEST-NEEDSNOACTION";
        if (itemFilter & calIWcapCalendar.ITEM_FILTER_REQUEST_PENDING)
            compstate += ";REQUEST-PENDING";
        if (itemFilter & calIWcapCalendar.ITEM_FILTER_REQUEST_WAITFORREPLY)
            compstate += ";REQUEST-WAITFORREPLY";
        if (compstate.length > 0)
            url += ("&compstate=" + compstate.substr(1));
        
        if (maxResult > 0)
            url += ("&maxResult=" + maxResult);
        // xxx todo: correctly normalized dates to zulu time?
        url += ("&dtstart=" + zRangeStart);
        url += ("&dtend=" + zRangeEnd);
        
        var this_ = this;
        this.session.issueAsyncRequest(
            url + "&fmt-out=text%2Fcalendar", stringToIcal,
            function( wcapResponse ) {
                this_.getItems_resp( wcapResponse,
                                     itemFilter, maxResult,
                                     rangeStart, rangeEnd, iListener );
            } );
    }
    catch (exc) {
        if (iListener != null) {
            iListener.onOperationComplete(
                this.superCalendar, Components.results.NS_ERROR_FAILURE,
                Components.interfaces.calIOperationListener.GET,
                null, exc );
        }
        if (exc == Components.interfaces.calIWcapErrors.WCAP_LOGIN_FAILED) {
            // silently ignore login failed, no calIObserver UI:
            this.log( "getItems_resp() ignored: " + errorToString(exc) );
        }
        else
            this.notifyError( exc );
    }
    this.log( "getItems() returning." );
};

function SyncState( finishFunc, abortFunc ) {
    this.m_finishFunc = finishFunc;
    this.m_abortFunc = abortFunc;
}
SyncState.prototype = {
    m_state: 0,
    m_finishFunc: null,
    m_abortFunc: null,
    m_exc: null,
    
    acquire: function() { /*this.checkAborted();*/ ++this.m_state; },
    release:
    function()
    {
        /*this.checkAborted();*/
        --this.m_state;
//         logMessage( "sync-state", "m_state = " + this.m_state );
        if (this.m_state == 0) {
            this.m_finishFunc();
        }
    },
    
    checkAborted: function() {
        if (this.m_exc)
            throw this.m_exc;
    },
    get hasAborted() { return this.m_exc != null; },
    abort: function( exc ) {
        if (!this.hasAborted) // store only first error that has occurred
            this.m_exc = exc;
        this.m_abortFunc( exc );
    }
};

function FinishListener( opType, syncState ) {
    this.m_opType = opType;
    this.m_syncState = syncState;
}
FinishListener.prototype = {
    m_opType: 0,
    m_syncState: null,
    
    // calIOperationListener:
    onOperationComplete:
    function( calendar, status, opType, id, detail )
    {
        if (status != Components.results.NS_OK) {
            this.m_syncState.abort( detail );
        }
        else if (this.m_opType != opType) {
            this.m_syncState.abort(
                new Error("unexpected operation type: " + opType) );
        }
        this.m_syncState.release();
    },
    onGetResult:
    function( calendar, status, itemType, detail, count, items )
    {
        this.m_syncState.abort( new Error("unexpected onGetResult()!") );
    }
};

calWcapCalendar.prototype.syncChangesTo_resp = function(
    wcapResponse, syncState, iListener, func )
{
    try {
        var icalRootComp = wcapResponse.data; // first statement, may throw
        var items = this.parseItems(
            icalRootComp,
            Components.interfaces.calICalendar.ITEM_FILTER_ALL_ITEMS,
            0, null, null );
        for each ( var item in items ) {
            // xxx todo: ignore single errors and continue?
            func( item );
        }
    }
    catch (exc) {
        syncState.abort( exc );
    }
    syncState.release();
};

calWcapCalendar.prototype.syncChangesTo = function(
    destCal, dtFrom, iListener )
{
//     this.ensureOnline();
    if (dtFrom != null) {
        // assure DATETIMEs:
        if (dtFrom.isDate) {
            dtFrom = dtFrom.clone();
            dtFrom.isDate = false;
        }
        dtFrom = this.session.getServerTime(dtFrom);
    }
    var zdtFrom = getIcalUTC(dtFrom);
    this.log( "syncChangesTo(): dtFrom=" + zdtFrom );
    try {
        // new stamp for this sync:
        var now = getTime();
        var this_ = this;
        const SYNC = Components.interfaces.calIWcapCalendar.SYNC;
        
        var syncState = new SyncState(
            // finishFunc:
            function() {
                if (iListener != null) {
                    if (syncState.hasAborted) {
                        this_.log( "firing SYNC failed!" );
                        iListener.onOperationComplete(
                            this_.superCalendar,
                            Components.results.NS_ERROR_FAILURE,
                            SYNC, null, syncState.m_exc );
                    }
                    else {
                        this_.log( "firing SYNC succeeded." );
                        iListener.onOperationComplete(
                            this_.superCalendar, Components.results.NS_OK,
                            SYNC, null, now );
                    }
                }
            },
            // abortFunc:
            function( exc ) {
//                 if (iListener != null) {
//                     iListener.onOperationComplete(
//                         this_.superCalendar,
//                         Components.results.NS_ERROR_FAILURE,
//                         SYNC, null, exc );
//                 }
                this_.logError( exc );
            } );
        
        var addItemListener = new FinishListener(
            Components.interfaces.calIOperationListener.ADD, syncState );
        if (dtFrom == null) {
            this.log( "syncChangesTo(): doing initial sync." );
            syncState.acquire();
            var url = this.session.getCommandUrl( "fetchcomponents_by_range" );
            url += ("&compressed=1&recurring=1&calid=" +
                    encodeURIComponent(this.calId));
            this.session.issueAsyncRequest(
                url + "&fmt-out=text%2Fcalendar", stringToIcal,
                function( wcapResponse ) {
                    this_.syncChangesTo_resp(
                        wcapResponse, syncState, iListener,
                        function( item ) {
                            syncState.acquire();
                            this_.log( "adding " + item.id );
                            // xxx todo: verify whether exceptions
                            // are written:
                            destCal.addItem( item, addItemListener );
                        } );
                } );
        }
        else {
            var modifiedItems = [];
            this.log( "syncChangesTo(): getting last modifications." );
            var modifyItemListener = new FinishListener(
                Components.interfaces.calIOperationListener.MODIFY, syncState );
            var params = ("&compressed=1&recurring=1&calid=" +
                          encodeURIComponent(this.calId));
            params += ("&fmt-out=text%2Fcalendar&dtstart=" + zdtFrom);
            syncState.acquire();
            this.session.issueAsyncRequest(
                this.session.getCommandUrl("fetchcomponents_by_lastmod")+params,
                stringToIcal,
                function( wcapResponse ) {
                    this_.syncChangesTo_resp(
                        wcapResponse, syncState, iListener,
                        function( item ) {
                            var dtCreated = item.getProperty("CREATED");
                            var bAdd = (dtCreated == null ||
                                        dtCreated.compare(dtFrom) >= 0);
                            syncState.acquire();
                            modifiedItems.push( item.id );
                            if (bAdd) {
                                // xxx todo: verify whether exceptions
                                // are written:
                                this_.log( "adding " + item.id );
                                destCal.addItem( item, addItemListener );
                            }
                            else {
                                this_.log( "modifying " + item.id );
                                destCal.modifyItem( item, null,
                                                    modifyItemListener );
                            }
                        } );
                } );
            this.log( "syncChangesTo(): getting deleted items." );
            var deleteItemListener = new FinishListener(
                Components.interfaces.calIOperationListener.DELETE, syncState );
            syncState.acquire();
            this.session.issueAsyncRequest(
                this.session.getCommandUrl("fetch_deletedcomponents") + params,
                stringToIcal,
                function( wcapResponse ) {
                    this_.syncChangesTo_resp(
                        wcapResponse, syncState, iListener,
                        function( item ) {
                            // don't delete anything that has been touched
                            // by lastmods:
                            for each ( var mid in modifiedItems ) {
                                if (item.id == mid) {
                                    this_.log(
                                        "skipping deletion of " + item.id );
                                    return;
                                }
                            }
                            syncState.acquire();
                            if (item.parentItem == item) {
                                this_.log( "deleting " + item.id );
                                destCal.deleteItem( item, deleteItemListener );
                            }
                            else {
                                // modify parent instead of
                                // straight-forward deleteItem(). WTF.
                                var parent = item.parentItem.clone();
                                parent.recurrenceInfo.removeOccurrenceAt(
                                    item.recurrenceId );
                                this_.log( "modifying " + parent.id );
                                destCal.modifyItem( parent, item,
                                                    deleteItemListener );
                            }
                        } );
                } );
        }
    }
    catch (exc) {
        if (iListener != null) {
            iListener.onOperationComplete(
                this.superCalendar, Components.results.NS_ERROR_FAILURE,
                Components.interfaces.calIWcapCalendar.SYNC,
                null, exc );
        }
        this.notifyError( exc );
    }
    this.log( "syncChangesTo() returning." );
};

