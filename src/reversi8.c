#include <stdlib.h>
#include <string.h>
#include "stdint.h"
#include "timers.h"
#include "boton_eints.h"
#include "event_queue.h"
#include "gpio.h"
#include "Gestion_pulsacion.h"
#include "Power_management.h"
#include "led.h"
#include "watchdog.h"
#include "uart0.h"
#include "interprete.h"
#include "rtc.h"
#include "reversi8.h"

// Estados del juego
typedef enum {
	INICIAL,
	USUARIO,
	PARPADEO,
	IA,
	FINAL
} estado;


volatile static int usuario_ha_pasado = 0;

// Tamaño del tablero
enum { DIM=8 };

// Valores que puede devolver la función patron_volteo())
enum {
	NO_HAY_PATRON = 0,
  PATRON_ENCONTRADO = 1
};

enum {
	CASILLA_VACIA = 0,
	FICHA_BLANCA = 1,
	FICHA_NEGRA = 2
};

enum {
	CHAR_VACIA = '-',
	CHAR_BLANCA = 'B',
	CHAR_NEGRA = 'N'
};

// candidatas: indica las posiciones a explorar
// Se usa para no explorar todo el tablero innecesariamente
// Sus posibles valores son NO, SI, CASILLA_OCUPADA
const int8_t  NO              = 0;
const int8_t  SI              = 1;
const int8_t  CASILLA_OCUPADA = 2;

/////////////////////////////////////////////////////////////////////////////
// TABLAS AUXILIARES
// declaramos las siguientes tablas como globales para que sean más fáciles visualizarlas en el simulador
// __attribute__ ((aligned (8))): specifies a minimum alignment for the variable or structure field, measured in bytes, in this case 8 bytes

static const int8_t __attribute__ ((aligned (8))) tabla_valor[DIM][DIM] =
{
    {8,2,7,3,3,7,2,8},
    {2,1,4,4,4,4,1,2},
    {7,4,6,5,5,6,4,7},
    {3,4,5,0,0,5,4,3},
    {3,4,5,0,0,5,4,3},
    {7,4,6,5,5,6,4,7},
    {2,1,4,4,4,4,1,2},
    {8,2,7,3,3,7,2,8}
};


// Tabla de direcciones. Contiene los desplazamientos de las 8 direcciones posibles
const int8_t vSF[DIM] = {-1,-1, 0, 1, 1, 1, 0,-1};
const int8_t vSC[DIM] = { 0, 1, 1, 1, 0,-1,-1,-1};

//////////////////////////////////////////////////////////////////////////////////////
// Variables globales que no deberían serlo
// tablero, fila, columna y ready son varibles que se deberían definir como locales dentro de reversi8.
// Sin embargo, las hemos definido como globales para que sea más fácil visualizar el tablero y las variables en la memoria
//////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Tablero sin inicializar
////////////////////////////////////////////////////////////////////
static int8_t __attribute__ ((aligned (8))) tablero[DIM][DIM] = {
	        {CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA},
	        {CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA},
	        {CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA},
	        {CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA},
	        {CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA},
	        {CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA},
	        {CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA},
	        {CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA}
	    };

  ////////////////////////////////////////////////////////////////////
     // VARIABLES PARA INTERACCIONAR CON LA ENTRADA SALIDA
     // Pregunta: ¿hay que hacer algo con ellas para que esto funcione bien?
     // (por ejemplo añadir alguna palabra clave para garantizar que la sincronización a través de esa variable funcione)
static int8_t fila 		= 0;
static int8_t columna = 0;

static int8_t __attribute__ ((aligned (8))) candidatas[DIM][DIM] =
	{
			{NO,NO,NO,NO,NO,NO,NO,NO},
			{NO,NO,NO,NO,NO,NO,NO,NO},
			{NO,NO,NO,NO,NO,NO,NO,NO},
			{NO,NO,NO,NO,NO,NO,NO,NO},
			{NO,NO,NO,NO,NO,NO,NO,NO},
			{NO,NO,NO,NO,NO,NO,NO,NO},
			{NO,NO,NO,NO,NO,NO,NO,NO},
			{NO,NO,NO,NO,NO,NO,NO,NO}
	};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 0 indica CASILLA_VACIA, 1 indica FICHA_BLANCA y 2 indica FICHA_NEGRA
// pone el tablero a cero y luego coloca las fichas centrales.
void init_table(int8_t tablero[][DIM], int8_t candidatas[][DIM])
{
    int i, j;

    for (i = 0; i < DIM; i++)
    {
        for (j = 0; j < DIM; j++)
            tablero[i][j] = CASILLA_VACIA;
    }
#if 0
    for (i = 3; i < 5; ++i) {
	for(j = 3; j < 5; ++j) {
	    tablero[i][j] = i == j ? FICHA_BLANCA : FICHA_NEGRA;
	}
    }

    for (i = 2; i < 6; ++i) {
	for (j = 2; j < 6; ++j) {
	    if((i>=3) && (i < 5) && (j>=3) && (j<5)) {
		candidatas[i][j] = CASILLA_OCUPADA;
	    } else {
		candidatas[i][j] = SI; //CASILLA_LIBRE;
	    }
	}
    }
#endif
    // arriba hay versión alternativa
    tablero[3][3] = FICHA_BLANCA;
    tablero[4][4] = FICHA_BLANCA;
    tablero[3][4] = FICHA_NEGRA;
    tablero[4][3] = FICHA_NEGRA;

    candidatas[3][3] = CASILLA_OCUPADA;
    candidatas[4][4] = CASILLA_OCUPADA;
    candidatas[3][4] = CASILLA_OCUPADA;
    candidatas[4][3] = CASILLA_OCUPADA;

    // casillas a explorar:
    candidatas[2][2] = SI;
    candidatas[2][3] = SI;
    candidatas[2][4] = SI;
    candidatas[2][5] = SI;
    candidatas[3][2] = SI;
    candidatas[3][5] = SI;
    candidatas[4][2] = SI;
    candidatas[4][5] = SI;
    candidatas[5][2] = SI;
    candidatas[5][3] = SI;
    candidatas[5][4] = SI;
    candidatas[5][5] = SI;
}

////////////////////////////////////////////////////////////////////////////////
// Espera a que ready valga 1.
// CUIDADO: si el compilador coloca esta variable en un registro, no funcionará.
// Hay que definirla como "volatile" para forzar a que antes de cada uso la cargue de memoria

void esperar_mov(int8_t *ready)
{
    while (*ready == 0) {};  // bucle de espera de respuestas hasta que el se modifique el valor de ready (hay que hacerlo manualmente)

    *ready = 0;  //una vez que pasemos el bucle volvemos a fijar ready a 0;
}

////////////////////////////////////////////////////////////////////////////////
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IMPORTANTE: AL SUSTITUIR FICHA_VALIDA() Y PATRON_VOLTEO()
// POR RUTINAS EN ENSAMBLADOR HAY QUE RESPETAR LA MODULARIDAD.
// DEBEN SEGUIR SIENDO LLAMADAS A FUNCIONES Y DEBEN CUMPLIR CON EL ATPCS
// (VER TRANSPARENCIAS Y MATERIAL DE PRACTICAS):
//  - DEBEN PASAR LOS PARAMETROS POR LOS REGISTROS CORRESPONDIENTES
//  - GUARDAR EN PILA SOLO LOS REGISTROS QUE TOCAN
//  - CREAR UN MARCO DE PILA TAL Y COMO MUESTRAN LAS TRANSPARENCIAS
//    DE LA ASIGNATURA (CON EL PC, FP, LR,....)
//  - EN EL CASO DE LAS VARIABLES LOCALES, SOLO HAY QUE APILARLAS
//    SI NO SE PUEDEN COLOCAR EN UN REGISTRO.
//    SI SE COLOCAN EN UN REGISTRO NO HACE FALTA
//    NI GUARDARLAS EN PILA NI RESERVAR UN ESPACIO EN LA PILA PARA ELLAS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////////////////////////////////////////////////////////////////////////////////
// Devuelve el contenido de la posición indicadas por la fila y columna actual.
// Además informa si la posición es válida y contiene alguna ficha.
// Esto lo hace por referencia (en *posicion_valida)
// Si devuelve un 0 no es válida o está vacia.
int8_t ficha_valida(int8_t tablero[][DIM], int8_t f, int8_t c, int *posicion_valida)
{
    int8_t ficha;

    // ficha = tablero[f][c];
    // no puede accederse a tablero[f][c]
    // ya que algún índice puede ser negativo

    if ((f < DIM) && (f >= 0) && (c < DIM) && (c >= 0) && (tablero[f][c] != CASILLA_VACIA))
    {
        *posicion_valida = 1;
        ficha = tablero[f][c];
    }
    else
    {
        *posicion_valida = 0;
        ficha = CASILLA_VACIA;
    }
    return ficha;
}
// ejemplo de declaración de una función definida externamente:
// extern int patron_volteo_arm(int8_t tablero[][8], int *longitud,int8_t f, int8_t c, int8_t SF, int8_t SC, int8_t color);
////////////////////////////////////////////////////////////////////////////////
// La función patrón volteo comprueba si hay que actualizar una determinada direccion,
// busca el patrón de volteo (n fichas del rival seguidas de una ficha del jugador actual)
// en una dirección determinada
// SF y SC son las cantidades a sumar para movernos en la dirección que toque
// color indica el color de la pieza que se acaba de colocar
// la función devuelve PATRON_ENCONTRADO (1) si encuentra patrón y NO_HAY_PATRON (0) en caso contrario
// FA y CA son la fila y columna a analizar
// longitud es un parámetro por referencia. Sirve para saber la longitud del patrón que se está analizando.
//          Se usa para saber cuantas fichas habría que voltear
int patron_volteo(int8_t tablero[][DIM], int *longitud, int8_t FA, int8_t CA, int8_t SF, int8_t SC, int8_t color)
{
	int posicion_valida; // indica si la posición es valida y contiene una ficha de algún jugador
	int8_t casilla;   // casilla es la casilla que se lee del tablero

	FA = FA + SF;
	CA = CA + SC;
	casilla = ficha_valida(tablero, FA, CA, &posicion_valida);
	while ((posicion_valida == 1) && (casilla != color))
	// mientras la casilla está en el tablero, no está vací­a,
	// y es del color rival seguimos buscando el patron de volteo
	{
		FA = FA + SF;
		CA = CA + SC;
		*longitud = *longitud + 1;
		casilla = ficha_valida(tablero, FA, CA, &posicion_valida);
	}
    // si la ultima posición era válida y la ficha es del jugador actual,
    // entonces hemos encontrado el patrón
	if ((posicion_valida == 1) && (casilla == color) && (*longitud >0))
		return PATRON_ENCONTRADO; // si hay que voltear una ficha o más hemos encontrado el patrón
	else
		return NO_HAY_PATRON; // si no hay que voltear no hay patrón
}
////////////////////////////////////////////////////////////////////////////////
// voltea n fichas en la dirección que toque
// SF y SC son las cantidades a sumar para movernos en la dirección que toque
// color indica el color de la pieza que se acaba de colocar
// FA y CA son la fila y columna a analizar
void voltear(int8_t tablero[][DIM], int8_t FA, int8_t CA, int8_t SF, int8_t SC, int n, int8_t color)
{
    int i;

    for (i = 0; i < n; i++)
    {
        FA = FA + SF;
        CA = CA + SC;
        tablero[FA][CA] = color;
    }
}
////////////////////////////////////////////////////////////////////////////////
// comprueba si hay que actualizar alguna ficha
// no comprueba que el movimiento realizado sea válido
// f y c son la fila y columna a analizar
// int8_t vSF[DIM] = {-1,-1, 0, 1, 1, 1, 0,-1};
// int8_t vSC[DIM] = { 0, 1, 1, 1, 0,-1,-1,-1};
int actualizar_tablero(int8_t tablero[][DIM], int8_t f, int8_t c, int8_t color)
{
    int8_t SF, SC; // cantidades a sumar para movernos en la dirección que toque
    int i, flip, patron;

    for (i = 0; i < DIM; i++) // 0 es Norte, 1 NE, 2 E ...
    {
        SF = vSF[i];
        SC = vSC[i];
        // flip: numero de fichas a voltear
        flip = 0;
        patron = patron_volteo(tablero, &flip, f, c, SF, SC, color);
        //printf("Flip: %d \n", flip);
        if (patron == PATRON_ENCONTRADO )
        {
            voltear(tablero, f, c, SF, SC, flip, color);
        }
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// Recorre todo el tablero comprobando en cada posición si se puede mover
// En caso afirmativo, consulta la puntuación de la posición y si es la mejor
// que se ha encontrado la guarda
// Al acabar escribe el movimiento seleccionado en f y c

// Candidatas
// NO    0
// SI    1
// CASILLA_OCUPADA 2
int elegir_mov(int8_t candidatas[][DIM], int8_t tablero[][DIM], int8_t *f, int8_t *c)
{
    int i, j, k, found;
    int mf = -1; // almacena la fila del mejor movimiento encontrado
    int mc;      // almacena la columna del mejor movimiento encontrado
    int8_t mejor = 0; // almacena el mejor valor encontrado
    int patron, longitud;
    int8_t SF, SC; // cantidades a sumar para movernos en la dirección que toque

    // Recorremos todo el tablero comprobando dónde podemos mover
    // Comparamos la puntuación de los movimientos encontrados y nos quedamos con el mejor
    for (i=0; i<DIM; i++)
    {
        for (j=0; j<DIM; j++)
        {   // indica en qué casillas quizá se pueda mover
            if (candidatas[i][j] == SI)
            {
                if (tablero[i][j] == CASILLA_VACIA)
                {
                    found = 0;
                    k = 0;

                    // en este bucle comprobamos si es un movimiento válido
                    // (es decir si implica voltear en alguna dirección)
                    while ((found == 0) && (k < DIM))
                    {
                        SF = vSF[k];    // k representa la dirección que miramos
                        SC = vSC[k];    // 1 es norte, 2 NE, 3 E ...

                        // nos dice qué hay que voltear en cada dirección
                        longitud = 0;
                        patron = patron_volteo(tablero, &longitud, i, j, SF, SC, FICHA_BLANCA);
                        //  //printf("%d ", patron);
                        if (patron == PATRON_ENCONTRADO)
                        {
                            found = 1;
                            if (tabla_valor[i][j] > mejor)
                            {
                                mf = i;
                                mc = j;
                                mejor = tabla_valor[i][j];
                            }
                        }
                        k++;
                        // si no hemos encontrado nada probamos con la siguiente dirección
                    }
                }
            }
        }
    }
    *f = (int8_t) mf;
    *c = (int8_t) mc;
    // si no se ha encontrado una posición válida devuelve -1
    return mf;
}
////////////////////////////////////////////////////////////////////////////////
// Cuenta el número de fichas de cada color.
// Los guarda en la dirección b (blancas) y n (negras)
void contar(int8_t tablero[][DIM], int *b, int *n)
{
    int i,j;

    *b = 0;
    *n = 0;

    // recorremos todo el tablero contando las fichas de cada color
    for (i=0; i<DIM; i++)
    {
        for (j=0; j<DIM; j++)
        {
            if (tablero[i][j] == FICHA_BLANCA)
            {
                (*b)++;
            }
            else if (tablero[i][j] == FICHA_NEGRA)
            {
                (*n)++;
            }
        }
    }
}

void actualizar_candidatas(int8_t candidatas[][DIM], int8_t f, int8_t c)
{
    // donde ya se ha colocado no se puede volver a colocar
    // En las posiciones alrededor sí
    candidatas[f][c] = CASILLA_OCUPADA;
    if (f > 0)
    {
        if (candidatas[f-1][c] != CASILLA_OCUPADA)
            candidatas[f-1][c] = SI;

        if ((c > 0) && (candidatas[f-1][c-1] != CASILLA_OCUPADA))
            candidatas[f-1][c-1] = SI;

        if ((c < 7) && (candidatas[f-1][c+1] != CASILLA_OCUPADA))
            candidatas[f-1][c+1] = SI;
    }
    if (f < 7)
    {
        if (candidatas[f+1][c] != CASILLA_OCUPADA)
            candidatas[f+1][c] = SI;

        if ((c > 0) && (candidatas[f+1][c-1] != CASILLA_OCUPADA))
            candidatas[f+1][c-1] = SI;

        if ((c < 7) && (candidatas[f+1][c+1] != CASILLA_OCUPADA))
            candidatas[f+1][c+1] = SI;
    }
    if ((c > 0) && (candidatas[f][c-1] != CASILLA_OCUPADA))
        candidatas[f][c-1] = SI;

    if ((c < 7) && (candidatas[f][c+1] != CASILLA_OCUPADA))
        candidatas[f][c+1] = SI;
}

// vSF[DIM] = {-1,-1, 0, 1, 1, 1, 0,-1};
// vSC[DIM]

int movimiento_valido(uint8_t fila, uint8_t col)	{
	int valido = 0;
	int i;
	if(fila >= DIM || col >= DIM)	{
		return 0;	// Si esta fuera de rango
	}
	else	{
		// Recorre las casillas de alrededor
		for(i = 0; i < DIM; i = i+1)	{
			if(tablero[ fila + vSF[i] ][ col + vSC[i] ] != CASILLA_VACIA)	{
				valido = 1;
				break;
			}
		}
		return valido;
	}
}

// Manda el tablero por linea serie
void sendTablero()	{
	int fila, columna;
	for(fila = 0; fila < DIM; fila++)	{
		for(columna = 0; columna < DIM; columna++)	{
			if(tablero[fila][columna] == CASILLA_VACIA) 
				sendchar2(CHAR_VACIA);
			else if(tablero[fila][columna] == FICHA_BLANCA) 
				sendchar2(CHAR_BLANCA);
			else if(tablero[fila][columna] == FICHA_NEGRA) 
				sendchar2(CHAR_NEGRA);
		}
		sendchar2('\n');
	}
}

void mostrarInicioJuego()	{
	sendstring("-----------------------------------\n");
	sendstring("| Bienvenido al juego del reversi |\n");
	sendstring("|---------------------------------|\n");
	sendstring("| Los comandos disponibles son:   |\n");
	sendstring("| • Iniciar partida:   #NEW!      |\n");
	sendstring("| • Pasar:             #PAS!      |\n");
	sendstring("| • Jugada:            #FCS!      |\n");
	sendstring("| • Finalizar partida: #RST!      |\n");
	sendstring("-----------------------------------\n");
}

void leyendaVolverAJugar()	{
	sendstring("-----------------------------------\n|         !Final del juego!       |\n|---------------------------------|\n| Para comenzar una nueva partida |\n| pulse un boton                  |\n-----------------------------------\n");
}


// mostrar_estadisticas muestra las estadísticas de la partida cuando esta finaliza
// motivo = 0 -> IA y User pasan, motivo = 1 -> Llegada de comando RST
void mostrar_estadisticas(int movUser, int movIA, int blancas, int negras, int motivo, int timeIATotal, int segPartida, int minPartida)	{
	char movUserString[10], movIAString[10], blancasString[10], negrasString[10], timeIAString[10], timeSegString[10], timeMinString[10];
	snprintf(movUserString, 10, "%d", movUser);
	snprintf(movIAString, 10, "%d", movIA);
	snprintf(blancasString, 10, "%d", blancas);
	snprintf(negrasString, 10, "%d", negras);
	snprintf(timeIAString, 10, "%d", timeIATotal);
	snprintf(timeSegString, 10, "%d", segPartida);
	snprintf(timeMinString, 10, "%d", minPartida);
	
	
	sendstring("--Partida de reversi finalizada--\n");
	sendstring("--Motivo de fin: ");
	if (motivo == 0)	{
		sendstring("IA y usuario pasan\n");
	}
	else {
		sendstring("Llegada de comando RST\n");
	}
	
	sendstring("-Estadisticas:\n");
	sendstring("--Usuario:\n");
	sendstring("---Movimientos: "); sendstring(movUserString);
	sendstring("\n---Fichas:"); sendstring(negrasString);
	sendstring("\n--IA:\n");
	sendstring("---Movimientos: "); sendstring(movIAString);
	sendstring("\n---Fichas:"); sendstring(blancasString);
	sendstring("\n---Tiempo total IA(ms): "); sendstring(timeIAString);
	
	
	sendstring("\n-----------------------\n");
	sendstring("-Tiempo total partida: \n");
		
	sendstring("--Minutos: "); sendstring(timeMinString);
	sendstring("\n--Segundos: "); sendstring(timeSegString); sendstring("\n");
	
	leyendaVolverAJugar();
}

/*
 * Realiza las acciones oportunas dado un evento 
 * determinado
 * 
 * Devuelve 0 si no necesita IDLE
 * Devuelve 1 si necesita IDLE
 */
int gestion_reversi8(event_t evento, uint32_t aux)
{
	static estado state = INICIAL;
	static int parpadeos_restantes 			= 0;
	static int interrupciones_restantes = 0;
	//numero de movimientos
	static int movUsuario 	= 0;
	static int movIA = 0;
	//tiempo total de IA
	static int time_init 		= 0;
	static int time_final 	= 0;
	static int timeIATotal 	= 0;
	//tiempo inicial
	static uint8_t segIni 	= 0;
	static uint8_t minIni 	= 0;
	
	// 25 * 10ms
	int interr_parpadeo = 25; 
	// Parpadeos totales
	int veces_parpadeo 	= 12; 
	// Por defecto, siempre IDLE
	int power_mode 			= 1;
	
	// la máquina ha conseguido mover o no
	int done;    
	// número de fichas de cada color
  int blancas;
	int negras;
	//tiempo de partida
	uint8_t segFin;
	uint8_t minFin;
	// fila y columna elegidas por la máquina para 
	//  su movimiento
  int8_t f, c;    
	
	
	if(evento == event_comando_acabar)
	{
		state = FINAL;
		
		segFin = rtc_leer_segundos() - segIni;
		minFin = rtc_leer_minutos() - minIni;
		
		contar(tablero, &blancas, &negras);
		mostrar_estadisticas(movUsuario, movIA, blancas, 
												 negras, 1, timeIATotal, segFin, 
												 minFin);
		
		power_mode = 0;
	}
	else {
		switch(state)	
		{
			case INICIAL:
				if(evento == event_eint0 || evento == event_eint1 
					 || evento == event_comando_nueva)
				{
					init_table(tablero, candidatas);
					// Reset movimientos
					movIA = 0;
					movUsuario = 0;
					// Reset timers
					timeIATotal = 0;
					segIni = rtc_leer_segundos();
					minIni = rtc_leer_minutos();
					// Pasa a usuario
					state = USUARIO;
					// Imprime el tablero inicial
					sendTablero();
					// Alimenta el WD
					WD_feed();
				}
				break;
				
				
			case USUARIO:
				// Caso usuario introduce un movimiento (boton)
				if(evento == event_comando_jugada)		
				{
					// Obtiene de aux la fila y columna
					fila = aux >> 8;
					columna = aux & 0xf;	
					
					if(movimiento_valido(fila, columna) == 1)
					{
						tablero[fila][columna] = FICHA_NEGRA;
						usuario_ha_pasado = 0;
						state = PARPADEO;
						parpadeos_restantes = veces_parpadeo;
						interrupciones_restantes = interr_parpadeo;
					}
					WD_feed();
				}
				// Caso usuario pasa
				else if(evento == event_comando_pasar)	
				{
					usuario_ha_pasado = 1;
					state = PARPADEO;
					parpadeos_restantes = veces_parpadeo;
					interrupciones_restantes = interr_parpadeo;
					WD_feed();
				}
				break;
				
				
			case PARPADEO:
				if(evento == event_alarma_periodica)
				{
					interrupciones_restantes--;
					if(interrupciones_restantes == 0)
					{
						interrupciones_restantes = interr_parpadeo;
						parpadeos_restantes--;
						// Solo parpadea si el usuario ha 
						//  introducido movimiento
						if(usuario_ha_pasado == 0)		
						{
							if(tablero[fila][columna] == CASILLA_VACIA)
							{
								tablero[fila][columna] = FICHA_NEGRA;
							}
							else
							{
								tablero[fila][columna] = CASILLA_VACIA;
							}
						}
					}
				}
				
				if(parpadeos_restantes == 0 || 
					 evento == event_eint0)
				{
					if(usuario_ha_pasado == 0)
					{
						movUsuario++;
						tablero[fila][columna] = FICHA_NEGRA;
						actualizar_tablero(tablero, fila, columna, 
															 FICHA_NEGRA);
						actualizar_candidatas(candidatas, fila, 
																  columna);
					}
					// Pasamos a movimiento de la IA
					state = IA;		
				}
				else if(evento == event_eint1)
				{
					state = USUARIO;
					sendTablero();
					WD_feed();
				}
				break;
				
			// Seguramente vendra llamado por una interr 
			//  periodica, pero no se necesita
			case IA:	
				time_init = temporizador_leer();
				done = elegir_mov(candidatas, tablero, &f, &c);
				
				// Usuario pasa y IA pasa
				if (usuario_ha_pasado == 1 && done == -1)			
				{
					state = FINAL;
					sendTablero();
					segFin = rtc_leer_segundos() - segIni;
					minFin = rtc_leer_minutos() - minIni;
					contar(tablero, &blancas, &negras);
					mostrar_estadisticas(movUsuario, movIA, blancas, 
															 negras, 0, timeIATotal, 
															 segFin, minFin);
					power_mode = 0;
				}
				// IA no ha pasado
				else if(done != -1)														
				{
					movIA++;
					tablero[f][c] = FICHA_BLANCA;
					actualizar_tablero(tablero, f, c, FICHA_BLANCA);
					actualizar_candidatas(candidatas, f, c);
					state = USUARIO;
					sendTablero();
					WD_feed();
				}
				// IA ha pasado pero usuario no
				else																					
				{
					state = USUARIO;
					sendTablero();
					WD_feed();
				}
				// Tiempo de la IA
				time_final = temporizador_leer() - time_init;		
				timeIATotal = time_final + timeIATotal;
				
				break;
				
			case FINAL:
				if(evento == event_eint0 ||evento == event_eint1)	{
					state = INICIAL;
					mostrarInicioJuego();
				}
				else {
					power_mode = 0;
				}
				break;
				
			default:
				// PANIC
				while(1);
				// Aqui no debería llegar
				//printf("CASO RARO");		
		}
	}
	return power_mode;
}

/*
 * Procesa un comando siguiendo la sintaxis dada
 * Introduce el evento correspondiente en la cola
 */
void procesar_comando(uint32_t aux)	{
	uint8_t comando[3];
	//Variables auxiliares para comando jugada
	int fila;
	int col;
	int checksum;
	
	comando[0] = aux >> 16;
	comando[1] = (aux & 0xff00) >> 8;
	comando[2] = aux & 0xff;
	
	
	if(comando[0] == 'P' && comando[1] == 'A' 
			&& comando[2] == 'S')
	{
    cola_guardar_eventos(event_comando_pasar,0);
  }
  else if(comando[0] == 'R' && comando[1] == 'S' 
						&& comando[2] == 'T') 
	{
    cola_guardar_eventos(event_comando_acabar,0);
  }
  else if(comando[0] == 'N' && comando[1] == 'E' 
						&& comando[2] == 'W')
	{
    cola_guardar_eventos(event_comando_nueva,0);
  }
	//comando jugada
  else { 
		fila = (comando[0] - '0');
		col = (comando[1] - '0');
		checksum = (comando[2] - '0');
    if( ( ( fila + col ) & 0x7 ) == checksum )
		{
      cola_guardar_eventos(event_comando_jugada,
													 (fila << 8) + col);
    }
  }
}

/*
 * Lee un evento de la cola y realiza la gestión oportuna
 * 
 * Cambia el procesador a modo IDLE o POWER-DOWN
 */
void planificador()	{
	uint32_t auxData;
	uint32_t times;
	event_t evento;
	
	int idle_reversi8 = 1;
	int idle_gestion_boton0 = 0;
	int idle_gestion_boton1 = 0;
	int idle_escribiendo = 0;
	
	mostrarInicioJuego();
	// Interrupcion cada 10 ms
	temporizador_interrupcion_periodica(10);	
	WD_init(60);
	
	// Descomentar para empezar el juego sin pulsacion
	//idle_reversi8 = gestion_reversi8(event_eint0);		
	
	while(1)
	{
		evento = leer_evento(&auxData, &times);
		
		if( evento == event_none )
		{
			idle_escribiendo = uart0_leyendo();
			if( (idle_reversi8 | idle_gestion_boton0 | 
				   idle_gestion_boton1 | idle_escribiendo) != 0)
			{
				PM_idle();
			} 
			else
			{
				PM_power_down();
			}
		}
		
		else if( evento == event_eint0 )
		{
			// Gestionamos el boton 0
			idle_gestion_boton0 = gestion_eint0_pulsado(evento);
			// Gestionamos el juego
			idle_reversi8 = gestion_reversi8(evento,0);
		}
		else if( evento == event_eint1 )
		{
			// Gestionamos el boton 1
			idle_gestion_boton1 = gestion_eint1_pulsado(evento);
			// Gestionamos el juego
			idle_reversi8 = gestion_reversi8(evento,0);
		}
		else if( evento == event_alarma_periodica )
		{
			// Gestionamos el juego
			idle_reversi8 = gestion_reversi8(evento,0);
			gestion_led(evento);
		}
		else if( evento == event_alarma)
		{
			// Gestionamos el boton 0
			idle_gestion_boton0 = gestion_eint0_pulsado(evento);
			// Gestionamos el boton 1
			idle_gestion_boton1 = gestion_eint1_pulsado(evento);
		}
		else if( evento == event_char_leido)
		{
			// Gestionamos el boton 0
			procesar_caracter(auxData);
		}
		else if( evento == event_comando_reconocido )
		{
			procesar_comando(auxData);
		}
		else
		{
			// Gestionamos el juego
			idle_reversi8 = gestion_reversi8(evento, auxData);
		}	
	}
}
