/*
	ATOSE_PROCESS_ENTRY_POINT.ASM
	-----------------------------
	This is the entry point for an executable
*/

.global _Reset

/*
	ATOSE_EXE_ENTRY_POINT
	---------------------
	All we're going to do is to call main
*/
_Reset:
ATOSE_exe_entry_point:
	bl main

/*
	There should be some "clean up" code here, we'll add it later
*/


