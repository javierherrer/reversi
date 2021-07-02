#include "Power_management.h"
#include "boton_eints.h"

/*
 * Set the processor into power down state 
 * The watchdog cannot wake up the processor from power down
 */
void PM_power_down (void)  
{
	// EXTINT0 and EXTINT1 will awake the processor
  EXTWAKE = 0x3; 		
 
//	Se fuerza que los botones puedan interrumpir
//	¡POSIBLE CONDICION DE CARRERA!
//	Se trata de leer pero no hay evento, a la vez que 
//	interrumpe una rsi de boton, luego el procesador se 
//	duerme pero el evento eint no se ha tratado
	
	boton0_reactivate();
	boton1_reactivate();
	
	// Power-down mode
	PCON |= 0x02;		
	Switch_to_PLL();
}


/*
 * Set the processor into idle state 
 * Peripherals can wake up the processor with interrupts
 */
void PM_idle (void)  {
	// Idle mode (PC stopped but peripherals remain active)
	PCON |= 0x01;		
	Switch_to_PLL();			
}
