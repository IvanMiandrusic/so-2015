/*
 * Operaciones.h
 *
 *  Created on: 28/8/2015
 *      Author: utnso
 */

#ifndef OPERACIONES_H_
#define OPERACIONES_H_
#include "Comunicacion.h"
#include "parser.h"

typedef  struct {
	int id;
	char* texto;
}t_respuesta;

void* thread_Cpu(void*);

void* iniciar (int32_t);
void* leer (int32_t);
void* escribir (int32_t, char* );
void* entrada_salida (int32_t);
void* finalizar ();
void tipo_Cod_Operacion(int32_t, header_t*);
void ejecutar_Instrucciones (int32_t, PCB*);
t_respuesta* mAnsisOp_iniciar(int32_t,PCB*, int32_t);
int32_t enviarResultadoAlPlanificador(int32_t, sock_t*);
t_respuesta* mAnsisOp_leer(int32_t, PCB*,int32_t);
t_respuesta* mAnsisOp_escribir(int32_t,PCB*, int32_t,char*);
t_respuesta* mAnsisOp_IO(int32_t,PCB*,int32_t);
t_respuesta* mAnsisOp_finalizar(int32_t, PCB*);

void ejecutar_FIFO(int32_t, PCB*);
void ejecutar_RR(int32_t, PCB*);


#endif /* OPERACIONES_H_ */
