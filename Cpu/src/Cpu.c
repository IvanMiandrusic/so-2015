/*Source file */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <commons/config.h>
#include <commons/log.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include "libsocket.h"
#include "Cpu.h"
#include "Operaciones.h"

/* VARIABLES GLOBALES (Definir acá o en el Header)*/
ProcesoCPU* arch;
t_log* loggerInfo;
t_log* loggerError;
t_log* loggerDebug;
t_list* socketsCPU;
int32_t* tiempoInicial;
int32_t* tiempoFinal;
int32_t* tiempoAcumulado;
int32_t* estado;
sem_t sem_mutex;

void crear_estructura_config(char* path)
{
    t_config* archConfig = config_create(path);
    arch = malloc(sizeof(ProcesoCPU));
    arch->ip_planificador = config_get_string_value(archConfig, "IP_PLANIFICADOR");
    arch->puerto_planificador = config_get_int_value(archConfig, "PUERTO_PLANIFICADOR");
    arch->ip_memoria = config_get_string_value(archConfig, "IP_MEMORIA");
    arch->puerto_memoria = config_get_int_value(archConfig, "PUERTO_MEMORIA");
    arch->cantidad_hilos = config_get_int_value(archConfig, "CANTIDAD_HILOS");
    arch->retardo = config_get_int_value(archConfig, "RETARDO");
}

void clean(){
	free(tiempoAcumulado);
	free(tiempoFinal);
	free(tiempoInicial);
	free(estado);
	list_destroy_and_destroy_elements(socketsCPU,free);
	free(arch);
	log_destroy(loggerDebug);
	log_destroy(loggerError);
	log_destroy(loggerInfo);
}

/* Función que es llamada cuando ctrl+c */
void ifProcessDie(){

	/** Si el proceso muere, debo avisar al planificador que me limpie de su lista de cpus **/
	log_info(loggerInfo, ANSI_COLOR_BOLDBLUE "Se dara de baja el proceso CPU"ANSI_COLOR_RESET);
	int32_t i;
	for(i=0; i < arch->cantidad_hilos; i++){ envioDie(i);}
	clean();
	exit(0);
}

void envioDie (int32_t id){
		header_t* header_control_c = _create_header(CPU_DIE, sizeof(int32_t));
		sock_t* socketPlanificador = getSocketPlanificador(id);
		int32_t enviado = _send_header(socketPlanificador, header_control_c);
		if(enviado == ERROR_OPERATION)return;
		enviado = _send_bytes(socketPlanificador, &id, sizeof(int32_t));
		if(enviado == ERROR_OPERATION)return;
}

/*Función donde se inicializan los semaforos */

void inicializoSemaforos(){
	//Abajo, una inicialización ejemplo, sem_init(&semaforo, flags, valor) con su validacion
	int32_t semMutex = sem_init(&sem_mutex,0,1);
	if(semMutex==-1)log_error(loggerError,"No pudo crearse el semaforo Mutex");
}

/*Se crea un archivo de log donde se registra to-do */

void crearArchivoDeLog() {
	char* pathLog = "Cpu.log";
	char* archLog = "CPU";
	loggerInfo = log_create(pathLog, archLog, 1, LOG_LEVEL_INFO);
	loggerError = log_create(pathLog, archLog, 1, LOG_LEVEL_ERROR);
	loggerDebug = log_create(pathLog, archLog, 1, LOG_LEVEL_DEBUG);
}


int main(int argc, char** argv) {

	if(argc!=2) {
		printf(ANSI_COLOR_BOLDRED "Cantidad erronea de parametros. Este proceso recibe un parametro \n" ANSI_COLOR_RESET);
		return EXIT_FAILURE;
	}

	/*Tratamiento del ctrl+c en el proceso */
	if(signal(SIGINT, ifProcessDie) == SIG_ERR ) log_error(loggerError, ANSI_COLOR_RED "Error con la señal SIGINT" ANSI_COLOR_RESET);

	/** Chequeamos si la conexion se rompe del lado memoria **/
	if (signal(SIGPIPE, ifProcessDie) == SIG_ERR) log_error(loggerError, ANSI_COLOR_RED "Error con la señal SIGPIPE" ANSI_COLOR_RESET);

	/*Se genera el struct con los datos del archivo de config.- */
	char* path = argv[1];
	crear_estructura_config(path);
	socketsCPU=list_create();
	tiempoInicial=malloc(sizeof(int32_t)* (arch->cantidad_hilos));
	tiempoFinal=malloc(sizeof(int32_t)* (arch->cantidad_hilos));
	tiempoAcumulado=malloc(sizeof(int32_t)* (arch->cantidad_hilos));
	estado=malloc(sizeof(int32_t)* (arch->cantidad_hilos));

	/*Se genera el archivo de log, to-do lo que sale por pantalla */
	crearArchivoDeLog();
	log_info(loggerInfo, "Se cargaron correctamente los parametros de configuración");

	/*Se inicializan todos los semaforos necesarios */
	inicializoSemaforos();

	/*Creo los hilos necesarios para la ejecución*/
	int32_t i;
	int32_t cantidad_hilos=arch->cantidad_hilos;
    pthread_t CPUthreads[cantidad_hilos];

    for(i=0;i<cantidad_hilos;i++){
    	t_sockets* sockets=malloc(sizeof(t_sockets));
    	sockets->socketPlanificador= malloc(sizeof(sock_t));
    	sockets->socketMemoria= malloc(sizeof(sock_t));
    	list_add_in_index(socketsCPU, i, sockets);
    	int resultado = pthread_create(&CPUthreads[i], NULL, thread_Cpu, (void*) i );
    	if (resultado != 0) {
    		log_error(loggerError, ANSI_COLOR_RED "Error al crear el hilo de ejecucion CPU número: %d"ANSI_COLOR_RESET, i);
    		abort();
    	}else{log_info(loggerInfo, ANSI_COLOR_BOLDGREEN "Se creo exitosamente la CPU número: %d"ANSI_COLOR_RESET, i);}

    }
	for(i=0;i<cantidad_hilos;i++){ // espera a que terminen los hilos para terminar el proceso
		pthread_join(CPUthreads[i],NULL);
	}
	clean();
	return EXIT_SUCCESS;
}
