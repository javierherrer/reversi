#ifndef __POWER_MANAGEMENT_H__
#define __POWER_MANAGEMENT_H__

#include <LPC210x.H>                       /* LPC210x definitions */

void PM_power_down (void);

void PM_idle (void);

extern void Switch_to_PLL(void);

#endif // __POWER_MANAGEMENT_H__
