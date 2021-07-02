#ifndef __EVENT_QUEUE_H__
#define __EVENT_QUEUE_H__

#include <LPC210x.H>
#include "stdint.h"
#include "timers.h"
#include "eventos.h"

const static int MAX_SIZE_QUEUE = 32;


extern uint32_t lsl(uint32_t reg, uint32_t desp);
extern uint32_t lsr(uint32_t reg, uint32_t desp);

// Struct cola circular de eventos
struct colaEventos
{
	int8_t ultimo;										// Indica la posicion del ultimo evento introducido
	int8_t primero_no_procesado;			// Indica la posicion del evento mas antiguo sin procesar
	uint32_t eventos[MAX_SIZE_QUEUE];							// Eventos
	uint32_t tiempos[MAX_SIZE_QUEUE];							// Tiempos
};

// Inicializa la cola de eventos
void inicializarCola(void);

uint8_t hay_nuevos_eventos(void);

// Guarda el evento del tipo ID_evento con la información auxiliar 'auxData'
void cola_guardar_eventos(event_t ID_evento, uint32_t auxData);

event_t leer_solo_evento(void);

event_t leer_evento(uint32_t * auxData, uint32_t * tiempo);

#endif // __EVENT_QUEUE__
