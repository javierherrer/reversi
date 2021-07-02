#include "interprete.h"
#include "event_queue.h"
#include "uart0.h"

const static int MAX_BUFFER_SIZE = 10;
static uint8_t buffer[MAX_BUFFER_SIZE] = {0,0,0,0,0,0,0,0,0,0};

static int reading = 0, index = 0;

/*
 * Guarda un caracter en el buffer
 */
void guardar_caracter(uint8_t caracter) 
{
  if(index < MAX_BUFFER_SIZE) 
	{
    buffer[index] = caracter;
    index++;
  }
  else  
	{
    // ERROR: Buffer overflow
    while(1);
  }
}

/*
 * Guarda un comando completo como evento
 */
void guardar_comando()	
{
	uint32_t comando = 0;
	comando += buffer[2];		// 0 1 2
	comando += buffer[1] << 8;
	comando += buffer[0] << 16;
	cola_guardar_eventos(event_comando_reconocido, comando);
}

/*
 * Procesa un caracter recibido siguiendo el patrón #xxx!
 */
void procesar_caracter(uint8_t character)	
{
	if(character == '#') 
	{
		reading = 1;
		index = 0;
	}
	else if(index == 3) 
	{
		if(character == '!' && reading == 1)
		{
			guardar_comando();
			sendchar('\n');
		}
		reading = 0;
		index = 0;
	}
	else if(reading == 1) 
	{
		guardar_caracter(character);
	}
}

