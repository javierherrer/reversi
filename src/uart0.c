#include <LPC210x.H>                     /* LPC21xx definitions               */
#include "event_queue.h"
#define CR 0x0D

static const int WRITE_BUFFER_SIZE = 2048;

static volatile int writeBuffer[WRITE_BUFFER_SIZE];
static volatile int leer 		 = 0;
static volatile int escribir = 0;

int uart0_leyendo()
{
	if(leer != escribir)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int readFromBuffer()
{
	int leido;
	
	if(leer != escribir)
	{
		leido = writeBuffer[leer];
		leer = (leer + 1) % WRITE_BUFFER_SIZE;
		
		return leido;
	}
	else {
		return -1;
	}
}

void writeOnBuffer(int elem)
{
	writeBuffer[escribir] = elem;
	escribir = (escribir + 1) % WRITE_BUFFER_SIZE;
}


// implementation of putchar (also used by printf function to output data)
void sendchar (int ch) // Write character to Serial Port
{                 
  if (ch == '\n') 
	{
    while (!(U0LSR & 0x20));
    U0THR = CR;                          // output CR
  }
  while (!(U0LSR & 0x20));
  U0THR = ch;
}

void sendchar2 (int ch)
{
	int leido = 0;
	
	if (ch != '\0')
	{
		writeOnBuffer(ch);
	}
	
	if(U0LSR & 0x20)
	{
		leido = readFromBuffer();
		if(leido != -1)
		{
			sendchar(leido);
		}
	}
}

/*
 * Guarda una cadena en el buffer circular
 * Será mostrada mediante interrupciones THRE
 *   o USLR & 0x20
 */
void sendstring(char cadena[])
{
	int i = 0;
	int leido = 0;
	
	while(cadena[i] != '\0')
	{
		writeOnBuffer(cadena[i]);
		i++;
	}
	
	if(U0LSR & 0x20)
	{
		leido = readFromBuffer();
		if(leido != -1)
		{
			sendchar(leido);
		}
	}
}

/*
 * Si interrupción es RDA:
 *   Se muestra el caracter y se guarda como evento
 * Si interrupción es THRE:
 *   Se muestra uno de los caracteres pendientes del buffer
 */
void rsi_uart0(void) __irq
{
	int leido;
	if (U0IIR & (1<<2)) 
	{ 
		// If the RDA interrupt is triggered
		if(U0LSR & 0x01)
		{
			leido = U0RBR;
			sendchar(leido);
			cola_guardar_eventos(event_char_leido, leido);
		}
	}
	else 
	{ 				
		// THRE Interrupt
		leido = readFromBuffer();
		if(leido != -1)
		{
			sendchar(leido);
		}
	}
	
	VICVectAddr = 0;
}

/* 
 * Initialize Serial Interface
 */
void init_serial (void) 
{               
  PINSEL0 &= 0x0;
  PINSEL0 |= 0x5; /* Enable RxD0 and TxD0              */
  U0LCR = 0x83;   /* 8 bits, no Parity, 1 Stop bit     */
  U0DLL = 10;     /* 9600 Baud Rate @ 15MHz VPB Clock  */
  U0LCR = 0x03;   /* DLAB = 0                          */

	// RBR and THRE interrupts enabled
  U0IER |= (1<<0)|(1<<1);														
	// set interrupt vector in 2
  VICVectAddr4 = (unsigned long)rsi_uart0;          
  
	// 0x20 bit 5 enables vectored IRQs. 
  // 6 is the number of the interrupt assigned. 
	// Number 6 is the UARTT0
  VICVectCntl4 = 0x20 | 6;                   
	// Enable UART0 Interrupt
  VICIntEnable = VICIntEnable | 0x00000040;                  
	
	leer = 0;
	escribir = 0;
}



