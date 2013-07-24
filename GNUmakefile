#
# Linux / MacOS entry point for the make of ATOSE and tools
#
# Copyright (c) 2013 Andrew Trotman
#

#
#	Get the current OS
#
UNAME := $(shell uname)

#
#	Set up operating specific tools.  Because they are exported they are passed
# 	along to any instances of make called from here.
#

export DEL=rm

#
#	Go ahead and 4tart the build process
#

all :
	make -f makefile.os $(UNAME)

clean :
	make -f makefile.os clean