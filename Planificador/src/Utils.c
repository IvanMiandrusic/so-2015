/*
 * Utils.c
 *
 *  Created on: 10/09/2015
 *      Author: federico
 */
#include <stdlib.h>
#include <stdio.h>
#include <commons/string.h>
#include "Utils.h"

char* convertToString(int32_t num) {

	char buffer[7];
	sprintf( buffer, "%d", num);
	buffer[7] = '\0';
	return buffer;
}

char* get_hours(char* time) {

	char* hora = malloc(3);
	hora[0]= time[0];
	hora[1]= time[1];
	hora[2] = '\0';

	return hora;
}

char* get_minutes(char* time) {

	char* minutos=malloc(3);
	minutos[0]=time[3];
	minutos[1]=time[4];
	minutos[2]='\0';

	return minutos;
}

char* get_seconds(char* time) {

	char* segundos=malloc(3);
	segundos[0]=time[6];
	segundos[1]=time[7];
	segundos[2]='\0';

	return segundos;
}



/** Retorna el tiempo actual del sistema en formato hhmmssmmmm **/
char* get_actual_time() {

	char* time = temporal_get_string_time();

	char* locale_time = string_new();
	string_append_with_format(&locale_time, "%s%s%s",
					get_hours(time),
					get_minutes(time),
					get_seconds(time));

	free(time);
	return locale_time;
}

/** Retorna el tiempo actual del sistema como entero en formato hhmmssmmmm **/
int32_t get_actual_time_integer() {

	char* time = get_actual_time();
	int32_t valor= atoi(time);
	free(time);
	return valor;
}

char* generate_absolute_path(char* filePath) {

	/** Creo el comando locate para encontrar el path del archivo en el FS y redirecciono salida en archivo **/
	char* command = string_new();
	string_append_with_format(&command, "locate -i -l 1 %s > locate.out", filePath);

	/** Ejecuto el comando **/
	system(command);

	/** Abro archivo para leer el path **/
	FILE* output_file = fopen("locate.out", "r");
	if(NULL == output_file)
	{
		log_error(loggerError, ANSI_COLOR_BOLDRED "Un error ocurrio al abrir el archivo con el path" ANSI_COLOR_RESET);
		printf(ANSI_COLOR_BOLDRED "Un error ocurrio al abrir el archivo con el path" ANSI_COLOR_RESET);
		return NULL;
	}

	/** Obtengo el tamaño del contenido **/
	fseek(output_file, 0, SEEK_END);
	int32_t fsize = ftell(output_file);
	if(fsize < 0) {
		log_error(loggerError, ANSI_COLOR_BOLDRED "Un error ocurrio para saber tamaño del archivo con el path" ANSI_COLOR_RESET);
		printf(ANSI_COLOR_BOLDRED "Un error ocurrio para saber tamaño del archivo con el path" ANSI_COLOR_RESET);
		return NULL;
	}
	fseek(output_file, 0, SEEK_SET);

	/** Leo el contenido (el path absoluto) **/
	char* absolute_path = malloc(fsize);
	int32_t read = fread(absolute_path, fsize, 1, output_file);
	if(read == 0) {
		log_error(loggerError, ANSI_COLOR_BOLDRED "Un error ocurrio al leer el archivo con el path" ANSI_COLOR_RESET);
		printf(ANSI_COLOR_BOLDRED "El path del mCod ingresado es incorrecto" ANSI_COLOR_RESET);
		return NULL;
	}

	/** Cierro el archivo **/
	if(!fclose(output_file)){
		log_error(loggerError, ANSI_COLOR_BOLDGREEN "Exito al cerrar el archivo con el path" ANSI_COLOR_RESET);
	}
	else {
		log_error(loggerError, ANSI_COLOR_BOLDRED "Un error ocurrio al cerrar el archivo con el path" ANSI_COLOR_RESET);
		printf(ANSI_COLOR_BOLDRED "Un error ocurrio al cerrar el archivo con el path" ANSI_COLOR_RESET);
		return NULL;
	}

	absolute_path[fsize-1] = '\0';

	return absolute_path;
}
