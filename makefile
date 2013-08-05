#
# Windows verison of the makefile for building ATOSE.
#
# Copyright (c) 2013 Andrew Trotman
#

#
#	Set up operating specific tools
#

DEL = del
CXX = $(CXX) -nologo
#
# Host specific stuff
#

EXT = exe

#
#	Which version of the Window DDK do we have?
#
VER_8 = "C:\Program Files (x86)\Windows Kits\8.0\"
!IF "$(WINDOWSSDKDIR)" == $(VER_8)
DDK_DIR = $(WINDOWSSDKDIR)\Include
DDK_INCLUDE = "$(DDK_DIR)\shared" -I"$(DDK_DIR)\km" -I"$(DDK_DIR)\km\crt" -I"$(DDK_DIR)\um"
DDK_LIB = "$(WINDOWSSDKDIR)\Lib\win8\um\x64\setupapi.lib" "$(WINDOWSSDKDIR)\Lib\win8\um\x64\hid.lib"
!ELSE
DDK_DIR = "c:\WinDDK\7600.16385.1\inc"
DDK_INCLUDE = $(DDK_DIR)\ddk -I$(DDK_DIR)\api -I$(DDK_DIR)\crt
DDK_LIB = c:\WinDDK\7600.16385.1\lib\wxp\i386\setupapi.lib c:\WinDDK\7600.16385.1\lib\wxp\i386\hid.lib
!ENDIF

#
# Include the generic stuff
#

!include makefile.mak

#
# Host specific implicit rules
#
{$(TESTS_DIR)}.c{$(BIN_DIR)}.elf:
	@echo $@
	@$(CC) $(CCFLAGS) -o $@ $(TESTS_DIR)/imx6q.s $< $(CLINKFLAGS) -T $(TESTS_DIR)/imx6q.ld

{$(TOOLS_DIR)}.c{$(BIN_DIR)}.$(EXT):
	@$(CXX) $(CXXFLAGS) -Fe$@ /Tp $< -Fo$(OBJ_DIR)\$(@B).obj

{$(SOURCE_DIR)}.c{$(OBJ_DIR)}.o:
	@echo $@
	@$(CCXX) $(CCXXFLAGS) -c $< -o $@

{$(SOURCE_DIR)}.asm{$(OBJ_DIR)}.o:
	@echo $@
	@$(AS) $(ASFLAGS) $< -o $@

#
# Host specific explicit rules
#

$(BIN_DIR)\imx_run.$(EXT) : $(TOOLS_DIR)\imx_run.c
	@$(CXX) $(CXXFLAGS) -Fe$@ /Tp $(TOOLS_DIR)\imx_run.c /X -I$(DDK_INCLUDE) $(DDK_LIB) -Fo$(OBJ_DIR)\$(@B).obj

#
# Host specific build rules
#
clean:
	$(DEL) $(CLEANABLE:/=\)

depend:
	makedepend -fmakefile.depend  -Y $(SOURCE_DIR)/*.c $(TOOLS_DIR)/*.c $(TESTS_DIR)/*.c 2> nul
