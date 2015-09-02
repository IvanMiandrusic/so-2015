/*
 * Operaciones.h
 *
 *  Created on: 28/8/2015
 *      Author: utnso
 */

#ifndef OPERACIONES_H_
#define OPERACIONES_H_
#include "Comunicacion.h"
void* thread_Cpu(void*);

void* iniciar (int32_t);
void* leer (int32_t);
void* escribir (int32_t, char* );
void* entrada_salida (int32_t);
void* finalizar ();
void tipo_Cod_Operacion (int32_t,int32_t);
void ejecutar_Instrucciones (int32_t, PCB*);
void mAnsisOp_iniciar(int32_t,int32_t);
void mAnsisOp_leer(int32_t,int32_t);
void mAnsisOp_escribir(int32_t,int32_t,char*);
void mAnsisOp_IO(int32_t,int32_t);
void mAnsisOp_finalizar(int32_t);

#endif /* OPERACIONES_H_ */
