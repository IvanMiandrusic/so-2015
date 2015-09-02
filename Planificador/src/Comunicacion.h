/*
 * Comunicacion.h
 *
 *  Created on: 30/08/2015
 *      Author: federico
 */

#ifndef COMUNICACION_H_
#define COMUNICACION_H_

#include <stdlib.h>
#include <stdio.h>
#include "Planificador.h"

/** Codigos de operacion en los envios hacia la cpu **/
#define ENVIO_QUANTUM 1
#define ENVIO_PCB 2
#define FINALIZAR_PROCESO 3

/** Codigos de operacion en las recepciones desde la cpu **/
#define NUEVA_CPU 1
#define TERMINO_RAFAGA 2
#define INSTRUCCION_IO 3
#define RESULTADO_OK 4
#define RESULTADO_ERROR 5
#define RESPUESTA_UTILIZACION_CPU 6

void deserializar_pedido(char* pedidoSerializado); //tipo a definir
char* serializarPCB(PCB* unPCB);

#endif /* COMUNICACION_H_ */
