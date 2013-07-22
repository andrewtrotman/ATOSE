#
# Makefile for ATOSE host tools.
# Copyright (c) 2013 Andrew Trotman
#

EXT = mac

BIN_DIR = ../bin/

all : $(BIN_DIR)bin_to_c.$(EXT) \
	$(BIN_DIR)elf_reader.$(EXT) \
	$(BIN_DIR)imx_run.$(EXT) \
	$(BIN_DIR)imximage_render.$(EXT)

$(BIN_DIR)%.$(EXT) : %.c
	@echo $@
	$(CXX) $(CXXFLAGS) -o $@ $< -framework CoreFoundation -framework IOKit

