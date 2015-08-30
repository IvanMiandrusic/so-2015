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

void admin_consola();
int32_t analizar_operacion_asociada(char*);
void correrPath();
void finalizarPID(int32_t);
void comandoPS();
void usoDeLasCpus();
void mostrarEstadoProceso(PCB*);
void mostrarComandos();

#endif /* CONSOLA_H_ */
