
#ifndef OPERACIONES_H_
#define OPERACIONES_H_
#include "Comunicacion.h"
#include "parser.h"
#include <commons/string.h>
#include <pthread.h>

typedef  struct {
	int32_t id;
	int32_t retardo;
	char* texto;
}t_respuesta;

typedef  struct {
	sock_t* socketPlanificador;
	sock_t* socketMemoria;
}t_sockets;

void* thread_Use(void* );
void* thread_Cpu(void*);
void ejecutar(int32_t, PCB*);
void tipo_Cod_Operacion(int32_t, header_t*);
t_respuesta* mAnsisOp_iniciar(int32_t,PCB*, int32_t);
t_respuesta* mAnsisOp_leer(int32_t, PCB*,int32_t);
t_respuesta* mAnsisOp_escribir(int32_t,PCB*, int32_t,char*);
t_respuesta* mAnsisOp_IO(int32_t,PCB*,int32_t);
t_respuesta* mAnsisOp_finalizar(int32_t, PCB*);
t_respuesta* analizadorLinea(int32_t,PCB*,char* const );
void enviar_Header_ID_Retardo_PCB_Texto (int32_t,int32_t,PCB*,char*,int32_t);
sock_t* getSocketPlanificador(int32_t );
sock_t* getSocketMemoria(int32_t );

#endif /* OPERACIONES_H_ */
