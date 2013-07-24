#
# Windows entry point for the make of ATOSE and tools
#
# Copyright (c) 2013 Andrew Trotman
#

#
#	Set up operating specific tools
#

DEL = rm

#
#	Go ahead and start the build process
#

all :
	make -f makefile.os $(OS) $(ARCH)
	
clean :
	make -f makefile.os clean
