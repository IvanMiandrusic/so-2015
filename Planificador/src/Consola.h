/*
 * Consola.h
 *
 *  Created on: 28/8/2015
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include "Planificador.h"

#define COMANDO_SIZE 50

typedef enum comandos {
	CORRER_PATH=1,
	FINALIZAR_PID=2,
	PS=3,
	CPU=4,
	CERRAR_CONSOLA=5,
	HELP=6
}t_command;

void consola_planificador();
void limpiarBuffer();
int32_t analizar_operacion_asociada(char*);
void correrPath();
void finalizarPID(int32_t);
void comandoPS();
void usoDeLasCpus();
void mostrarEstadoProceso(PCB*);
void mostrarComandos();

#endif /* CONSOLA_H_ */
