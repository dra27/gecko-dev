#!gmake
#
# The contents of this file are subject to the Netscape Public
# License Version 1.1 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at http://www.mozilla.org/NPL/
#
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
#
# The Original Code is mozilla.org code.
#
# The Initial Developer of the Original Code is Netscape
# Communications Corporation.  Portions created by Netscape are
# Copyright (C) 1998 Netscape Communications Corporation. All
# Rights Reserved.
#
# Contributor(s):
# Serge Pikalev <sep@sparc.spb.su>
#

DEPTH           = ../../../..
topsrcdir       = ../../../..
srcdir          = .
VPATH           = .



include $(DEPTH)/config/autoconf.mk
include $(topsrcdir)/config/config.mk

XPIDL_MODULE	= bcPluginWrappers
XPIDLSRCS	=  \
	nsIXPIDLPlugin.idl \
	nsIXPIDLPluginInstance.idl \
	nsIXPIDLPluginStreamListener.idl \
	nsIXPIDLPluginManager.idl \
	nsIXPIDLPluginManager2.idl \
	nsIXPIDLPluginInstancePeer.idl \
	nsIXPIDLPluginTagInfo.idl \
	nsIXPIDLPluginTagInfo2.idl \
	nsIXPIDLPluginStreamInfo.idl \
	nsIXPIDLInputStream.idl \
	nsIXPIDLOutputStream.idl \
	$(NULL)

DSO_LDOPTS            += \
	$(XPCOM_LIBS) \
	$(NSPR_LIBS) \
	$(NULL)

include $(topsrcdir)/config/rules.mk
