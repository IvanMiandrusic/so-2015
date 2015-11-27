/*Source file */

#include <stdio.h>
#include <stdlib.h>
#include "Utils.h"
#include "Planificador.h"
#include "Consola.h"
#include "Comunicacion.h"
#include <pthread.h>
#include <unistd.h>

#define TAM_FINALIZAR -10

/** TAD para PLANIFICADOR **/
ProcesoPlanificador* arch;
/** Loggers **/
t_log* loggerInfo;
t_log* loggerError;
t_log* loggerDebug;
/** TADs **/
t_list* colaListos;
t_list* colaBlock;
t_list* colaExec;
t_list* colaFinalizados;
t_list* colaCPUs;
t_list* colaMetricas;
t_list* retardos_PCB;
/** ID para los PCB **/
int32_t idParaPCB = 0;
/** SOCKET SERVIDOR **/
sock_t* socketServidor;
/** Semaforos **/
sem_t semMutex_colaBlock;
sem_t semMutex_colaExec;
sem_t semMutex_colaFinalizados;
sem_t semMutex_colaListos;
sem_t semMutex_colaCPUs;
sem_t semMutex_colaMetricas;
sem_t sem_list_retardos;
sem_t sem_io;

void crear_estructura_config(char* path) {
	t_config* archConfig = config_create(path);
	arch = malloc(sizeof(ProcesoPlanificador));
	arch->puerto_escucha = config_get_int_value(archConfig, "PUERTO_ESCUCHA");
	arch->algoritmo = config_get_string_value(archConfig,
			"ALGORITMO_PLANIFICACION");
	arch->quantum = config_get_int_value(archConfig, "QUANTUM");
}

/* Función que es llamada cuando ctrl+c */
void ifProcessDie() {
	log_info(loggerInfo, ANSI_COLOR_BOLDBLUE "Se dara de baja el proceso PLANIFICADOR"ANSI_COLOR_RESET);
	cleanAll();
	exit(EXIT_FAILURE);
}

/*Función donde se inicializan los semaforos */

void inicializoSemaforos() {

	int32_t semMutexColaListos = sem_init(&semMutex_colaListos, 0, 1);
	if (semMutexColaListos == -1)
		log_error(loggerError, ANSI_COLOR_BOLDRED
				"No pudo crearse el semaforo Mutex de Cola listos" ANSI_COLOR_RESET);

	int32_t semMutexColaBlock = sem_init(&semMutex_colaBlock, 0, 1);
	if (semMutexColaBlock == -1)
		log_error(loggerError, ANSI_COLOR_BOLDRED
				"No pudo crearse el semaforo Mutex de Cola Block" ANSI_COLOR_RESET);

	int32_t semMutexColaExec = sem_init(&semMutex_colaExec, 0, 1);
	if (semMutexColaExec == -1)
		log_error(loggerError, ANSI_COLOR_BOLDRED
				"No pudo crearse el semaforo Mutex de Cola Exec" ANSI_COLOR_RESET);

	int32_t semMutexColaFinalizados = sem_init(&semMutex_colaFinalizados, 0, 1);
	if (semMutexColaFinalizados == -1)
		log_error(loggerError, ANSI_COLOR_BOLDRED
				"No pudo crearse el semaforo Mutex de Cola Finalizados" ANSI_COLOR_RESET);

	int32_t semMutexCpu = sem_init(&semMutex_colaCPUs, 0, 1);
	if (semMutexCpu == -1)
		log_error(loggerError, ANSI_COLOR_BOLDRED
				"No pudo crearse el semaforo Mutex de Colas CPU" ANSI_COLOR_RESET);

	int32_t semMutexAFinalizar = sem_init(&semMutex_colaMetricas, 0, 1);
	if (semMutexAFinalizar == -1)
		log_error(loggerError, ANSI_COLOR_BOLDRED
				"No pudo crearse el semaforo Mutex de Colas a Finalizar" ANSI_COLOR_RESET);

	int32_t semMutexRetardos = sem_init(&sem_list_retardos, 0, 1);
	if (semMutexRetardos == -1)
		log_error(loggerError, ANSI_COLOR_BOLDRED
				"No pudo crearse el semaforo Mutex de lista retardos" ANSI_COLOR_RESET);

	int32_t semIO = sem_init(&sem_io, 0, 0);
	if (semIO == -1)
		log_error(loggerError, ANSI_COLOR_BOLDRED
				"No pudo crearse el semaforo para manejar la IO" ANSI_COLOR_RESET);
}

/*Se crea un archivo de log donde se registra to-do */

void crearArchivoDeLog() {
	char* pathLog = "Planificador.log";
	char* archLog = "Planificador";
	loggerInfo = log_create(pathLog, archLog, false, LOG_LEVEL_INFO);
	loggerError = log_create(pathLog, archLog, false, LOG_LEVEL_ERROR);
	loggerDebug = log_create(pathLog, archLog, false, LOG_LEVEL_DEBUG);
}

PCB* generarPCB(int32_t PID, char* rutaArchivo) {
	PCB* unPCB = malloc(sizeof(PCB));
	unPCB->PID = PID;
	unPCB->estado = LISTO;
	unPCB->ruta_archivo = string_duplicate(rutaArchivo);
	unPCB->siguienteInstruccion = 0;
	return unPCB;

}

CPU_t* generarCPU(int32_t ID, sock_t* socketCPU) {
	CPU_t* unaCPU = malloc(sizeof(CPU_t));
	unaCPU->ID = ID;
	unaCPU->socketCPU = malloc(sizeof(sock_t));
	unaCPU->socketCPU->fd = socketCPU->fd;
	unaCPU->estado = LIBRE;
	unaCPU->pcbID = 0;
	unaCPU->rendimiento = 0;
	return unaCPU;
}

void creoEstructurasDeManejo() {
	colaListos = list_create();
	colaBlock = list_create();
	colaExec = list_create();
	colaFinalizados = list_create();
	colaMetricas = list_create();
	colaCPUs = list_create();
	retardos_PCB = list_create();

}

void cleanAll() {
	list_destroy_and_destroy_elements(colaListos, free);
	list_destroy_and_destroy_elements(colaExec, free);
	list_destroy_and_destroy_elements(colaBlock, free);
	list_destroy_and_destroy_elements(colaFinalizados, free);
	list_destroy_and_destroy_elements(retardos_PCB, free);
	list_destroy_and_destroy_elements(colaCPUs, free);
	list_destroy_and_destroy_elements(colaMetricas, free);
	sem_destroy(&semMutex_colaBlock);
	sem_destroy(&semMutex_colaExec);
	sem_destroy(&semMutex_colaFinalizados);
	sem_destroy(&semMutex_colaListos);
	sem_destroy(&semMutex_colaCPUs);
	sem_destroy(&semMutex_colaMetricas);
	sem_destroy(&sem_list_retardos);
	sem_destroy(&sem_io);
	free(arch);
	log_destroy(loggerInfo);
	log_destroy(loggerError);
	log_destroy(loggerDebug);
	clean_socket(socketServidor);
}

void administrarPath(char* filePath) {

	idParaPCB++;
	PCB* unPCB = generarPCB(idParaPCB, filePath);
	Metricas* metricas = iniciarMetricas(unPCB->PID);
	sem_wait(&semMutex_colaMetricas);
	list_add(colaMetricas, metricas);
	sem_post(&semMutex_colaMetricas);
	agregarPcbAColaListos(unPCB);
	log_info(loggerInfo,
			ANSI_COLOR_YELLOW"El proceso con id %d y mCod %s esta listo para ejecutarse"ANSI_COLOR_RESET,
			unPCB->PID, unPCB->ruta_archivo);
	return;

}

Metricas* iniciarMetricas(int32_t PID) {

	Metricas* misMetricas = malloc(sizeof(Metricas));
	misMetricas->PID = PID;
	misMetricas->finalizado = 0;
	misMetricas->hora_de_Creacion = get_actual_time_integer();
	misMetricas->hora_ejecucion = 0;
	misMetricas->hora_listo = get_actual_time_integer();
	misMetricas->horasEjec = 0;
	misMetricas->minEjec = 0;
	misMetricas->segEjec = 0;
	misMetricas->horasEsp = 0;
	misMetricas->minEsp = 0;
	misMetricas->segEsp = 0;
	misMetricas->horasRsp = 0;
	misMetricas->minRsp = 0;
	misMetricas->seg_Resp = 0;
	misMetricas->correspondeCalculoTResp = true;
	return misMetricas;
}

//Funcion del hilo servidor de conexiones
void servidor_conexiones() {

	connection_pool_server_listener(socketServidor, procesarPedido);
}

void procesarPedido(sock_t* socketCPU, header_t* header) {

	int32_t recibido;
	int32_t cpu_id;

	/** Recibo el cpu_id desde la CPU **/
	recibido = _receive_bytes(socketCPU, &(cpu_id), sizeof(int32_t));
	if (recibido == ERROR_OPERATION) return;

	switch (get_operation_code(header)) {
	case NUEVA_CPU: {
		/** Genero un nuevo cpu_t **/
		CPU_t* nuevaCPU = generarCPU(cpu_id, socketCPU);
		agregarColaCPUs(nuevaCPU);
		log_info(loggerDebug,
				ANSI_COLOR_YELLOW"Se conecto una CPU nueva con id %d y socket %d"ANSI_COLOR_RESET,
				nuevaCPU->ID, nuevaCPU->socketCPU->fd);
		/** Envio el quantum a la CPU **/
		header_t* header_quantum = _create_header(ENVIO_QUANTUM,sizeof(int32_t));
		int32_t enviado = _send_header(socketCPU, header_quantum);
		if (enviado == ERROR_OPERATION)return;
		enviado = _send_bytes(socketCPU, &(arch->quantum), sizeof(int32_t));
		if (enviado == ERROR_OPERATION)return;

		free(header_quantum);
		asignarPCBaCPU();
		break;
	}
	case TERMINO_RAFAGA: {
		recibirOperacion(socketCPU, cpu_id, TERMINO_RAFAGA);
		break;
	}
	case INSTRUCCION_IO: {
		recibirOperacion(socketCPU, cpu_id, INSTRUCCION_IO);
		break;
	}
	case RESULTADO_ERROR: {
		recibirOperacion(socketCPU, cpu_id, RESULTADO_ERROR);
		break;
	}
	case RESULTADO_OK: {
		recibirOperacion(socketCPU, cpu_id, RESULTADO_OK);
		break;
	}
	case UTILIZACION_CPU: {
		int32_t tiempo_uso_cpu;
		recibido = _receive_bytes(socketCPU, &tiempo_uso_cpu, sizeof(int32_t));
		if (recibido == ERROR_OPERATION) {
			log_error(loggerError,ANSI_COLOR_BOLDRED "Fallo al recibir tiempo_uso_cpu (Utilizacion_CPU)"ANSI_COLOR_RESET);
			return;
		}
		/** Actualizar rendimiento de la CPU **/
		bool findCpu(void* parametro) {
			CPU_t* unaCpu = (CPU_t*) parametro;
			return unaCpu->ID == cpu_id;
		}
		CPU_t* cpu = list_find(colaCPUs, findCpu);
		cpu->rendimiento = tiempo_uso_cpu;
		break;
	}
	case CPU_DIE: {
		limpiarCpuById(cpu_id);
		log_info(loggerInfo,ANSI_COLOR_BOLDMAGENTA "La CPU con ID: %d se Desconecto"ANSI_COLOR_RESET, cpu_id);
		break;
	}
	default: {
		break;
	}
	}
	free(header);
}

void limpiarCpuById(int32_t cpu_id) {

	bool findCpu(void* parametro) {
		CPU_t* unaCpu = (CPU_t*) parametro;
		return unaCpu->ID == cpu_id;
	}
	CPU_t* cpu = list_remove_by_condition(colaCPUs, findCpu);
	if (cpu->estado == OCUPADO)
		finalizarPCB(cpu->pcbID, RESULTADO_ERROR);
	free(cpu);
}

void recibirOperacion(sock_t* socketCPU, int32_t cpu_id, int32_t cod_Operacion) {
	int32_t tiempo;
	if (cod_Operacion == INSTRUCCION_IO) {
		//recibe el tiempo de I/O
		int32_t recibido_tiempo = _receive_bytes(socketCPU, &(tiempo), sizeof(int32_t));
		if (recibido_tiempo == ERROR_OPERATION) return;
	}

	/** tamaño pcb **/
	int32_t tamanio_pcb;

	int32_t recibido = _receive_bytes(socketCPU, &tamanio_pcb, sizeof(int32_t));
	if (recibido == ERROR_OPERATION) return;

	char* pcb_serializado = malloc(tamanio_pcb);
	recibido = _receive_bytes(socketCPU, pcb_serializado, tamanio_pcb);
	if (recibido == ERROR_OPERATION) return;

	PCB* pcb = deserializarPCB(pcb_serializado);
	free(pcb_serializado);

	/** recibo el char* de resultados **/
	int32_t tamanio_resultado_operaciones;
	recibido = _receive_bytes(socketCPU, &tamanio_resultado_operaciones, sizeof(int32_t));
	if (recibido == ERROR_OPERATION) return;

	char* resultado_operaciones = malloc(1+tamanio_resultado_operaciones);
	recibido = _receive_bytes(socketCPU, resultado_operaciones,	tamanio_resultado_operaciones);
	if (recibido == ERROR_OPERATION) return;
	resultado_operaciones[tamanio_resultado_operaciones]='\0';

	/** Loggeo resultado operaciones **/
	log_info(loggerInfo, ANSI_COLOR_BOLDMAGENTA "Recibido el proceso id: %d, ejecuto: %s" ANSI_COLOR_RESET, pcb->PID, resultado_operaciones);
//	Todo calcularMetrica(pcb->PID, TIEMPO_EXEC);

	/** Operar la respuesta **/
	if (cod_Operacion == INSTRUCCION_IO) {
		log_info(loggerInfo, ANSI_COLOR_YELLOW"El proceso: %d pidio una I/O"ANSI_COLOR_RESET, pcb->PID);
		operarIO(cpu_id, tiempo, pcb);
		calcularMetrica(pcb->PID, TIEMPO_RSP);
	}
	if (cod_Operacion == TERMINO_RAFAGA) {
		log_info(loggerInfo, ANSI_COLOR_BOLDWHITE "Termina rafaga del proceso con ruta: %s e ID: %d con respuesta:%s" ANSI_COLOR_RESET,
				pcb->ruta_archivo, pcb->PID, resultado_operaciones);
		liberarCPU(cpu_id);
		sacarDeExec(pcb->PID);
		pcb->estado=LISTO;
		agregarPcbAColaListos(pcb);
		actualizarMetricas(pcb->PID, TIEMPO_ESP);
		asignarPCBaCPU();
	}
	if (cod_Operacion == RESULTADO_ERROR || cod_Operacion == RESULTADO_OK) {
		finalizarPCB(pcb->PID, cod_Operacion);
		liberarCPU(cpu_id);
		asignarPCBaCPU();
	}

	free(resultado_operaciones);
}

void liberarCPU(int32_t cpu_id) {

	bool getCpuByID(void* argumento) {
		CPU_t* unaCPU = (CPU_t*) argumento;
		return unaCPU->ID == cpu_id;
	}
	CPU_t* cpu_encontrada = list_find(colaCPUs, getCpuByID);
	cpu_encontrada->estado = LIBRE;


}

void actualizarMetricas(int32_t pid, int32_t tipo) {

	Metricas* metrica = obtenerMetrica(pid);
	int32_t horaActual = get_actual_time_integer();
	if (tipo == TIEMPO_ESP) {
		metrica->hora_listo = horaActual;
		return;
	}
	if (tipo == TIEMPO_EXEC) {
		metrica->hora_ejecucion = horaActual;
		return;
	}

}

Metricas* obtenerMetrica(int32_t PID){
	bool getIDMetricas(void* parametro) {
		Metricas* metrica= (Metricas*) parametro;
		return metrica->PID == PID;
	}
	Metricas* metrica = list_find(colaMetricas, getIDMetricas);
	return metrica;
}

void mostrarMetricas(int32_t ID){
	Metricas* metrica=obtenerMetrica(ID);
	log_info(loggerInfo,
			ANSI_COLOR_BOLDYELLOW"El tiempo de Respuesta del Proc %d es %d horas, %d minutos, %d segundos"ANSI_COLOR_RESET,
			ID, metrica->horasRsp, metrica->minRsp, metrica->seg_Resp);
	log_info(loggerInfo,
			ANSI_COLOR_BOLDYELLOW"El tiempo de Ejecucion del Proc %d es %d horas, %d minutos, %d segundos"ANSI_COLOR_RESET,
			ID, metrica->horasEjec, metrica->minEjec, metrica->segEjec);
	log_info(loggerInfo,
			ANSI_COLOR_BOLDYELLOW"El tiempo de Espera del Proc %d es %d horas, %d minutos, %d segundos"ANSI_COLOR_RESET,
			ID, metrica->horasEsp, metrica->minEsp, metrica->segEsp);
}

void calcularMetrica(int32_t ID, int32_t tipo) {

	Metricas* metrica=obtenerMetrica(ID);
	t_time* t_horaActual = obtengoTiempo(get_actual_time_integer());
	if (tipo == TIEMPO_EXEC) {
		t_time* t_horaEjec = obtengoTiempo(metrica->hora_ejecucion);
		t_time* resultado = calculoDefinitivo(t_horaActual, t_horaEjec);
		metrica->horasEjec = metrica->horasEjec + resultado->horas;
		metrica->minEjec = metrica->minEjec + resultado->minutos;
		metrica->segEjec = metrica->segEjec + resultado->segundos;
		if (metrica->segEjec > 59) {
			int32_t seg = adaptarHora(metrica->segEjec);
			metrica->minEjec = metrica->segEjec / 60;
			metrica->segEjec = seg;
		}
		if (metrica->minEjec > 59) {
			int32_t min = adaptarHora(metrica->minEjec);
			metrica->horasEjec = metrica->minEjec / 60;
			metrica->minEjec = min;
		}
		free(t_horaEjec);
		free(resultado);
		free(t_horaActual);
		return;
	}
	if (tipo == TIEMPO_ESP) {
		t_time* t_horaEsp = obtengoTiempo(metrica->hora_listo);
		t_time* resultado = calculoDefinitivo(t_horaActual, t_horaEsp);
		metrica->horasEsp = metrica->horasEsp + resultado->horas;
		metrica->minEsp = metrica->minEsp + resultado->minutos;
		metrica->segEsp = metrica->segEsp + resultado->segundos;
		if (metrica->segEsp > 59) {
			int32_t seg = adaptarHora(metrica->segEsp);
			metrica->minEsp = metrica->segEsp / 60;
			metrica->segEsp = seg;
		}
		if (metrica->minEsp > 59) {
			int32_t min = adaptarHora(metrica->minEsp);
			metrica->horasEsp = metrica->minEsp / 60;
			metrica->minEsp = min;
		}
		free(resultado);
		free(t_horaEsp);
		free(t_horaActual);
		return;
	}
	if(tipo==TIEMPO_RSP){
		if(metrica->correspondeCalculoTResp){

		t_time* t_horaCreacion = obtengoTiempo(metrica->hora_de_Creacion);

		t_time* respuestaRSP = calculoDefinitivo(t_horaActual,t_horaCreacion);
		metrica->horasRsp = respuestaRSP->horas;
		metrica->minRsp = respuestaRSP->minutos;
		metrica->seg_Resp = respuestaRSP->segundos;
		metrica->correspondeCalculoTResp = false;
		free(t_horaCreacion);
		free(respuestaRSP);
		free(t_horaActual);
		}
	}
}

int32_t adaptarHora(int32_t hora) {
	double decimal = hora / 60;
	int32_t partEntera = decimal;
	int32_t deciEntero = (decimal - partEntera) * 60;
	return deciEntero;
}

t_time* calculoDefinitivo(t_time* t_horaActual, t_time* t_horaInicial) {

	t_time* definitivo = malloc(sizeof(t_time));

	if (t_horaActual->segundos >= t_horaInicial->segundos)
		definitivo->segundos = t_horaActual->segundos - t_horaInicial->segundos;

	if (t_horaActual->segundos < t_horaInicial->segundos) {
		t_horaActual->minutos -= 1;
		definitivo->segundos = t_horaActual->segundos + 60 - t_horaInicial->segundos;
	}

	if (t_horaActual->minutos >= t_horaInicial->minutos)
		definitivo->minutos = t_horaActual->minutos - t_horaInicial->minutos;

	if (t_horaActual->minutos < t_horaInicial->minutos) {
		t_horaActual->horas -= 1;
		definitivo->minutos = t_horaActual->minutos + 60 - t_horaInicial->minutos;
	}

	if (t_horaActual->horas >= t_horaInicial->horas) definitivo->horas = t_horaActual->horas - t_horaInicial->horas;
	if (t_horaActual->horas < t_horaInicial->horas) definitivo->horas = 24 + t_horaActual->horas - t_horaInicial->horas;

	return definitivo;
}

t_time* obtengoTiempo(int32_t tiempo) {

	t_time* new_time = malloc(sizeof(t_time));
	if (tiempo > 10000) {
		new_time->horas = tiempo / 10000;
	} else {
		new_time->horas = 0;
	}
	if (tiempo > 100) {
		new_time->minutos = (tiempo - new_time->horas * 10000) / 100;
	} else {
		new_time->minutos = 0;
	}
	new_time->segundos = (tiempo - new_time->horas * 10000)	- new_time->minutos * 100;

	return new_time;

}

void agregarPidParaFinalizar(int32_t pcbID) {
	bool getPcbByID(void* unPCB) {
		PCB* pcb = (PCB*) unPCB;
		return pcb->PID == pcbID;
	}
	bool getIDMetricas(void* metrica) {
		Metricas* unaMetrica = (Metricas*) metrica;
		return ((unaMetrica->PID == pcbID) && (unaMetrica->finalizado == 1));
	}
	bool getMetrica(void* metrica) {
		Metricas* unaMetrica = (Metricas*) metrica;
		return ((unaMetrica->PID == pcbID) && (unaMetrica->finalizado == 0));
	}

	if (list_any_satisfy(colaFinalizados, getPcbByID)
			|| list_any_satisfy(colaMetricas, getIDMetricas)) {
		log_info(loggerInfo, ANSI_COLOR_BOLDMAGENTA
		"El PID indicado ya se esta finalizando o ya Finalizo"ANSI_COLOR_RESET);
		printf(
				ANSI_COLOR_BOLDRED "El PID indicado ya se esta finalizando o ya Finalizo \n" ANSI_COLOR_RESET);
		return;
	} else {
		log_info(loggerInfo, "El Proc: %d procedera a Finalizarse", pcbID);
		Metricas* unaMetrica = list_find(colaMetricas, getMetrica);
		unaMetrica->finalizado = 1;

		return;
	}
}

void finalizarPCB(int32_t pcbID, int32_t tipo) {

	bool getPcbByID(void* unPCB) {
		PCB* pcb = (PCB*) unPCB;
		return pcb->PID == pcbID;
	}
	PCB* pcb_found = list_remove_by_condition(colaExec, getPcbByID);
	if (tipo == RESULTADO_OK) {
		pcb_found->estado = FINALIZADO_OK;
		log_info(loggerInfo,
				ANSI_COLOR_BOLDGREEN"Finaliza correctamente el PID:%d y ruta:%s"ANSI_COLOR_RESET,
				pcbID, pcb_found->ruta_archivo);
	}

	if (tipo == RESULTADO_ERROR) {
		pcb_found->estado = FINALIZADO_ERROR;
		log_info(loggerInfo,
				ANSI_COLOR_BOLDRED"Se Finalizo con error el PID: %d y ruta: %s"ANSI_COLOR_RESET,
				pcbID, pcb_found->ruta_archivo);
	}
	calcularMetrica(pcb_found->PID, TIEMPO_RSP);
	calcularMetrica(pcb_found->PID, TIEMPO_EXEC);
	agregarPcbAColaFinalizados(pcb_found);
	mostrarMetricas(pcb_found->PID);
	removerMetrica(pcb_found->PID);
	return;
}

void removerMetrica(int32_t ID) {
	bool getPcbByID(void* unaMetrica) {
		Metricas* metrica = (Metricas*) unaMetrica;
		return (metrica->PID == ID);
	}
	sem_wait(&semMutex_colaMetricas);
	list_remove_by_condition(colaMetricas, getPcbByID);
	sem_post(&semMutex_colaMetricas);
}

void operarIO(int32_t cpu_id, int32_t tiempo, PCB* pcb) {
	bool getPcbByID(void* unPCB) {
		PCB* miPcb = (PCB*) unPCB;
		return miPcb->PID == pcb->PID;
	}
	retardo* retardo_nuevo = malloc(sizeof(retardo));
	retardo_nuevo->ID = pcb->PID;
	retardo_nuevo->retardo = tiempo;

	/** Guardarme el PID y su retardo **/
	sem_wait(&sem_list_retardos);
	list_add(retardos_PCB, retardo_nuevo);
	sem_post(&sem_list_retardos);

	/** PCB de Listos a Bloqueado **/
	PCB* pcb_found = list_remove_by_condition(colaExec, getPcbByID);
	pcb_found->estado = BLOQUEADO;
	pcb_found->siguienteInstruccion = pcb->siguienteInstruccion;
	agregarPcbAColaBlock(pcb_found);

	sem_post(&sem_io);
	/** Cambio estado CPU a LIBRE **/
	liberarCPU(cpu_id);
	/** Se asigna un nuevo PCB a la cpu q se libera **/
	asignarPCBaCPU();
}

/** Funcion del hilo de IO **/

void procesar_IO() {

	while (true) {

		sem_wait(&sem_io);
		PCB* pcb_to_sleep = list_get(colaBlock, 0);

		bool getRetardoByID(void* unPCB) {
			retardo* retard = (retardo*) unPCB;
			return (retard->ID == pcb_to_sleep->PID);
		}

		/** Saco ese PID de la lista de retardos **/
		sem_wait(&sem_list_retardos);
		retardo* tiempo_retardo = list_remove_by_condition(retardos_PCB, getRetardoByID);
		sem_post(&sem_list_retardos);

		/** Simulo la IO del proceso **/
		sleep(tiempo_retardo->retardo);
		log_info(loggerInfo, ANSI_COLOR_YELLOW"Finalizo la I/O del proc: %d"ANSI_COLOR_RESET,tiempo_retardo->ID);
		/** El proceso va a la cola de LISTOS **/
		pcb_to_sleep->estado = LISTO;
		agregarPcbAColaListos(pcb_to_sleep);

		PCB* pcb = list_remove(colaBlock, 0);

		/** Se asigna un nuevo PCB a la cpu q se libera **/
		asignarPCBaCPU();

	}
}

/** Funcion que saca el tamaño de un PCB para enviar **/
int32_t obtener_tamanio_pcb(PCB* pcb) {
	return 4 * sizeof(int32_t) + string_length(pcb->ruta_archivo);
}

bool hay_cpu_libre() {

	bool estaLibre(void* cpu) {
		CPU_t* unaCpu = (CPU_t*) cpu;
		return unaCpu->estado == LIBRE;
	}

	return list_any_satisfy(colaCPUs, estaLibre);
}

void cambiarAUltimaInstruccion(PCB* pcb) {

	FILE* programa = fopen(pcb->ruta_archivo, "r");
	if (programa == NULL) {
		log_error(loggerError, ANSI_COLOR_BOLDRED "Error al intentar abrir la ruta_archivo del pcb con id: %d"ANSI_COLOR_RESET,
				pcb->PID);
		return;
	}
	fseek(programa, TAM_FINALIZAR, SEEK_END);
	int32_t posicion = ftell(programa);
	if (posicion < 0) {
		log_error(loggerError, ANSI_COLOR_BOLDRED "Error en la posicion devuelta por ftell para cambiar la instruccion" ANSI_COLOR_RESET);
	}
	pcb->siguienteInstruccion = posicion;
	fclose(programa);
	return;
}

void asignarPCBaCPU() {
	if (hay_cpu_libre() && !list_is_empty(colaListos)) {
		PCB* pcbAEnviar = list_remove(colaListos, 0);

		bool getPcbByID(void* unaMetrica) {
			Metricas* metrica = (Metricas*) unaMetrica;
			return ((metrica->PID == pcbAEnviar->PID)
					&& (metrica->finalizado == 1));
		}

		if (list_any_satisfy(colaMetricas, getPcbByID)) {
			cambiarAUltimaInstruccion(pcbAEnviar);
			pcbAEnviar->estado = FINALIZANDO;
		} else {
			pcbAEnviar->estado = EJECUCION;
		}

		char* paquete = serializarPCB(pcbAEnviar);
		int32_t tamanio_pcb = obtener_tamanio_pcb(pcbAEnviar);
		enviarPCB(paquete, tamanio_pcb, pcbAEnviar->PID, ENVIO_PCB);
		log_info(loggerInfo,
				ANSI_COLOR_BOLDYELLOW"El Proc con ruta: %s e ID: %d entro en ejecucion mediante un algoritmo %s"ANSI_COLOR_RESET,
				pcbAEnviar->ruta_archivo, pcbAEnviar->PID, arch->algoritmo);

		/** Pongo el PCB en ejecucion **/
		calcularMetrica(pcbAEnviar->PID, TIEMPO_ESP);
//		Todo actualizarMetricas(pcbAEnviar->PID, TIEMPO_EXEC);

		agregarPcbAColaExec(pcbAEnviar);
		mostrarContenidoListas();
		free(paquete);
	}

}

void mostrarContenidoListas() {
	void mostrarElementos(void* parametro) {
		PCB* unPCB = (PCB*) parametro;
		log_info(loggerInfo, ANSI_COLOR_CYAN"ID: %d   Ruta: %s"ANSI_COLOR_RESET,
				unPCB->PID, unPCB->ruta_archivo);
	}
	log_info(loggerInfo,
	ANSI_COLOR_BOLDCYAN"La cola de Listos contiene:"ANSI_COLOR_RESET);
	list_iterate(colaListos, mostrarElementos);

	log_info(loggerInfo,
			ANSI_COLOR_BOLDCYAN"La cola de Ejec contiene:"ANSI_COLOR_RESET);
	list_iterate(colaExec, mostrarElementos);

	log_info(loggerInfo,
			ANSI_COLOR_BOLDCYAN"La cola de Bloqueados contiene:"ANSI_COLOR_RESET);
	list_iterate(colaBlock, mostrarElementos);

	log_info(loggerInfo,
			ANSI_COLOR_BOLDCYAN"La cola de Finalizados contiene:"ANSI_COLOR_RESET);
	list_iterate(colaFinalizados, mostrarElementos);

}

CPU_t* obtener_cpu_libre() {

	bool estaLibre(void* cpu) {
		CPU_t* unaCpu = (CPU_t*) cpu;
		return unaCpu->estado == LIBRE;
	}

	CPU_t* cpu_encontrada = list_find(colaCPUs, estaLibre);
	return cpu_encontrada;
}

void enviarPCB(char* paquete_serializado, int32_t tamanio_pcb, int32_t pcbID,
		int32_t tipo) {

	CPU_t* CPU = obtener_cpu_libre();
	log_debug(loggerDebug, "Cpu libre: %d socket %d y tamanio pcb %d", CPU->ID,
			CPU->socketCPU->fd, tamanio_pcb);

	if (CPU == NULL)
		log_debug(loggerDebug, "No encontre ninguna CPU");


	/** Asigno el pcbID a la CPU_t **/
	CPU->pcbID = pcbID;

	int32_t enviado = send_msg(CPU->socketCPU, tipo, paquete_serializado,
			tamanio_pcb); //le envia el tipo de Envio
	if (enviado == ERROR_OPERATION) {
		log_error(loggerError, ANSI_COLOR_BOLDRED"Fallo en el envio de PCB"ANSI_COLOR_RESET);
		return;
	}
	CPU->estado = OCUPADO;
}

void agregarPcbAColaListos(PCB* pcb) {

	sem_wait(&semMutex_colaListos);
	list_add(colaListos, pcb);
	sem_post(&semMutex_colaListos);

	return;

}

void agregarPcbAColaBlock(PCB* pcb) {

	sem_wait(&semMutex_colaBlock);
	list_add(colaBlock, pcb);
	sem_post(&semMutex_colaBlock);
	return;
}

void agregarPcbAColaExec(PCB* pcb) {

	sem_wait(&semMutex_colaExec);
	list_add(colaExec, pcb);
	sem_post(&semMutex_colaExec);
	return;
}

void agregarPcbAColaFinalizados(PCB* pcb) {

	sem_wait(&semMutex_colaExec);
	list_add(colaFinalizados, pcb);
	sem_post(&semMutex_colaExec);
	return;

}

void agregarColaCPUs(CPU_t* cpu) {

	sem_wait(&semMutex_colaCPUs);
	list_add(colaCPUs, cpu);
	sem_post(&semMutex_colaCPUs);
	return;

}

void sacarDeExec(int32_t pcbID) {

	bool getPcbByID(void* arg) {
		PCB* unPCB = (PCB*) arg;
		return unPCB->PID == pcbID;
	}
	sem_wait(&semMutex_colaExec);
	list_remove_and_destroy_by_condition(colaExec, getPcbByID, free);
	sem_post(&semMutex_colaExec);

}

//----------------------------------// MAIN //------------------------------------------//

int main(int argc, char** argv) {

	if(argc!=2) {
		printf(ANSI_COLOR_BOLDRED "Cantidad erronea de parametros. Este proceso recibe un parametro \n" ANSI_COLOR_RESET);
		return EXIT_FAILURE;
	}

	/*Se genera el archivo de log */
	crearArchivoDeLog();

	/*Tratamiento del ctrl+c en el proceso */
	if (signal(SIGINT, ifProcessDie) == SIG_ERR)
		log_error(loggerError, ANSI_COLOR_BOLDRED "Error con la señal SIGINT" ANSI_COLOR_RESET);

	/*Se genera el struct con los datos del archivo de config.- */
	char* path = argv[1];
	crear_estructura_config(path);

	/*Se inicializan todos los semaforos necesarios */
	inicializoSemaforos();

	/*Se inicializan todos los elementos de gestión necesarios */
	creoEstructurasDeManejo();

	/** Creacion del socket servidor **/
	socketServidor = create_server_socket(arch->puerto_escucha);

	int32_t result = listen_connections(socketServidor);

	if (result == ERROR_OPERATION) {
		log_error(loggerError, ANSI_COLOR_BOLDRED "Error al escuchar conexiones" ANSI_COLOR_RESET);
		exit(EXIT_FAILURE);
	}

	/** Creo hilos CONSOLA y SERVIDOR DE CONEXIONES **/
	int32_t resultado_creacion_hilos;

	pthread_t server_thread;
	resultado_creacion_hilos = pthread_create(&server_thread, NULL,	servidor_conexiones, NULL);
	if (resultado_creacion_hilos != 0) {
		log_error(loggerError, ANSI_COLOR_BOLDRED "Error al crear el hilo del servidor de conexiones" ANSI_COLOR_RESET);
		abort();
	} else {
		log_info(loggerInfo,
				"Se creo exitosamente el hilo de servidor de conexiones");
	}

	pthread_t consola_thread;
	resultado_creacion_hilos = pthread_create(&consola_thread, NULL, consola_planificador, NULL);
	if (resultado_creacion_hilos != 0) {
		log_error(loggerError, ANSI_COLOR_BOLDRED "Error al crear el hilo de la consola" ANSI_COLOR_RESET);
		exit(EXIT_FAILURE);
	} else {
		log_info(loggerInfo, "Se creo exitosamente el hilo de la consola");
	}

	pthread_t io_thread;
	resultado_creacion_hilos = pthread_create(&io_thread, NULL, procesar_IO, NULL);
	if (resultado_creacion_hilos != 0) {
		log_error(loggerError, ANSI_COLOR_BOLDRED "Error al crear el hilo de IO" ANSI_COLOR_RESET);
		exit(EXIT_FAILURE);
	} else {
		log_info(loggerInfo, "Se creo exitosamente el hilo de IO");
	}

	pthread_join(server_thread, NULL);
	pthread_join(consola_thread, NULL);
	pthread_join(io_thread, NULL);

	cleanAll();

	return EXIT_SUCCESS;
}
