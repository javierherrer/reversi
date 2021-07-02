
#ifndef __TIMERS_H__
	#define __TIMERS_H__

	#include "stdint.h"


	void temporizador_iniciar(void);

	void temporizador_empezar(void);

	uint32_t temporizador_leer(void);

	uint32_t temporizador_parar(void);

	uint8_t temporizador_iniciado(void);

	void temporizador_alarma(uint32_t retardo);

	void temporizador_interrupcion_periodica(uint32_t periodo);

#endif // __TIMERS_H__
