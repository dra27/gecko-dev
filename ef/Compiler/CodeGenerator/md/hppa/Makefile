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

DEPTH			= ../../..

MODULE_NAME		= hppa

include $(DEPTH)/config/config.mk

INCLUDES		+=										\
	-I$(DEPTH)/Utilities/General						\
	-I$(DEPTH)/Utilities/zlib							\
	-I$(DEPTH)/Runtime/ClassReader						\
	-I$(DEPTH)/Compiler/FrontEnd						\
	-I$(DEPTH)/Runtime/NativeMethods					\
	-I$(DEPTH)/Runtime/System							\
	-I$(DEPTH)/Compiler/PrimitiveGraph					\
	-I$(DEPTH)/Runtime/ClassInfo						\
	-I$(DEPTH)/Runtime/FileReader						\
	-I$(DEPTH)/Compiler/CodeGenerator					\
	-I$(DEPTH)/Compiler/CodeGenerator/md				\
	-I$(DEPTH)/Compiler/RegisterAllocator				\
	$(NULL)

CXXSRCS			= 										\
	HPPAEmitter.cpp										\
	HPPAInstruction.cpp									\
	HPPAMul.cpp											\
	hppa.nad.burg.cpp									\
	$(NULL)

include $(DEPTH)/config/rules.mk

export:: hppa.nad.burg.cpp

libs:: $(MODULE)

#
# Rules to generate hppa.nad.burg.[cpp][h]
#

hppa.nad.burg.cpp: hppa.nad.burg $(BURG)
	$(BURG) -I -o $@ < $<

hppa.nad.burg: hppa.nad $(DEPTH)/Compiler/PrimitiveGraph/PrimitiveOperations $(DEPTH)/Tools/Nad/nad.pl
	$(PERL) $(DEPTH)/Tools/Nad/nad.pl $< $(DEPTH)/Compiler/PrimitiveGraph/PrimitiveOperations	\
		$(DEPTH)/Compiler/PrimitiveGraph/PrimitiveOperations.h									\
		$(DEPTH)/Compiler/PrimitiveGraph/PrimitiveOperations.cpp								\
		$<.burg.h > $@

#
# Extra cleaning
#

clobber::
	rm -f hppa.nad.burg.cpp hppa.nad.burg.h hppa.nad.burg

realclean clobber_all::
	rm -f hppa.nad.burg.cpp hppa.nad.burg.h hppa.nad.burg
