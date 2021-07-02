#ifndef __UART0_H__
#define __UART0_H__

#include "stdint.h"

int uart0_leyendo(void);

void init_serial(void);

void sendchar (int ch);

void sendchar2 (int ch);

void sendstring(char cadena[]);

#endif // __UART0_H__
