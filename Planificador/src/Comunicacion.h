
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
#define UTILIZACION_CPU 6
#define CPU_DIE 7

PCB* deserializarPCB(char* pedidoSerializado); //tipo a definir
char* serializarPCB(PCB* unPCB);

#endif /* COMUNICACION_H_ */
