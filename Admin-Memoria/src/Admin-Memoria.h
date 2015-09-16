/*
 * Admin-Memoria.h
 *
 *  Created on: 27/8/2015
 *      Author: utnso
 */
/*Header File*/

#ifndef ADMIN_MEMORIA_H_
#define ADMIN_MEMORIA_H_

#include <string.h>
#include <sys/wait.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "libsocket.h"
#include "Utils.h"
#include "Comunicacion.h"
#include "Estructuras.h"

typedef struct estructura_configuracion			//estructura que contiene los datos del archivo de configuracion
{
  int32_t puerto_escucha;
  char* ip_swap;
  int32_t puerto_swap;
  int32_t maximo_marcos;
  int32_t cantidad_marcos;
  int32_t tamanio_marco;
  int32_t entradas_tlb;
  char* tlb_habilitada;
  int32_t retardo;
  char* algoritmo_reemplazo;
}ProcesoMemoria;

/* VARIABLES GLOBALES */
extern ProcesoMemoria* arch;
extern t_log* loggerInfo;
extern t_log* loggerError;
extern t_log* loggerDebug;
extern sem_t sem_mutex_tlb;
extern sem_t sem_mutex_tabla_paginas;
extern sock_t* socketServidorCpus;
extern sock_t* socketSwap;
extern int32_t* frames;

/** Funciones de configuracion inicial **/
ProcesoMemoria* crear_estructura_config(char*);
void inicializoSemaforos();
void crearArchivoDeLog();
void crear_estructuras_memoria();
void limpiar_estructuras_memoria();

/** Funciones de señales **/
void ifProcessDie();
void ifSigusr1();
void ifSigusr2();
void ifSigpoll();
void dump();
void limpiar_MP();

/** Funciones de operaciones con CPU **/
void procesar_pedido(sock_t*, header_t* );
void iniciar_proceso(sock_t*, t_pedido_cpu*);
void buscar_pagina(t_pagina*);
void finalizarPid(sock_t* );
int32_t limpiar_Informacion_PID(int32_t );
void leer_pagina(sock_t* , header_t* );
void escribirPagina(sock_t*, header_t*);
t_resultado_busqueda buscar_y_escribir_pagina(t_pagina* );
t_resultado_busqueda TLB_buscar_pagina_escribir(t_pagina*);
t_resultado_busqueda buscar_pagina_a_escribir_tabla_paginas(t_pagina*);
#endif /* ADMIN_MEMORIA_H_ */
