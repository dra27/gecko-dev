# -*- Mode: makefile -*-
#
# Hacked together by Adam Rogers [adam (at) jargon (dot) ca] from
# Linux_All.mk and WINNT5.1.mk to enable building using GNU tools
# under MinGW on a Win32 platform. [September 6 2008]
#


#
# Config for all versions of Linux
#

TOOLPREF =
OS_WINVER = 0x500

include $(DEPTH)/config/WINNT-MinGW.mk
