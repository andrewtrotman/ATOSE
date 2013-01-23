VER_8 = C:\Program Files (x86)\Windows Kits\8.0\

!IF "$(WINDOWSSDKDIR)" == "$(VER_8)"
DDK_DIR = $(WINDOWSSDKDIR)\Include
DDK_INCLUDE = "$(DDK_DIR)\shared" -I"$(DDK_DIR)\km" -I"$(DDK_DIR)\km\crt" -I"$(DDK_DIR)\um"
DDK_LIB = "$(WINDOWSSDKDIR)\Lib\win8\um\x64\setupapi.lib" "$(WINDOWSSDKDIR)\Lib\win8\um\x64\hid.lib"
!ELSE
DDK_DIR = "c:\WinDDK\7600.16385.1\inc"
DDK_INCLUDE = $(DDK_DIR)\ddk -I$(DDK_DIR)\api -I$(DDK_DIR)\crt
DDK_LIB = c:\WinDDK\7600.16385.1\lib\wxp\i386\setupapi.lib c:\WinDDK\7600.16385.1\lib\wxp\i386\hid.lib
!ENDIF


all : imx_run.exe image_dump.exe


imx_run.exe : imx_run.c
	@cl /nologo /W3 /Zi imx_run.c /X -I$(DDK_INCLUDE) $(DDK_LIB)

image_dump.exe : image_dump.c
	@cl /nologo /W3 /Zi image_dump.c -D_CRT_SECURE_NO_DEPRECATE

clean:
	del *.obj *.exe *.bak *.pdb *.sln *.ilk
