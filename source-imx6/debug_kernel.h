/*
	DEBUG_KERNEL.H
	--------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef DEBUG_KERNEL_H_
#define DEBUG_KERNEL_H_

#include <stdint.h>

void debug_init(void);
void debug_putc(char value);
void debug_print_hex(int data);
void debug_print_hex_byte(uint8_t data);
void debug_print_string(const char *string);
void debug_print_this(const char *start, uint32_t hex, const char *end = "");
void debug_print_cf_this(const char *start, uint32_t hex1, uint32_t hex2, const char *end = "");
void debug_dump_buffer(unsigned char *buffer, uint32_t address, uint64_t bytes);
void debug_print_registers(void);

#endif