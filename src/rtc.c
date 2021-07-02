#include "rtc.h"

/*
 * Inicializa el RTC
 * Resetea la cuenta, ajusta el reloj y activa el enable
 */
void rtc_init(void)	
{
	CCR = 0x2;		// Reset
	
	PREINT = 0x01C8;
	PREFRAC = 0x61C0;
	
	CCR = 0x1;		// Enable
}

/*
 * Devuelve los minutos del juego (entre 0 y 59)
 */
uint8_t rtc_leer_segundos(void)	
{
	// Devuelve directamente los segundos del RTC
	return SEC;
}

/*
 * Devuelve los segundos transcurridos en el minuto actual
 * (entre 0 y 59)
 */
uint8_t rtc_leer_minutos(void)
{
	// Devuelve directamente los minutos del RTC
	return MIN;
}
