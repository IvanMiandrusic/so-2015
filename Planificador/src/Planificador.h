/*
 * Planificador.h
 *
 *  Created on: 27/8/2015
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <commons/string.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/temporal.h>
#include "libsocket.h"



typedef enum estados_pcb {
       LISTO=1,
       EJECUCION=2,
       BLOQUEADO=3,
       FINALIZADO_OK=4,
       FINALIZADO_ERROR=5,
	   FINALIZANDO=6 //agregue este estado, que representa que al pcb lo esta finalizando la cpu
}estados_pcb_t;

typedef struct tiempo {
	int32_t horas;
	int32_t minutos;
	int32_t segundos;

}t_time;

typedef enum tipos_metricas{
		MOSTRAR_METRICAS=1,
		TIEMPO_EXEC=2,
		TIEMPO_ESP=3,
		TIEMPO_RSP=4

}tipos_metricas;

typedef enum estados_cpu {

	LIBRE=1,
	OCUPADO=2
}t_estado_cpu;

typedef struct estructura_metricas{

	int32_t PID;
	int32_t finalizado;
	int32_t hora_de_Creacion;
	int32_t hora_ejecucion;
	int32_t hora_listo;
	int32_t horasEjec;
	int32_t minEjec;
	int32_t segEjec;
	int32_t horasEsp;
	int32_t minEsp;
	int32_t segEsp;
	int32_t horasRsp;
	int32_t minRsp;
	int32_t seg_Resp;
	int32_t resp;

}Metricas;

typedef struct estructura_configuracion			//estructura que contiene los datos del archivo de configuracion
{
  int32_t puerto_escucha;
  char* algoritmo;
  int32_t quantum;

}ProcesoPlanificador;

typedef struct estructura_PCB			//estructura que contiene los datos del pcb
{
  int32_t PID;
  char* ruta_archivo;
  int32_t estado;
  int32_t siguienteInstruccion;
  int32_t horaInicial;
}PCB;

typedef struct estructura_CPU      //estructura que contiene los datos que nos envia cada cpu
{
	int32_t ID;
	sock_t* socketCPU;
	t_estado_cpu estado;
	int32_t pcbID;
	int32_t rendimiento;
}CPU_t;

typedef struct estructura_retardo     //estructura que contiene los datos del retardo
{
	int32_t ID;
	int32_t retardo;
}retardo;



/* DECLARACION DE VARIABLES GLOBALES */
extern ProcesoPlanificador* arch;
extern t_log* loggerInfo;
extern t_log* loggerError;
extern t_log* loggerDebug;
extern t_list* colaListos;
extern t_list* colaBlock;
extern t_list* colaExec;
extern t_list* colaFinalizados;
extern t_list* colaCPUs;
extern t_list* retardos_PCB;

ProcesoPlanificador* crear_estructura_config(char*);
void ifProcessDie();
void inicializoSemaforos();
void crearArchivoDeLog();
PCB* generarPCB(int32_t, char*);
void creoEstructurasDeManejo();
void cleanAll();
void consola_planificador();
void administrarPath(char* );
void servidor_conexiones();
void procesarPedido(sock_t* , header_t* );
CPU_t* generarCPU(int32_t , sock_t* );
void asignarPCBaCPU();
int32_t obtener_tamanio_pcb(PCB* );
void enviarPCB(char* , int32_t , int32_t , int32_t );
CPU_t* obtener_cpu_libre();
bool hay_cpu_libre();
void operarIO(int32_t , int32_t , PCB* );
void procesar_IO();
void agregarPcbAColaListos(PCB* );
void agregarPcbAColaBlock(PCB* );
void agregarPcbAColaFinalizados(PCB* );
void agregarPcbAColaExec(PCB*);
void agregarColaCPUs(CPU_t*);
void finalizarPCB(int32_t , int32_t );
void liberarCPU(int32_t );
void agregarPidParaFinalizar(int32_t );
void cambiarAUltimaInstruccion(PCB* );
void recibirOperacion(sock_t*, int32_t, int32_t);
void sacarDeExec(int32_t );
Metricas* iniciarMetricas(int32_t );
void calcularMetrica(int32_t , int32_t );
void actualizarMetricas(int32_t , int32_t );
void removerMetrica(int32_t );
t_time* obtengoTiempo(int32_t );
t_time* calculoDefinitivo(t_time* , t_time* );
int32_t adaptarHora(int32_t );
void limpiarCpuById(int32_t );
void mostrarContenidoListas();

/** Closures **/
bool estaLibre(CPU_t* cpu);



#endif /* PLANIFICADOR_H_ */
