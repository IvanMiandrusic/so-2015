/*
 * Utils.c
 *
 *  Created on: 10/09/2015
 *      Author: federico
 */
#include <stdlib.h>
#include <stdio.h>
#include <commons/string.h>
#include <limits.h>
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

	char absolute_path[PATH_MAX+1];

	char* found = realpath(filePath, absolute_path);

	/** Chequeo si encontro el archivo en FS y obtengo el path absoluto **/
	if(found != NULL) {
		return absolute_path;
	}
	else {
		perror(ANSI_COLOR_BOLDRED "The mCod no fue encontrado" ANSI_COLOR_RESET);
		return NULL;
	}
}
