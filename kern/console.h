/* See COPYRIGHT for copyright information. */

#ifndef _CONSOLE_H_
#define _CONSOLE_H_
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/types.h>

#define MONO_BASE	0x3B4
#define MONO_BUF	0xB0000
#define CGA_BASE	0x3D4
#define CGA_BUF		0xB8000

#define CRT_ROWS	25
#define CRT_COLS	80
#define CRT_SIZE	(CRT_ROWS * CRT_COLS)

#define BG_GREEN    0x2000
#define BG_RED      0x4000
#define BG_BLUE     0x1000
#define BG_LIGHT    0x8000
#define BG_WHITE    0x7000
#define BG_DARK     0x0000

#define FG_GREEN    0x0200
#define FG_RED      0x0400
#define FG_BLUE     0x0100
#define FG_LIGHT    0x0800
#define FG_DARK     0x0000
#define FG_WHITE    0x0700

void set_ccolor(int);
void cons_init(void);
int cons_getc(void);

void kbd_intr(void); // irq 1
void serial_intr(void); // irq 4

#endif /* _CONSOLE_H_ */
