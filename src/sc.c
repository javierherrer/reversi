

#include "sys_calls.h"

int estado = 0;

/*
 * Bloquea interrupciones y guarda estado
 */
void lock()
{
	estado = lock_isr_fiq();
}

/* 
 * Recupera el estado anterior
 * Reactiva las interrupcines apropiadas
 */
void unlock()
{
	if(estado == 0)
	{
		// MODO FIQ
		// no se hace nada
	}
		
	else if(estado == 2)
	{
		// MODO IRQ
		enable_fiq();
	}
		
	else if(estado == 3)
	{
		// MODO USUARIO
		enable_isr_fiq();
	}
}
