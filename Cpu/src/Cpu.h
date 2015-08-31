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

typedef struct estructura_configuracion			//estructura que contiene los datos del archivo de configuracion
{
  char* ip_planificador;
  int32_t puerto_planificador;
  char* ip_memoria;
  int32_t puerto_memoria;
  int32_t cantidad_hilos;
  int32_t retardo;
}ProcesoCPU;

ProcesoCPU* crear_estructura_config(char*);
void ifProcessDie();
void inicializoSemaforos();
void crearArchivoDeLog();





#endif /* CPU_H_ */
