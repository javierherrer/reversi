#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#include <LPC210x.H>                       /* LPC210x definitions */

void WD_init(int sec);

void WD_feed(void);

#endif // __WATCHDOG_H__
