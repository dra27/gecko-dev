# The contents of this file are subject to the Mozilla Public
# License Version 1.1 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
#
# The Original Code is mozilla.org code.
#
# The Initial Developer of the Original Code is Sun Microsystems,
# Inc. Portions created by Sun are
# Copyright (C) 1999 Sun Microsystems, Inc. All
# Rights Reserved.
#
# Contributor(s):


DEPTH=../../../../../../..
UTIL_DIR=$(DEPTH)/utils
BUILD_DIR=$(DEPTH)/build
CONFIG_FILE=$(DEPTH)/config/CommonProperties
LST_FILE=test/basic/api/BWTest.lst.ORIG
BUILD_DIR_CLASSES=$(BUILD_DIR)/classes
BASIC_PATH=org/mozilla/pluglet/test/basic
TEST_DIRS=test/basic/api
CLASSES_LOCATION=$(BUILD_DIR_CLASSES)/$(BASIC_PATH)/api
API_HTML_ROOT_DIR=$(HTML_ROOT_DIR)/api
JSRC= *.java
JAVAC=$(JDKHOME)/bin/javac
JAVA=$(JDKHOME)/bin/java
CLASSPATH=$(MOZILLA_FIVE_HOME)/../classes:$(BUILD_DIR_CLASSES)

#All tests list
#PlugletManager_getValue was excluded from test list, method removed
#PlugletStreamInfo_requestRead commented,method doesn't work
TESTS = PlugletPeer_getMIMEType \
	  PlugletPeer_getMode \
	  PlugletPeer_getValue \
	  PlugletPeer_setWindowSize \
	  PlugletPeer_showStatus \
	  PlugletPeer_getTagInfo \
	  PlugletManager_reloadPluglets \
	  PlugletManager_userAgent \
	  PlugletStreamInfo_getContentType \
 	  PlugletStreamInfo_getLastModified \
	  PlugletStreamInfo_getLength \
	  PlugletStreamInfo_getURL \
	  PlugletStreamInfo_isSeekable \
	  PlugletStreamInfo_requestRead \
	  PlugletTagInfo2_getAlignment_left \
	  PlugletTagInfo2_getAlignment_right \
	  PlugletTagInfo2_getAlignment_top \
	  PlugletTagInfo2_getAlignment_bottom \
	  PlugletTagInfo2_getAlignment_middle \
	  PlugletTagInfo_getAttribute \
  	  PlugletTagInfo_getAttributes \
	  PlugletTagInfo2_getParameter \
	  PlugletTagInfo2_getParameters \
	  PlugletTagInfo2_getParameters_1 \
	  PlugletTagInfo2_getAttribute \
	  PlugletTagInfo2_getDOMElement \
  	  PlugletTagInfo2_getAttributes \
	  PlugletTagInfo2_getBorderHorizSpace \
	  PlugletTagInfo2_getBorderHorizSpace_1 \
	  PlugletTagInfo2_getBorderVertSpace \
	  PlugletTagInfo2_getBorderVertSpace_1 \
	  PlugletTagInfo2_getDocumentBase \
	  PlugletTagInfo2_getDocumentEncoding \
	  PlugletTagInfo2_getHeight \
	  PlugletTagInfo2_getHeight_1 \
	  PlugletTagInfo2_getTagText \
	  PlugletTagInfo2_getTagType \
	  PlugletTagInfo2_getTagType_1 \
	  PlugletTagInfo2_getUniqueID \
	  PlugletTagInfo2_getWidth \
	  PlugletTagInfo2_getWidth_1 \
	  PlugletManager_getURL \
	  PlugletManager_getURL_1 \
	  PlugletManager_postURL \
	  PlugletPeer_newStream_0 \
	  PlugletPeer_newStream_1 

default: classes tests test_pluglet

classes:
	@echo Compile classes ..
	@$(JAVAC) -classpath $(CLASSPATH) -d $(BUILD_DIR_CLASSES) Pluglet*.java

test_pluglet:
	@mkdir -p $(BUILD_DIR_CLASSES)/$(BASIC_PATH)/api
	@cp -f manifest $(BUILD_DIR_CLASSES)/$(BASIC_PATH)/api
	@cd $(BUILD_DIR_CLASSES); \
	$(JDKHOME)/bin/jar cfm $(BASIC_PATH)/api/TestAPIPluglet.jar $(BASIC_PATH)/api/manifest $(BASIC_PATH)/*.class $(BASIC_PATH)/api/*.class

tests:
	@$(MAKE) tests_rec MAKE_ARGS=all

combinations:
	@echo Make combinations ..
	@echo -- NOTE: You need to do combinations only if you change
	@echo -- ParamCombination files in test dirs. 
	@echo -- For rest cases defult LST files exists in test dirs
	@mkdir -p $(BUILD_DIR)/$(TEST_DIRS)
	@$(MAKE) tests_rec MAKE_ARGS=$@ COMMON_LST_FILE=$(LST_FILE)

clobber:
	@if [ -e $(CLASSES_LOCATION) ]; then \
		rm -rf $(CLASSES_LOCATION)/* ; \
	fi
	@$(MAKE) tests_rec MAKE_ARGS=$@

tests_rec:
	@for i in $(TESTS);do \
		echo Making $(MAKE_ARGS) in $${i}; \
		cd $${i}; \
		$(MAKE)  $(MAKE_ARGS) HTML_DEST_DIR=$(API_HTML_ROOT_DIR); \
		cd ..; \
	done













