#include "watchdog.h"
#include "sys_calls.h"
const int TICKS_SEG = 14999 * 250;		// Perido de 1 s

/*
 * Inicializa el watchdog timer para que se resetee el
 *  procesador dentro de sec segundos si no se le 
 *  "alimenta"
 */
void WD_init(int sec)	{
	// Asigna el periodo
	WDTC = sec * TICKS_SEG;
	WDMOD = 0x3;
	// Se debe alimentar una primera vez para ponerlo en 
	//  marcha
	WD_feed();
}

/* 
 * Alimenta al watchdog timer
 */
void WD_feed(void)	{
	// Desactivar interrupciones
	disable_isr_fiq();
	
	WDFEED = 0xAA;
	WDFEED = 0x55;
	
	enable_isr_fiq();
}
