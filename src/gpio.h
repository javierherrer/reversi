
#ifndef __GPIO_H__
#define __GPIO_H__

#include <LPC210X.H>
#include <stdint.h>
#include <stdio.h>


void GPIO_iniciar(void);

uint32_t GPIO_leer(uint32_t bit_inicial, uint32_t num_bits);


// Escribe los bits de valor desde 'bit_inicial' hasta el 'bit_inicial'+''
void GPIO_escribir(uint32_t bit_inicial, uint32_t num_bits, uint32_t valor);
	
// Desde el pin numero 'bit_inicial', coloca los 'num_bits' pines como entrada (si hay suficientes, sino los que haya)
void GPIO_marcar_entrada(int32_t bit_inicial, int32_t num_bits);

// Desde el pin numero 'bit_inicial', coloca los 'num_bits' pines como salida (si hay suficientes, sino los que haya)
void GPIO_marcar_salida(int32_t bit_inicial, int32_t num_bits);


#endif // __GPIO_H__
