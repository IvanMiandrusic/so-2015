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
#include <string.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include "Planificador.h"

extern int32_t idParaPCB;

#define COMANDO_SIZE 50

typedef enum comandos {
	CORRER_PATH=1,
	FINALIZAR_PID=2,
	PS=3,
	CPU=4,
	CERRAR_CONSOLA=5,
	HELP=6,
	CLEAR=7
}t_command;

void consola_planificador();
void limpiarBuffer();
int32_t analizar_operacion_asociada(char*);
void correrPath(char*);
void finalizarPID(char*);
void comandoPS();
void usoDeLasCpus();
void mostrarComandos();
char* get_estado_proceso(int32_t);
bool validarComando(char*, char*);
bool validarComandoConParametro(char*, char*);

/** Closures **/
void mostrarEstadoProceso(PCB*);

#endif /* CONSOLA_H_ */
