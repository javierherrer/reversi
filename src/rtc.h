#ifndef __RTC_H__
#define __RTC_H__

#include <LPC210x.H>                       /* LPC210x definitions */
#include "eventos.h"

void rtc_init(void);

uint8_t rtc_leer_segundos(void);

uint8_t rtc_leer_minutos(void);

#endif // __RTC_H__
