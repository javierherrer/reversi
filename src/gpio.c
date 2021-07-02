#include "gpio.h"

extern uint32_t lsl(uint32_t reg, uint32_t desp);
extern uint32_t lsr(uint32_t reg, uint32_t desp);


static volatile uint32_t aux = 0;
static volatile uint32_t dim = sizeof(aux) * 8;	// En principio dim = 32

/*
 * bit_inicial indica el primer bit a leer
 * num_bits indica cuántos bits queremos leer
 * 
 * Devuelve un entero con el valor de los bits indicado
 */
uint32_t GPIO_leer(uint32_t bit_inicial, uint32_t num_bits)
{
	aux = IOPIN;
	aux = lsl(aux, dim - bit_inicial - num_bits);
	aux = lsr(aux, dim - num_bits);
	return aux;
}
// Example: IOPIN = 0x12345678, args = (8,4): 0x12345678 --> 0x67800000 --> 0x00000006


// Escribe los bits de valor desde 'bit_inicial' hasta el 'bit_inicial'+''

/*
 * Similar a GPIO_leer pero escribiendo en los bits 
 * indicados el valor
 *
 * Si el valor no puede representarse en los bits indicados
 *  se escribirán los num_bits menos significativos a 
 *  partir del inicial
 */
void GPIO_escribir(uint32_t bit_inicial, uint32_t num_bits, 
	                 uint32_t valor)	
{
	// Bits inicial y final a tratar
	uint32_t ini = lsl(0x1, bit_inicial);	
	
	// contador de bits escritos
	int bits_written = 0;		
	uint32_t j;
	for(j = ini; bits_written < num_bits; j = lsl(j,1) )	
	{
		if( (j & valor) > 0)
		{
			// Si el bit j de valor es 1, set bit j en GPIO
			IOSET = j;	
		}
		else
		{
			// Si el bit j de valor es 0, clear bit j en GPIO
			IOCLR = j;	
		}			
		bits_written++;
		
		if(j == 0x8000000) 
		{
			break;
		}
	}
}	
// GPIO: 0x12345678 (12,4) valor: 0xffffffff ---> GPIO: 0x1234f678
	
// Desde el pin numero 'bit_inicial', coloca los 'num_bits' pines como entrada (si hay suficientes, sino los que haya)

/*
 * Los bits indicados se utilizarán como pines de entrada
 */
void GPIO_marcar_entrada(int32_t bit_inicial,
	                       int32_t num_bits) 
{
	uint32_t i;
	// Bit inicial
	uint32_t ini = lsl(0x1, bit_inicial);	
	int bits_set = 0;
	for(i = ini; bits_set < num_bits; i = lsl(i,1) )	
	{
		// Bit i = 0
		IODIR = IODIR & (~i);	
		bits_set++;
		
		if(i == 0x8000000) break;
	}
}

// Desde el pin numero 'bit_inicial', coloca los 'num_bits' pines como salida (si hay suficientes, sino los que haya)

/*
 * Los bits indicados se utilizarán como pines de salida
 */
void GPIO_marcar_salida(int32_t bit_inicial, 
	                      int32_t num_bits) 
{
	// Bit inicial
	uint32_t ini = lsl(0x1, bit_inicial);	
	int bits_set = 0;
	uint32_t i;
	for(i = ini; bits_set < num_bits; i = lsl(i,1) )	
	{
		// Bit i = 1
		IODIR = IODIR | i;	
		bits_set++;
		
		if(i == 0x80000000) break;
	}
}
