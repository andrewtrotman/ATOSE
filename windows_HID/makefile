DDK_INCLUDE = "c:\WinDDK\7600.16385.1\inc\ddk" -I"c:\WinDDK\7600.16385.1\inc\api" -I"c:\WinDDK\7600.16385.1\inc\crt"
DDK_LIB = c:\WinDDK\7600.16385.1\lib\wxp\i386\setupapi.lib c:\WinDDK\7600.16385.1\lib\wxp\i386\hid.lib


### all : hid_talk.exe example.exe

hid_talk.exe : hid_talk.c
	cl /Zi hid_talk.c /X -I$(DDK_INCLUDE) $(DDK_LIB)

example.exe : example.c
	cl /Tp example.c

clean:
	del *.obj *.exe *.bak *.pdb *.sln *.ilk
