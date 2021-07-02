

#include "boton_eints.h"
#include "timers.h"
#include "Gestion_pulsacion.h"

typedef enum {
	NO_PULSADO = 0,
	PULSADO = 1
} estado;

estado procesando_eint0 = NO_PULSADO;
estado procesando_eint1 = NO_PULSADO;

uint32_t periodo_espera = 50;

int gestionando_eint0()	{
	return procesando_eint0;
}

/*
 * Monitoriza la pulsación cada 50 ms.
 * Si la tecla sigue pulsada no se hará nada
 * En caso contario se volverá a habilitar la interrupción
 *  de ese botón.
 */
int gestion_eint0_pulsado(event_t evento)	
{
	if(procesando_eint0 == PULSADO && evento == event_alarma)	
	{
		if(boton0_pulsado() == 1)	
		{
			temporizador_alarma(periodo_espera);
			boton0_clear();
		}
		else	
		{
			clear_nueva_pulsacion_0();
			boton0_reactivate();
			// Cambio estado
			procesando_eint0 = NO_PULSADO;	
		}
	}
	else if(evento == event_eint0)	
	{
		// Cambio estado
		procesando_eint0 = PULSADO;		
		temporizador_alarma(periodo_espera);
	}
	return gestionando_eint0();
}

int gestionando_eint1()	{
	return procesando_eint1;
}

/*
 * Monitoriza la pulsación cada 50 ms.
 * Si la tecla sigue pulsada no se hará nada
 * En caso contario se volverá a habilitar la interrupción
 *  de ese botón.
 */
int gestion_eint1_pulsado(event_t evento)	
{
	if(procesando_eint1 == PULSADO && evento == event_alarma)	
	{
		if(boton1_pulsado() == 1)	
		{
			temporizador_alarma(periodo_espera);
			boton1_clear();
		}
		else	
		{
			clear_nueva_pulsacion_1();
			boton1_reactivate();
			// Cambio estado
			procesando_eint1 = NO_PULSADO;	
		}
	}
	else if(evento == event_eint1)	
	{
		// Cambio estado
		procesando_eint1 = PULSADO;		
		temporizador_alarma(periodo_espera);
	}
	return gestionando_eint1();
}
