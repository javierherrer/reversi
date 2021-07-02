#include "led.h"
#include "gpio.h"

const int periodo_interr = 25;

int set = 0;

static volatile int interr_restantes = periodo_interr;


void gestion_led(event_t evento)
{	
	if(evento == event_alarma_periodica)	{
		interr_restantes = interr_restantes - 1;
		
		if(interr_restantes == 0)	{
			interr_restantes = periodo_interr;	// Reset del contador
			if(set == 0)	{
				set = 1;
				GPIO_escribir(31,1,0x80000000);
			}
			else	{
				set = 0;
				GPIO_escribir(31,1,0);
			}
		}
	}
}
