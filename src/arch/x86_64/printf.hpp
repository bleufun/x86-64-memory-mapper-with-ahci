#ifndef __PRINTF_HPP
#define __PRINTF_HPP

static int xpos;
static int ypos;

#define VGA_MODE_WIDTH 80
#define VGA_MODE_HEIGHT 24

#define video (unsigned char*)0xb8000
#define LINES 24
#define ATTRIBUTE 0xf4
#define COLUMNS 80

void clean_vga_buffer(char color);
void itoa (char *buf, long long base, long long d);
void prints(char *string, int length);
void printf (const char *format, ...);

#endif
