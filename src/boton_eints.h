#ifndef __BOTON_EINTS_H__
	#define __BOTON_EINTS_H__

	#include "stdint.h"

	void eints__init(void);

	unsigned int eint0_read_count(void);

	void eint0_clear_nueva_pulsacion(void);

	unsigned int eint0_read_count(void);

	uint8_t nueva_pulsacion_0(void);

	uint8_t nueva_pulsacion_1(void);

	void clear_nueva_pulsacion_0(void);

	void clear_nueva_pulsacion_1(void);

	uint8_t boton0_pulsado(void);

	uint8_t boton1_pulsado(void);

	void boton0_clear(void);

	void boton1_clear(void);

	void boton0_reactivate(void);

	void boton1_reactivate(void);

#endif // __BOTON_EINTS_H__
