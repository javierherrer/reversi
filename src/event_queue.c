#include "event_queue.h"
#include "sys_calls.h"
#include "sc.h"
// cola.c

const int mod_max = MAX_SIZE_QUEUE - 1;

// P3: Podria quitarse el volatile
static volatile struct colaEventos eventos;

void cola_guardar_eventos(event_t evento, uint32_t auxData);

/*
 * Inicializa la cola de eventos con un primer evento 
 * none que realiza la función de centinela
 */
void inicializarCola()	{
	int i;
	
	if(temporizador_iniciado() == 0)
	{	
		// PANIC, temporizador no iniciado
		while(1);
	}
	// Centinela (Hay un primer evento none)
	eventos.ultimo = 0;
	eventos.primero_no_procesado = 0;
	
	for(i = 0; i < MAX_SIZE_QUEUE; i++)
	{
		// Todos los eventos son nulos al inicio
		eventos.eventos[i] = event_none;		
		// No hay tiempo inicial
		eventos.tiempos[i] = 0;							
	}
}

/*
 * Guarda un evento en la cola evitando condiciones de carrera
 */
void cola_guardar_eventos(event_t evento, uint32_t auxData)
{
	int aux;
	// SC
	lock();
	// Se usa una variable auxiliar para control de concurrencia
	aux = (eventos.ultimo + 1) & mod_max;					
	eventos.ultimo = aux;
	unlock();
	
	if (aux != eventos.primero_no_procesado )	{
		// Caso regular, el evento a sustituir ha sido tratado
		// Bits 31-8 auxData[0,23], 7-0 ID_evento
		eventos.eventos[aux] = 
			(to_integer(evento) & 0x000000ff) | (auxData<<8);	
		eventos.tiempos[aux] = temporizador_leer();
		lock();
		if(eventos.primero_no_procesado == -1)
		{
			eventos.primero_no_procesado = aux;
		}
		unlock();
	}
	else	
	{	
		// Trataba de sustituir un evento no tratado, OVERFLOW
		// b. infinito
		while(1);		
	}
}

// Devuelve 1 si hay nuevos eventos sin procesar
// Devuelve 0 si no hay nuevos eventos sin procesar
uint8_t hay_nuevos_eventos()	{
	if(eventos.primero_no_procesado == -1 )		return 0;
	else	return 1;
}

/*
 * Lee un evento de la cola evitando condiciones de carrera
 */
event_t leer_evento(uint32_t * auxData, uint32_t * tiempo) 
{
	event_t aux;
	if(hay_nuevos_eventos() == 0)	
	{	
		// Si no hay nuevos eventos
		return event_none;
	}
	else	
	{			
		// Hay nuevos eventos
		*auxData = 
			(eventos.eventos[eventos.primero_no_procesado] 
				& 0xffffff00)>>8;
		*tiempo = eventos.tiempos[eventos.primero_no_procesado];
		aux = 
			to_event(eventos.eventos[eventos.primero_no_procesado] 
				& 0x000000ff);
		lock();
		if(eventos.primero_no_procesado != eventos.ultimo)
		{
			// Mueve primero_no_procesado a la siguiente posicion
			eventos.primero_no_procesado = 
				(eventos.primero_no_procesado + 1) & mod_max; 
		}
		else
		{
			eventos.primero_no_procesado = -1;
		}
		unlock();
		return aux;
	}
}
