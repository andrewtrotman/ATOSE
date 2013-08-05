#
# Linux / MacOS specific makefile for ATOSE
#
# Copyright (c) 2013 Andrew Trotman
#

#
#	Get the current OS
#
UNAME := $(shell uname)

#
#	Set up operating specific tools.
#
DEL = rm

#
# Host specific stuff
#

EXT = mac

#
# Include the generic stuff
#

include makefile.mak

#
# Host specific implic rules
#

$(BIN_DIR)/%.elf : $(TESTS_DIR)/%.c
	@echo $@
	$(CC) $(CCFLAGS) -o $@ $(TESTS_DIR)/imx6q.s $< $(CLINKFLAGS) -T $(TESTS_DIR)/imx6q.ld

$(BIN_DIR)%.$(EXT) : $(TOOLS_DIR)/%.c
        @echo $@
        $(CXX) $(CXXFLAGS) -x c++ -Wno-dangling-else -o $@ $< -framework CoreFoundation -framework IOKit
#
# Host specific build rules
#

clean:
	$(DEL) $(CLEANABLE)

