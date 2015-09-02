/*
 * Planificador.h
 *
 *  Created on: 27/8/2015
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include "libsocket.h"

typedef enum estados_pcb {
       LISTO=1,
       BLOQUEADO=2,
       EJECUCION=3,
       FINALIZADO=4
}estados_pcb_t;

typedef struct estructura_configuracion			//estructura que contiene los datos del archivo de configuracion
{
  int32_t puerto_escucha;
  char* algoritmo;
  int32_t quantum;

}ProcesoPlanificador;

typedef struct estructura_PCB			//estructura que contiene los datos del pcb
{
  int32_t PID;
  char* ruta_archivo;
  int32_t estado;
  int32_t siguienteInstruccion;
}PCB;

typedef struct estructura_CPU      //estructura que contiene los datos que nos envia cada cpu
{
	int32_t ID;
	sock_t* socketCPU;
}CPU_t;

/* DECLARACION DE VARIABLES GLOBALES */
extern ProcesoPlanificador* arch;
extern t_log* loggerInfo;
extern t_log* loggerError;
extern t_log* loggerDebug;
extern t_list* colaListos;
extern t_list* colaBlock;
extern t_list* colaExec;
extern t_list* colaFinalizados;

ProcesoPlanificador* crear_estructura_config(char*);
void ifProcessDie();
void inicializoSemaforos();
void crearArchivoDeLog();
PCB* generarPCB(int32_t, char*);
void creoEstructurasDeManejo();
void clean();
void consola_planificador();
void administrarPath(char* filePath);
void servidor_conexiones();
void procesarPedido(sock_t* socketCpu, header_t* header);
CPU_t* generarCPU(int32_t ID, sock_t* socketCPU);



#endif /* PLANIFICADOR_H_ */
