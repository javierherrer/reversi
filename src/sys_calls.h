#ifndef __SYS_CALLS_H__
#define __SYS_CALLS_H__

#include <LPC210x.H>                       /* LPC210x definitions */

void __swi(0xFF) enable_isr (void);
void __swi(0xFE) disable_isr (void);
void __swi(0xFD) enable_isr_fiq (void);
void __swi(0xFC) disable_isr_fiq (void);

int __swi(0xFB) lock_isr_fiq(void);
void __swi(0xFA) enable_fiq(void);

#endif // __SYS_CALLS_H__
