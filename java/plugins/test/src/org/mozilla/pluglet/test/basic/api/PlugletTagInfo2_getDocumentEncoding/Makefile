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


TEST_NAME=PlugletTagInfo2_getDocumentEncoding
DEPTH=../../../../../../../..
UTIL_DIR=$(DEPTH)/utils
BUILD_DIR=$(DEPTH)/build
CONFIG_FILE=$(DEPTH)/config/CommonProperties
BASIC_PATH=test/basic/api
DEST_DIR=$(BUILD_DIR)/$(BASIC_PATH)/$(TEST_NAME)
MAKE_PARAM=perl ../makeParameters.pl
MAKE_COMB=perl ../makeCombinations.pl

default: all
all: html properties
html:
	@mkdir -p $(HTML_DEST_DIR)/$(TEST_NAME)
	@cp -f *.html $(HTML_DEST_DIR)/$(TEST_NAME) 

properties:
	@mkdir -p $(DEST_DIR)
	@perl $(UTIL_DIR)/configurator.pl $(CONFIG_FILE) $(DEST_DIR)/TestProperties TestProperties
combinations:
	echo basic/api/$(TEST_NAME)>>$(BUILD_DIR)/$(COMMON_LST_FILE)
clobber:
	@rm -f $(HTML_DEST_DIR)/$(TEST_NAME)/*.html
	@rm -f $(DEST_DIR)/TestProperties
