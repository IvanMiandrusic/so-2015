/*
 * Consola.c
 *
 *  Created on: 28/8/2015
 *      Author: utnso
 */

#include <stdlib.h>
#include <stdio.h>
#include "Consola.h"
#include "Colores.h"
#include "Planificador.h"

void consola_planificador() {

	admin_consola();
}


void admin_consola(){

		char comandoSeleccionado[COMANDO_SIZE];
		bool finalizar;
		int32_t operacionAsociada;

		while(finalizar==false){
		printf(ANSI_COLOR_BOLDCYAN "INGRESE EL COMANDO QUE DESE EJECUTAR: " ANSI_COLOR_RESET);
		scanf("%[^\n]s", comandoSeleccionado);

		operacionAsociada = analizar_operacion_asociada(comandoSeleccionado);

		switch (operacionAsociada) {

		case CORRER_PATH: {
			printf("Ha elegido la opcion: Correr PATH\n");
			correrPath();
			limpiarBuffer();
			break;  //fin correr path
		}
		case FINALIZAR_PID: {
			int32_t proc_id;
			printf("Ha elegido la opcion: Finalizar PID\n");
			printf("Ingrese PID del proceso a finalizar\n");

			while(scanf("%d",&proc_id) != 1)
			{
				printf("Por favor solo ingrese números\n");
				while(getchar() != '\n');
			}

			finalizarPID(proc_id);
			limpiarBuffer();
			break;  //fin finalizar PID
		}
		case PS: {
			printf("Ha elegido la opcion: Comando ps\n");
			comandoPS();
			limpiarBuffer();
			break; //fin comando ps
		}
		case CPU: {
			printf("Ha elegido la opcion: Chequear uso de las Cpu\n" );
			usoDeLasCpus();
			limpiarBuffer();
			break; //fin uso de las Cpus
		}
		case CERRAR_CONSOLA:{
			finalizar=true;
			break;
		}
		case HELP: {
			mostrarComandos();
			limpiarBuffer();
			break;
		}
		default: {
			printf(ANSI_COLOR_RED "EL COMANDO INGRESADO NO CORRESPONDE CON NINGUNA DE LAS OPERACIONES \n");
			printf(ANSI_COLOR_BOLDCYAN "INGRESE help PARA VER COMANDOS DISPONIBLES." ANSI_COLOR_RESET ENTER);
			limpiarBuffer();
			break;
		}
		}

		}

		printf(ANSI_COLOR_CYAN "    ╔═══════════════════════════════╗ \n    ║ CONSOLA PLANIFICADOR ENDS     ║ \n    ╚═══════════════════════════════╝" ANSI_COLOR_RESET ENTER);

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
	return 7;
}

void correrPath() {
	char filePath[100];

	printf("Ingrese el path completo del archivo que desea correr\n");
	getchar();
	scanf("%[^\n]s", filePath);

	if (string_equals_ignore_case(arch->algoritmo, "FIFO")) {
		//se hace esto si es fifo
		administrarPath(filePath);
		asignarPCBaCPU();
	} else if (string_equals_ignore_case(arch->algoritmo, "RR")) {
		//Se hace esto si es RR
		administrarPath(filePath);
	}
}

void finalizarPID(int32_t proc_id) {

}

void mostrarEstadoProceso(PCB* unPcb) {

	char* proceso = string_new();
	string_append_with_format(&proceso, "mProc %d: %s -> %s\n", unPcb->PID,
			unPcb->ruta_archivo, unPcb->estado);
	printf("%s", proceso);
}

void comandoPS() {

//todo: probar!

	list_iterate(colaListos, mostrarEstadoProceso);
	list_iterate(colaBlock, mostrarEstadoProceso);
	list_iterate(colaExec, mostrarEstadoProceso);
	list_iterate(colaFinalizados, mostrarEstadoProceso);

}

void mostrarEstadoCpus() {
	//pedir a todas las cpus su rendimiento en el ultimo minuto
	//un paquete id y rendimiento
	char* cpus = string_new();
	//string_append_with_format(&cpus, "cpu %d: %d%%\n", id, rendimiento);
	printf("%s", cpus);
}

void usoDeLasCpus() {
	printf("El porcentaje de uso de las cpus en el último minuto es:\n");
	mostrarEstadoCpus();
}

void mostrarComandos() {

	printf(ANSI_COLOR_BOLDCYAN "Los comandos disponibles son: \n");
	printf("correr PATH \t\t PATH es la ruta relativa al programa mCod \n");
	printf("finalizar PID \t\t PID es el numero de proceso mProc \n");
	printf("ps \t\t Lista los mProc y su estado actual \n");
	printf("cpu \t\t Listar las cpus con su utilizacion \n");
	printf("cerrar consola \t\t Cerrar la consola" ANSI_COLOR_RESET ENTER);
}

