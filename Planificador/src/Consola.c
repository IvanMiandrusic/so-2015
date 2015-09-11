/*
 * Consola.c
 *
 *  Created on: 28/8/2015
 *      Author: utnso
 */

#include <stdlib.h>
#include <stdio.h>
#include "Consola.h"
#include "Utils.h"
#include "Planificador.h"

void consola_planificador(){

		char comandoSeleccionado[COMANDO_SIZE];
		bool finalizar;
		int32_t operacionAsociada;

		while(finalizar==false){
		printf(ANSI_COLOR_BOLDCYAN "INGRESE EL COMANDO QUE DESE EJECUTAR: " ANSI_COLOR_RESET);
		scanf("%[^\n]s", comandoSeleccionado);
		printf("\n");

		operacionAsociada = analizar_operacion_asociada(comandoSeleccionado);

		switch (operacionAsociada) {

		case CORRER_PATH: {
			correrPath(comandoSeleccionado);
			break;  //fin correr path
		}
		case FINALIZAR_PID: {
			finalizarPID(comandoSeleccionado);
			break;  //fin finalizar PID
		}
		case PS: {
			comandoPS();
			break; //fin comando ps
		}
		case CPU: {
			usoDeLasCpus();
			break; //fin uso de las Cpus
		}
		case CERRAR_CONSOLA:{
			finalizar=true;
			break;
		}
		case HELP: {
			mostrarComandos();
			break;
		}
		case CLEAR: {
			printf(ENTER ENTER ENTER ENTER ENTER ENTER);
			break;
		}
		default: {
			printf(ANSI_COLOR_BOLDRED "EL COMANDO INGRESADO NO CORRESPONDE CON NINGUNA DE LAS OPERACIONES"ANSI_COLOR_RESET "\n\n");
			printf(ANSI_COLOR_BOLDYELLOW "INGRESE help PARA VER COMANDOS DISPONIBLES." ANSI_COLOR_RESET);
			break;
		}
		}

		limpiarBuffer();
		printf(ENTER);

		}

		printf(ANSI_COLOR_BOLDCYAN "    ╔═══════════════════════════════╗ \n    ║ CONSOLA PLANIFICADOR ENDS     ║ \n    ╚═══════════════════════════════╝" ANSI_COLOR_RESET ENTER);

}

void limpiarBuffer() {
	getchar();
}

int32_t analizar_operacion_asociada(char* comandoSeleccionado) {

	if(string_starts_with(comandoSeleccionado, "correr")) return 1;
	if(string_starts_with(comandoSeleccionado, "finalizar")) return 2;
	if(string_starts_with(comandoSeleccionado, "ps")) return 3;
	if(string_starts_with(comandoSeleccionado, "cpu")) return 4;
	if(string_starts_with(comandoSeleccionado, "cerrar")) return 5;
	if(string_starts_with(comandoSeleccionado, "help")) return 6;
	if(string_starts_with(comandoSeleccionado, "clear")) return 7;
	return 8;
}

void correrPath(char* comando) {

	char* filePath = strstr(comando, " ") + 1;

	printf(ANSI_COLOR_BOLDYELLOW "Se procedera a iniciar un mProc nuevo con mCod %s" ANSI_COLOR_RESET "\n", filePath);

	if (string_equals_ignore_case(arch->algoritmo, "FIFO")) {
		//se hace esto si es fifo
		administrarPath(filePath);
		asignarPCBaCPU();
	} else if (string_equals_ignore_case(arch->algoritmo, "RR")) {
		//Se hace esto si es RR
		administrarPath(filePath);
	}
}

void finalizarPID(char* comando) {

	char* pid_comando = strstr(comando, " ") + 1;
	int32_t PID = atoi(pid_comando);

	printf(ANSI_COLOR_BOLDYELLOW "Se procedera a finalizar un mProc con id %d" ANSI_COLOR_RESET "\n", PID);

	/** Todo manda a finalizar proceso **/

}

char* get_estado_proceso(int32_t estado) {

	if(estado == LISTO) return "Listo";
	if(estado == EJECUCION) return "En ejecucion";
	if(estado == BLOQUEADO) return "Bloqueado";
	if(estado == FINALIZADO_OK) return "Finalizado_OK";
	if(estado == FINALIZADO_ERROR) return "Finalizado_ERROR";

	return NULL;
}

void comandoPS() {

	void mostrarEstadoProceso(PCB* unPcb) {

		char* proceso = string_new();
		string_append_with_format(&proceso, "mProc %d: %s -> %s\n", unPcb->PID,
				unPcb->ruta_archivo, get_estado_proceso(unPcb->estado));
		printf(ANSI_COLOR_BOLDGREEN "%s \n" ANSI_COLOR_RESET, proceso);
	}

	printf(ANSI_COLOR_BOLDYELLOW "Se procedera a mostrar el estado de los mProc en el sistema" ANSI_COLOR_RESET "\n");

	list_iterate(colaListos, mostrarEstadoProceso);
	list_iterate(colaBlock, mostrarEstadoProceso);
	list_iterate(colaExec, mostrarEstadoProceso);
	list_iterate(colaFinalizados, mostrarEstadoProceso);

}

void mostrarEstadoCpus() {

	void mostrar_rendimiento(CPU_t* cpu) {
		char* cpu_a_mostrar = string_new();
		string_append_with_format(&cpu_a_mostrar, "CPU %d: %d%%\n", cpu->ID, cpu->rendimiento);
		printf(ANSI_COLOR_BOLDGREEN "%s \n" ANSI_COLOR_RESET, cpu_a_mostrar);
	}

	list_iterate(colaCPUs, mostrar_rendimiento);
}

void usoDeLasCpus() {

	printf(ANSI_COLOR_BOLDYELLOW "Se procedera a mostrar el porcentaje de uso de las cpus en el último minuto" ANSI_COLOR_RESET "\n");

	mostrarEstadoCpus();
}

void mostrarComandos() {

	printf(ANSI_COLOR_BOLDYELLOW "Los comandos disponibles son: \n\n");
	printf("correr PATH \t\t PATH es la ruta relativa al programa mCod \n");
	printf("finalizar PID \t\t PID es el numero de proceso mProc \n");
	printf("ps \t\t\t Lista los mProc y su estado actual \n");
	printf("cpu \t\t\t Listar las cpus con su utilizacion \n");
	printf("clear \t\t\t Limpia la consola \n");
	printf("cerrar consola \t\t Cerrar la consola" ANSI_COLOR_RESET);
}

