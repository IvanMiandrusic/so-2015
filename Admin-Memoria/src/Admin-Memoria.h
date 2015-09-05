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
#include "Colores.h"
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

/** Funciones de configuracion inicial **/
ProcesoMemoria* crear_estructura_config(char*);
void inicializoSemaforos();
void crearArchivoDeLog();
void dump();

/** Funciones de señales **/
void ifProcessDie();
void ifSigurs1();
void ifSigurs2();

/** Funciones de operaciones con CPU **/
void procesar_pedido(sock_t* socketCpu, header_t* header);
void iniciar_proceso(sock_t*, t_pedido_cpu*);
char* buscar_pagina(t_pagina*);
int32_t borrarEspacio(int32_t);



#endif /* ADMIN_MEMORIA_H_ */
