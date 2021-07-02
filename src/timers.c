// Comentario inicial
//
//
//	Anotar que el periodo de interrupcion de timer0 tiene importantes condiciones. En este caso se ha tomado 1ms / interrupcion con el objetivo de reducir al minimo el numero de interrupciones, y sin perder
//  mucha precision, ya que los periodos que emplea el programa son grandes (50ms, 100ms), por lo que la variación es pequeña.
//	También se ha considerado el apagado del timer0 cuando no exista alarma ni interrupción periódica, pero finalmente solo se apaga cuando finaliza una alarma y no existe (aun) un interr. periódica establecida.


#include "timers.h"
#include <LPC210X.H>                            // LPC21XX Peripheral Registers
#include "event_queue.h"
// variable para contabilizar el número de interrupciones
static const unsigned int TICKS_MS = 14999;		// Perido de 1 ms
// Periodo de interrupción del timer0
static const unsigned int TICKS_MAX = 0xffffffff; 				// Cada cuanto interrumpe timer1

static volatile unsigned int t1_inicializado = 0;		// Var. para saber si se ha comenzado a contar tiempo

// Contador de periodos pasados por timer0
static volatile unsigned int t1_periodo_count = 0;
static volatile unsigned int t0_periodo_actual = 0;
static volatile unsigned int alarma_periodo_esperado = 0, alarma_set = 0;
static volatile unsigned int interr_periodo_esperado = 0, interr_set = 0, period;

/* Timer Counter 0 Interrupt executes each 100ms @ 60 MHz CPU Clock */

/*
 * Aumenta el contador en cada interrupción
 */
void temporizador1_rsi (void) __irq {
		t1_periodo_count++;
		T1IR = 1;     			// Clear interrupt flag
		VICVectAddr = 0;    // Acknowledge Interrupt
}

/*
 * Guarda evento alarma o alarma_periodica si corresponde
 * para esa interrupción
 */
void temporizador0_rsi (void) __irq  {
		t0_periodo_actual++;
		if(t0_periodo_actual == alarma_periodo_esperado) {
			// Interrupcion alarma
			cola_guardar_eventos(event_alarma, t0_periodo_actual);
			if(interr_set == 0) {
				// Si no hay nada mas que hacer, para el reloj
				T0TCR = 0; 						
			}
		}
		if(interr_set == 1 && 
					t0_periodo_actual == interr_periodo_esperado)	{
			// Interrupción periódica
			cola_guardar_eventos(event_alarma_periodica, period);
			interr_periodo_esperado =
						interr_periodo_esperado + period;
		}
		// Clear interrupt flag
		T0IR = 1;            
		// Acknowledge Interrupt
		VICVectAddr = 0;                            
}

/*
 * Inicializa timer0 y timer1
 */
void temporizador_iniciar(void)
{
		// Interrumpe cada ms
		T0MR0 = TICKS_MS;     
		// Interrumpe y reset a T0TC when MR0 is reached
    T0MCR = 0x3;               
		// Enables Timer0 Interrupt
    VICIntEnable = VICIntEnable | 0x00000010;   
		// Selects FIQ interrupts
    VICIntSelect = VICIntSelect | 0x00000010;          
	
		// Interrumpe cada periodo * ms = 
		// 		t1_periodo * 150.000-1 counts / 0.05 ms
		T1MR0 = TICKS_MAX; 				
		// Generates an interrupt and resets the count when 
		// 		the value of MR0 is reached and stops the counter 
		// 		(should act like an alarm)
		T1MCR = 0x3;	               			
	  // configuration of the IRQ slot number 0 of the VIC 
		// 		for Timer 0 Interrupt
		// set interrupt vector in 1
		VICVectAddr1 = (unsigned long)temporizador1_rsi;  
    // 0x20 bit 5 enables vectored IRQs.
		// 4 is the number of the interrupt assigned. 
		// Number 4 is the Timer 0 
		//		(see table 40 of the LPC2105 user manual  
		// Timer1 -> Slot 1
		VICVectCntl1 = 0x20 | 5;             
		// Enable Timer1 Interrupt
		VICIntEnable = VICIntEnable | 0x00000020;  
}

/*
 * Iniciar timer1
 */
void temporizador_empezar(void)	{
	t1_inicializado = 1;
	// Timer0 reset
	T1TCR = 0x3;	
	// Reset periodos
	t1_periodo_count = 0;													
	// Prescale Register reset
	T1PC = 0;																					 
	// Timer0 enable (T0TC)
	T1TCR = 0x1;                     									 
}

/*
 * Devuelve el tiempo transcurrido en el timer1
 */
uint32_t temporizador_leer(void)	{
	if(T1TCR == 0x1)
	{
		// Si están contando timer1, 
		//   devuelve el tiempo transcurrido
		return ((long long)t1_periodo_count * 
			((long long)TICKS_MAX + 1) + T1TC) / TICKS_MS;			
	}
	else
	{		
		return 0;	// Sino devuelve 0
	}
}

/*
 * Detiene timer1
 *
 * Devuelve el tiempo transcurrido desde 
 *  temporizador_empezar
 */
uint32_t temporizador_parar(void)
{
	uint32_t aux;
	if(T1TCR == 1)	
	{	
		// Si está activado timer0
		t1_inicializado = 0;
		aux = temporizador_leer(); // Toma el tiempo
		T1TCR = 0;								 // Disables timer0
		return aux;								 // Devuelve t transcurrido
	}
	else 
	{
		// Sino devuelve 0
		return 0;	
	}
}

/*
 * Acceso al timer1 vía una interrupción
 */
uint32_t __swi(0) clock_gettime(void);
uint32_t __SWI_0 (void) {
	return temporizador_leer();
}

uint8_t temporizador_iniciado(void)	{
	return t1_inicializado;
}

/*
 * Genera una notificación (evento en cola de eventos)
 *  dentro de un retardo indicado en milisegundos
 */
void temporizador_alarma(uint32_t retardo) 
{
	if(retardo > 0)
	{
		// Hay una alarma establecida
		alarma_set = 1;																									
		if(T0TCR == 0) 
		{
			t0_periodo_actual = 0; 
			T0TCR = 3; 
			T0PC = 0; 
			T0TCR = 1;
			// Timer0 reset and enable
		}													
		alarma_periodo_esperado = t0_periodo_actual + retardo;
	}
}

void temporizador_interrupcion_periodica(uint32_t periodo)	{
	if(periodo > 0)	{
		interr_set = 1;																									// Hay una alarma establecida
		period = periodo;
		if(T0TCR == 0) {t0_periodo_actual = 0; T0TCR = 3; T0PC = 0; T0TCR = 1;}
		interr_periodo_esperado = t0_periodo_actual + periodo;
		// if(T0TC > 1) interr_periodo_esperado++;													// Guarda en alarma_periodo_esperado el periodo en el que saltara la alarma
	}
}

void pause_timer0()	{
	T0TCR = 0;
}


void resume_timer0()	{
	T0TCR = 1;
}
