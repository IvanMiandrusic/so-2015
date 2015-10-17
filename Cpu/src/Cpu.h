/*
 * Cpu.h
 *
 *  Created on: 27/8/2015
 *      Author: utnso
 */
/*Header File*/

#ifndef CPU_H_
#define CPU_H_

#include "libsocket.h"
#include "Utils.h"
#include <semaphore.h>
extern sem_t sem_mutex;

typedef struct estructura_configuracion			//estructura que contiene los datos del archivo de configuracion
{
  char* ip_planificador;
  int32_t puerto_planificador;
  char* ip_memoria;
  int32_t puerto_memoria;
  int32_t cantidad_hilos;
  int32_t retardo;
}ProcesoCPU;

typedef struct estructura_PCB			//estructura que contiene los datos del pcb
{
  int32_t PID;
  char* ruta_archivo;
  int32_t estado;
  int32_t siguienteInstruccion;
}PCB;

void crear_estructura_config(char*);
void ifProcessDie();
void inicializoSemaforos();
void crearArchivoDeLog();
void clean();
void envioDie(int32_t);


#endif /* CPU_H_ */
