
#ifndef __EVENTOS_H__
#define __EVENTOS_H__

#include "stdint.h"

// Definicion de los eventos del sistema
typedef enum {
	event_none,
	event_eint0,
	event_eint1,
	event_alarma,
	event_alarma_periodica,
	event_char_leido,
	event_comando_reconocido,
	event_comando_pasar,
	event_comando_acabar,
	event_comando_nueva,
	event_comando_jugada,
} event_t;

event_t to_event(uint8_t a_convertir);

uint8_t to_integer(event_t a_convertir);

#endif // __EVENTOS_H__
