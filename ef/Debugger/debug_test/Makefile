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

# Makefile for the debugger

DIRS = ../Communication
DEPTH = ../..

CXXSRCS = \
	debug_test.cpp \
	$(NULL)

PROGRAM_NAME = debug_test

include $(DEPTH)/config/rules.mk

INCLUDES += \
		-I .. \
		-I $(DEPTH)/Exports \
		-I $(DEPTH)/Exports/md \
		-I $(DEPTH)/Utilities/General \
		-I $(DEPTH)/Debugger/Communication 

ifeq ($(OS_ARCH),Linux)
LDFLAGS += -L $(DIST)/lib -L$(DEPTH)/Utilities/zlib/$(OBJDIR)
EXTRA_LIBS = -lg++ -lnspr21 -lplc21 -lEF -lzlib
endif

ifeq ($(OS_ARCH),WINNT)
CXXFLAGS += /Fr 
EXTRA_LIBS += $(DIST)/lib/libDebuggerChannel.lib $(DIST)/lib/libnspr21.lib $(DIST)/lib/libplc21.lib  wsock32.lib
endif
