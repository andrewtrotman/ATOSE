#
# Makefile for ATOSE host tools.
# Copyright (c) 2013 Andrew Trotman
#

EXT = mac

BIN_DIR = ../bin/

EXE = $(BIN_DIR)bin_to_c.$(EXT) \
	$(BIN_DIR)elf_reader.$(EXT) \
	$(BIN_DIR)imx_run.$(EXT) \
	$(BIN_DIR)imximage_render.$(EXT)

all : $(EXE)

$(BIN_DIR)%.$(EXT) : %.c
	@echo $@
	@$(CXX) $(CXXFLAGS) -x c++ -Wno-dangling-else -o $@ $< -framework CoreFoundation -framework IOKit

clean :
	rm $(EXE)
