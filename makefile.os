#
# We arrive here once we know what operating system we are running on
# here we do specal processing for each and then chain to the ATOSE
# makefile to build everything
#
# Copyright (c) 2013 Andrew Trotman
#

default:
	echo "You are trying to build ATOSE on an operating system the build system does know understand."

Darwin:
	export EXT=mac; make --no-print-directory -f makefile.atose all

Windows_NT:
	set EXT=exe
	make -f makefile.atose all

Linux linux:
	export EXT=elf; make --no-print-directory -f makefile.atose all

#
#  a generic rule for building clean
#

clean :
	make -f makefile.atose clean