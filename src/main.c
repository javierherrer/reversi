
#include <LPC210x.H>                       /* LPC210x definitions */
#include "stdint.h"
#include "timers.h"
#include "boton_eints.h"
#include "event_queue.h"
#include "gpio.h"
#include "Gestion_pulsacion.h"

#include "rtc.h"
#include "watchdog.h"
#include "sys_calls.h"

#include "Power_management.h"
#include "reversi8.h"
#include "uart0.h"


int main (void) {
	temporizador_iniciar();
	temporizador_empezar();
	
	// Cola de eventos necesita temporizador iniciado
	inicializarCola();
	
	init_serial();
	eints__init();
	
	// P3 interaccion por UART0
	//GPIO_marcar_entrada(0,3);
	//GPIO_marcar_entrada(8,3);
	
	GPIO_marcar_salida(31,1);
	rtc_init();
	
	planificador();
}
