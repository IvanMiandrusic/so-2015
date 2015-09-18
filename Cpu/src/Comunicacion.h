#ifndef COMUNICACION_H_
#define COMUNICACION_H_

#include <stdlib.h>
#include <stdio.h>
#include "Cpu.h"
#include <commons/temporal.h>

/** Codigos de operacion en los envios hacia el planificador **/
#define NUEVA_CPU 1
#define TERMINO_RAFAGA 2
#define SOLICITUD_IO 3
#define RESULTADO_OK 4
#define RESULTADO_ERROR 5
#define UTILIZACION_CPU 6

/** Codigos de operacion en las recepciones desde el planificador **/
#define ENVIO_QUANTUM 1
#define ENVIO_PCB 2
#define FINALIZAR_PROCESO 3


/**Codigos de operacion en los envios hacia la memoria **/
#define M_INICIAR 1
#define M_LEER 2
#define M_ESCRIBIR 3
#define M_FINALIZAR 4

/** Codigos de operacion en las recepciones desde la memoria **/
#define OK 1
#define ERROR 2
#define CONTENIDO_PAGINA 3

PCB* deserializarPCB(char*);
char* serializarPCB(PCB*);
int32_t obtener_tamanio_pcb(PCB*);
int32_t obtengoSegundos();
#endif /* COMUNICACION_H_ */
