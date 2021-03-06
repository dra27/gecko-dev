/* -*- Mode: javascript; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * The Original Code is mozilla calendar code.
 *
 * The Initial Developer of the Original Code is
 *  Michiel van Leeuwen <mvl@exedo.nl>
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Vladimir Vukicevic <vladimir.vukicevic@oracle.com>
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

//
// calICSCalendar.js
//
// This is a non-sync ics file. It reads the file pointer to by uri when set,
// then writes it on updates. External changes to the file will be
// ignored and overwritten.
//
// XXX Should do locks, so that external changes are not overwritten.

const calIOperationListener = Components.interfaces.calIOperationListener;
const calICalendar = Components.interfaces.calICalendar;
const calCalendarManagerContractID = "@mozilla.org/calendar/manager;1";
const calICalendarManager = Components.interfaces.calICalendarManager;

var activeCalendarManager = null;
function getCalendarManager()
{
    if (!activeCalendarManager) {
        activeCalendarManager = 
            Components.classes[calCalendarManagerContractID].getService(calICalendarManager);
    }
    return activeCalendarManager;
}


function calICSCalendar () {
    this.wrappedJSObject = this;
    this.initICSCalendar();

    this.unmappedComponents = [];
    this.unmappedProperties = [];
    this.queue = new Array();
}

calICSCalendar.prototype = {
    mICSService: null,
    mObserver: null,
    locked: false,

    QueryInterface: function (aIID) {
        if (!aIID.equals(Components.interfaces.nsISupports) &&
            !aIID.equals(Components.interfaces.calICalendar) &&
            !aIID.equals(Components.interfaces.nsIStreamListener) &&
            !aIID.equals(Components.interfaces.nsIStreamLoaderObserver) &&
            !aIID.equals(Components.interfaces.nsIInterfaceRequestor)) {
            throw Components.results.NS_ERROR_NO_INTERFACE;
        }

        return this;
    },
    
    initICSCalendar: function() {
        this.mMemoryCalendar = Components.classes["@mozilla.org/calendar/calendar;1?type=memory"]
                                         .createInstance(Components.interfaces.calICalendar);
        this.mICSService = Components.classes["@mozilla.org/calendar/ics-service;1"]
                                     .getService(Components.interfaces.calIICSService);

        this.mObserver = new calICSObserver(this);
        this.mMemoryCalendar.addObserver(this.mObserver);
        this.mMemoryCalendar.wrappedJSObject.calendarToReturn = this;
    },

    get name() {
        return getCalendarManager().getCalendarPref(this, "NAME");
    },
    set name(name) {
        getCalendarManager().setCalendarPref(this, "NAME", name);
    },

    get type() { return "ics"; },

    mUri: null,
    get uri() { return this.mUri },
    set uri(aUri) {
        this.mMemoryCalendar.uri = this.mUri;
        this.mUri = aUri;
        this.refresh();
    },

    refresh: function() {
        // Lock other changes to the item list.
        this.locked = true;
        // set to prevent writing after loading, without any changes
        this.loading = true;

        var ioService = Components.classes["@mozilla.org/network/io-service;1"]
                                  .getService(Components.interfaces.nsIIOService);

        var channel = ioService.newChannelFromURI(fixupUri(this.mUri));
        channel.loadFlags |= Components.interfaces.nsIRequest.LOAD_BYPASS_CACHE;
        channel.notificationCallbacks = this;

        var streamLoader = Components.classes["@mozilla.org/network/stream-loader;1"]
                                     .createInstance(Components.interfaces.nsIStreamLoader);
        streamLoader.init(channel, this, this);
    },

    calendarPromotedProps: {
        "PRODID": true,
        "VERSION": true
    },

    // nsIStreamLoaderObserver impl
    // Listener for download. Parse the downloaded file

    // XXX use the onError observer calls
    onStreamComplete: function(loader, ctxt, status, resultLength, result)
    {
        // Create a new calendar, to get rid of all the old events
        this.mMemoryCalendar = Components.classes["@mozilla.org/calendar/calendar;1?type=memory"]
                                         .createInstance(Components.interfaces.calICalendar);
        this.mMemoryCalendar.uri = this.mUri;
        this.mMemoryCalendar.wrappedJSObject.calendarToReturn = this;
        this.mMemoryCalendar.addObserver(this.mObserver);

        this.mObserver.onStartBatch();

        var unicodeConverter = Components.classes["@mozilla.org/intl/scriptableunicodeconverter"]
                                         .createInstance(Components.interfaces.nsIScriptableUnicodeConverter);
        // ics files are always utf8
        unicodeConverter.charset = "UTF-8";
        var str = unicodeConverter.convertFromByteArray(result, result.length);

        // Wrap parsing in a try block. Will ignore errors. That's a good thing
        // for non-existing or empty files, but not good for invalid files.
        // We should not accidently overwrite third party files.
        // XXX Fix that
        try {
            var calComp = this.mICSService.parseICS(str);

            // Get unknown properties
            var prop = calComp.getFirstProperty("ANY");
            while (prop) {
                if (!this.calendarPromotedProps[prop.propertyName]) {
                    this.unmappedProperties.push(prop);
                    dump(prop.propertyName+"\n");
                }
                prop = calComp.getNextProperty("ANY");
            }

            var subComp = calComp.getFirstSubcomponent("ANY");
            while (subComp) {
                switch (subComp.componentType) {
                case "VEVENT":
                    var event = Components.classes["@mozilla.org/calendar/event;1"]
                                          .createInstance(Components.interfaces.calIEvent);
                    event.icalComponent = subComp;
                    this.mMemoryCalendar.addItem(event, null);
                    break;
                case "VTODO":
                    // XXX Last time i tried, vtodo didn't work. Sadly no time to
                    // debug now.
                    var todo = Components.classes["@mozilla.org/calendar/todo;1"]
                                         .createInstance(Components.interfaces.calITodo);
                    todo.icalComponent = subComp;
                    this.mMemoryCalendar.addItem(todo, null);
                    break;
                default:
                    this.unmappedComponents.push(subComp);
                    dump(subComp.componentType+"\n");
                }
                subComp = calComp.getNextSubcomponent("ANY");
            }
            
        } catch(e) { dump(e+"\n");}

        this.mObserver.onEndBatch();
        this.mObserver.onLoad();
        this.locked = false;
        this.processQueue();
    },

    writeICS: function () {
        // Don't save right after loading. There are no changes anyway.
        if (this.loading) {
            this.loading = false;
            return;
        }

        this.locked = true;

        if (!this.mUri)
            throw Components.results.NS_ERROR_FAILURE;

        var savedthis = this;
        var listener =
        {
            onOperationComplete: function(aCalendar, aStatus, aOperationType, aId, aDetail)
            {
                // All events are returned. Now set up a channel and a
                // streamloader to upload.
                // Will call onStopRequest when finised.
                var icsStr = calComp.serializeToICS();

                var ioService = Components.classes["@mozilla.org/network/io-service;1"]
                                          .getService(Components.interfaces.nsIIOService);
                var channel = ioService.newChannelFromURI(fixupUri(savedthis.mUri));
                channel.notificationCallbacks = savedthis;
                var uploadChannel = channel.QueryInterface(Components.interfaces.nsIUploadChannel);
                var postStream = Components.classes["@mozilla.org/io/string-input-stream;1"]
                                           .createInstance(Components.interfaces.nsIStringInputStream);
                postStream.setData(icsStr, icsStr.length);
                uploadChannel.setUploadStream(postStream, "text/calendar", -1);
                channel.asyncOpen(savedthis, savedthis);
            },
            onGetResult: function(aCalendar, aStatus, aItemType, aDetail, aCount, aItems)
            {
                var lastmodifed;
                for (var i=0; i<aCount; i++) {
                    calComp.addSubcomponent(aItems[i].icalComponent);
                }
            }
        };

        var calComp = this.mICSService.createIcalComponent("VCALENDAR");
        calComp.version = "2.0";
        calComp.prodid = "-//Mozilla.org/NONSGML Mozilla Calendar V1.0//EN";

        var i;
        for (i in this.unmappedComponents) {
             dump("Adding a "+this.unmappedComponents[i].componentType+"\n");
             calComp.addSubcomponent(this.unmappedComponents[i]);
        }
        for (i in this.unmappedProperties) {
             dump("Adding "+this.unmappedProperties[i].propertyName+"\n");
             calComp.addProperty(this.unmappedProperties[i]);
        }

        this.getItems(calICalendar.ITEM_FILTER_TYPE_ALL | calICalendar.ITEM_FILTER_COMPLETED_ALL,
                      0, null, null, listener);
    },

    // nsIStreamListener impl
    // For after publishing. Do error checks here
    onStartRequest: function(request, ctxt) {},
    onDataAvailable: function(request, ctxt, inStream, sourceOffset, count) {},
    onStopRequest: function(request, ctxt, status, errorMsg)
    {
        ctxt = ctxt.wrappedJSObject;
        var channel;
        try {
            channel = request.QueryInterface(Components.interfaces.nsIHttpChannel);
            dump(channel.requestSucceeded+"\n");
        } catch(e) {
        }

        if (channel && !channel.requestSucceeded) {
            ctxt.mObserver.onError(null,
                                   "Publishing the calendar file failed\n" +
                                       "Status code: "+channel.responseStatus+": "+channel.responseStatusText+"\n");
        }

        else if (!channel && !Components.isSuccessCode(request.status)) {
            ctxt.mObserver.onError(null,
                                   "Publishing the calendar file failed\n" +
                                       "Status code: "+request.status.toString(16)+"\n");
        }
        ctxt.locked = false;
        ctxt.processQueue();
    },

    addObserver: function (aObserver) {
        this.mObserver.addObserver(aObserver);
    },
    removeObserver: function (aObserver) {
        this.mObserver.removeObserver(aObserver);
    },

    // Always use the queue, just to reduce the amount of places where
    // this.mMemoryCalendar.addItem() and friends are called. less
    // copied code.
    addItem: function (aItem, aListener) {
        this.queue.push({action:'add', item:aItem, listener:aListener});
        this.processQueue();
    },

    modifyItem: function (aNewItem, aOldItem, aListener) {
        this.queue.push({action:'modify', oldItem: aOldItem,
                         newItem: aNewItem, listener:aListener});
        this.processQueue();
    },

    deleteItem: function (aItem, aListener) {
        this.queue.push({action:'delete', item:aItem, listener:aListener});
        this.processQueue();
    },

    getItem: function (aId, aListener) {
        return this.mMemoryCalendar.getItem(aId, aListener);
    },

    getItems: function (aItemFilter, aCount,
                        aRangeStart, aRangeEnd, aListener)
    {
        return this.mMemoryCalendar.getItems(aItemFilter, aCount,
                                             aRangeStart, aRangeEnd,
                                             aListener);
    },

    processQueue: function ()
    {
        if (this.locked)
            return;
        var a;
        while ((a = this.queue.shift())) {
            switch (a.action) {
                case 'add':
                    this.mMemoryCalendar.addItem(a.item, a.listener);
                    break;
                case 'modify':
                    this.mMemoryCalendar.modifyItem(a.newItem, a.oldItem,
                                                    a.listener);
                    break;
                case 'delete':
                    this.mMemoryCalendar.deleteItem(a.item, a.listener);
                    break;
            }
        }
    },

    // nsIInterfaceRequestor impl
    getInterface: function(iid, instance) {
        if (iid.equals(Components.interfaces.nsIAuthPrompt)) {
            // use the window watcher service to get a nsIAuthPrompt impl
            return Components.classes["@mozilla.org/embedcomp/window-watcher;1"]
                             .getService(Components.interfaces.nsIWindowWatcher)
                             .getNewAuthPrompter(null);
        }
        else if (iid.equals(Components.interfaces.nsIPrompt)) {
            // use the window watcher service to get a nsIPrompt impl
            return Components.classes["@mozilla.org/embedcomp/window-watcher;1"]
                             .getService(Components.interfaces.nsIWindowWatcher)
                             .getNewPrompter(null);
        }
        Components.returnCode = Components.results.NS_ERROR_NO_INTERFACE;
        return null;
    }
};

function fixupUri(aUri) {
    var uri = aUri;
    if (uri.scheme == 'webcal')
        uri.scheme = 'http';
    if (uri.scheme == 'webcals')
        uri.scheme = 'https';
    return uri;
}

function calICSObserver(aCalendar) {
    this.mCalendar = aCalendar;
    this.mObservers = new Array();
}

calICSObserver.prototype = {
    mCalendar: null,
    mInBatch: false,

    onStartBatch: function() {
        for (var i = 0; i < this.mObservers.length; i++)
            this.mObservers[i].onStartBatch();
        this.mInBatch = true;
    },
    onEndBatch: function() {
        for (var i = 0; i < this.mObservers.length; i++)
            this.mObservers[i].onEndBatch();

        this.mInBatch = false;
        this.mCalendar.writeICS();
    },
    onLoad: function() {
        for (var i = 0; i < this.mObservers.length; i++)
            this.mObservers[i].onLoad();
    },
    onAddItem: function(aItem) {
        for (var i = 0; i < this.mObservers.length; i++)
            this.mObservers[i].onAddItem(aItem);

        if (!this.mInBatch)
            this.mCalendar.writeICS();
    },
    onModifyItem: function(aNewItem, aOldItem) {
        for (var i = 0; i < this.mObservers.length; i++)
            this.mObservers[i].onModifyItem(aNewItem, aOldItem);

        if (!this.mInBatch)
            this.mCalendar.writeICS();
    },
    onDeleteItem: function(aDeletedItem) {
        for (var i = 0; i < this.mObservers.length; i++)
            this.mObservers[i].onDeleteItem(aDeletedItem);

        if (!this.mInBatch)
            this.mCalendar.writeICS();
    },
    onAlarm: function(aAlarmItem) {
        for (var i = 0; i < this.mObservers.length; i++)
            this.mObservers[i].onAlarm(aAlarmItem);
    },
    onError: function(aMessage) {
        for (var i = 0; i < this.mObservers.length; i++)
            this.mObservers[i].onError(aMessage);
    },

    // This observer functions as proxy for all the other observers
    // So need addObserver and removeObserver here
    addObserver: function (aObserver) {
        for each (obs in this.mObservers) {
            if (obs == aObserver)
                return;
        }

        this.mObservers.push(aObserver);
    },

    removeObserver: function (aObserver) {
        var newObservers = Array();
        for each (obs in this.mObservers) {
            if (obs != aObserver)
                newObservers.push(obs);
        }
        this.mObservers = newObservers;
    }
};

/****
 **** module registration
 ****/

var calICSCalendarModule = {

    mCID: Components.ID("{f8438bff-a3c9-4ed5-b23f-2663b5469abf}"),
    mContractID: "@mozilla.org/calendar/calendar;1?type=ics",
    
    registerSelf: function (compMgr, fileSpec, location, type) {
        compMgr = compMgr.QueryInterface(Components.interfaces.nsIComponentRegistrar);
        compMgr.registerFactoryLocation(this.mCID,
                                        "Calendar ICS provider",
                                        this.mContractID,
                                        fileSpec,
                                        location,
                                        type);
    },

    getClassObject: function (compMgr, cid, iid) {
        if (!cid.equals(this.mCID))
            throw Components.results.NS_ERROR_NO_INTERFACE;

        if (!iid.equals(Components.interfaces.nsIFactory))
            throw Components.results.NS_ERROR_NOT_IMPLEMENTED;

        return this.mFactory;
    },

    mFactory: {
        createInstance: function (outer, iid) {
            if (outer != null)
                throw Components.results.NS_ERROR_NO_AGGREGATION;
            return (new calICSCalendar()).QueryInterface(iid);
        }
    },

    canUnload: function(compMgr) {
        return true;
    }
};

function NSGetModule(compMgr, fileSpec) {
    return calICSCalendarModule;
}
