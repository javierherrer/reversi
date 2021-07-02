#include "boton_eints.h"
#include "eventos.h"
#include "event_queue.h"
#include <LPC210X.H>                            // LPC21XX Peripheral Registers


// variable para comprobar que se hacen las interrupciones que deberían hacerse
static volatile unsigned int eint0_count = 0;
// variable que se activa al detectar una nueva pulsación
static volatile int eint0_nueva_pulsacion = 0;

static volatile int nueva_pulsacion_eint0 = 0;
static volatile int nueva_pulsacion_eint1 = 0;

// Funciones

/*
 * Gestiona la interrupción de EINT0
 * Durante la gestión se dehabilitará la interrupción 
 *  externa correspondiente en el VIC, para que no vuelva 
 *  a interrumpir hasta que no termine la gestión de la 
 *  pulsación
 */
void rsi_eint0(void) __irq	{
	VICIntEnClr = 0x00004000;
	nueva_pulsacion_eint0 = 1;
	cola_guardar_eventos(event_eint0, 0);
	// Clear interrupt flag
	EXTINT =  EXTINT | 1;		
	// Acknowledge Interrupt
	VICVectAddr = 0;                     
}

/*
 * Gestiona la interrupción de EINT1
 * Durante la gestión se dehabilitará la interrupción 
 *  externa correspondiente en el VIC, para que no vuelva 
 *  a interrumpir hasta que no termine la gestión de la 
 *  pulsación
 */
void rsi_eint1(void) __irq	{
	VICIntEnClr = 0x00008000;
	nueva_pulsacion_eint1 = 1;
	cola_guardar_eventos(event_eint1, 0);
	// Clear interrupt flag
	EXTINT =  EXTINT | 2;		
	// Acknowledge Interrupt
	VICVectAddr = 0;                            
}

/*
 * Devuelve 1 si ha habido nueva pulsación
 */
uint8_t nueva_pulsacion_0()	{
	return nueva_pulsacion_eint0;
}

/*
 * Devuelve 1 si ha habido nueva pulsación
 */
uint8_t nueva_pulsacion_1()	{
	return nueva_pulsacion_eint1;
}

/*
 * Resetea a cero
 */
void clear_nueva_pulsacion_0()	{
	nueva_pulsacion_eint0 = 0;
}

/*
 * Resetea a cero
 */
void clear_nueva_pulsacion_1()	{
	nueva_pulsacion_eint1 = 0;
}

/*
 * Inicialización de los botones
 */
void eints__init (void) 
{
	nueva_pulsacion_eint0 = 0;
	eint0_count = 0;
	// clear interrupt flag 
	EXTINT =  EXTINT | 1;            	
	//IRQ slot number 2 of the VIC for EXTINT0
	VICVectAddr2 = (unsigned long)rsi_eint0;      
  // 0x20 bit 5 enables vectored IRQs. 
  // 14 is the number of the interrupt assigned. 
	// Number 14 is the EINT0 
	// (see table 40 of the LPC2105 user manual)
	// Sets bits 0 and 1 to 0
	PINSEL1 		= PINSEL1 & 0xfffffffC;	
	// Enable the EXTINT0 interrupt
	PINSEL1 		= PINSEL1 | 1;					
	VICVectCntl2 = 0x20 | 14;                  
	// Enable EXTINT0 Interrupt
  VICIntEnable = VICIntEnable | 0x00004000;                  
	
	
	nueva_pulsacion_eint1 = 0;
	// clear interrupt flag  
	EXTINT =  EXTINT | 2;           	
	// IRQ slot number 2 of the VIC for EXTINT0
	// set interrupt vector in 2
	VICVectAddr3 = (unsigned long)rsi_eint1;          
  // 0x20 bit 5 enables vectored IRQs. 
	// 14 is the number of the interrupt assigned. 
	// Number 14 is the EINT0 
	// (see table 40 of the LPC2105 user manual) 
	// Sets bits 0 and 1 to 0
	PINSEL0 		= PINSEL0 & 0xcfffffff;				
	// Enable the EXTINT1 interrupt 2_1110
	PINSEL0 		= PINSEL0 | 0x20000000;					
	VICVectCntl3 = 0x20 | 15;                   
	// Enable EXTINT1 Interrupt
  VICIntEnable = VICIntEnable | 0x00008000;                  
}

uint8_t boton0_pulsado()	{
	if( (EXTINT & 0x1) != 0 ) return 1;
	else return 0;
}

uint8_t boton1_pulsado()	{
	if( (EXTINT & 0x2) != 0 ) return 1;
	else return 0;
}

void boton0_clear()	{
	EXTINT =  EXTINT | 1;
}

void boton1_clear()	{
	EXTINT =  EXTINT | 2;
}

void boton0_reactivate()	{
	VICIntEnable = VICIntEnable | 0x00004000;
}

void boton1_reactivate()	{
	VICIntEnable = VICIntEnable | 0x00008000;
}
