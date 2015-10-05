/*
 * Utils.c
 *
 *  Created on: 10/09/2015
 *      Author: federico
 */
#include <stdlib.h>
#include <stdio.h>
#include "Utils.h"

char* convertToString(int32_t num) {

	char buffer[7];
	sprintf( buffer, "%d", num);
	buffer[7] = '\0';
	return buffer;
}

char* get_hours(char* time) {

	char* hora = malloc(2);
	hora[0]= time[0];
	hora[1]= time[1];
	hora[2] = '\0';

	return hora;
}

char* get_minutes(char* time) {

	char* minutos=malloc(2);
	minutos[0]=time[3];
	minutos[1]=time[4];
	minutos[2]='\0';

	return minutos;
}

char* get_minutes_v2(char* time) {

	char* minutos=malloc(2);
	minutos[0]=time[2];
	minutos[1]=time[3];
	minutos[2]='\0';

	return minutos;
}

char* get_seconds(char* time) {

	char* segundos=malloc(2);
	segundos[0]=time[6];
	segundos[1]=time[7];
	segundos[2]='\0';

	return segundos;
}

char* get_seconds_v2(char* time) {

	char* segundos=malloc(2);
	segundos[0]=time[4];
	segundos[1]=time[5];
	segundos[2]='\0';

	return segundos;
}

char* get_milliseconds(char* time){

	char* milisegundos=malloc(4);
	milisegundos[0]=time[9];
	milisegundos[1]=time[10];
	milisegundos[2]=time[11];
	milisegundos[3]=time[12];
	milisegundos[4]='\0';

	return milisegundos;
}

/** Retorna el tiempo actual del sistema en formato hhmmssmmmm **/
char* get_actual_time() {

	char* time = temporal_get_string_time();

	char* locale_time = string_new();
	string_append_with_format(&locale_time, "%s%s%s",
					get_hours(time),
					get_minutes(time),
					get_seconds(time));

	return locale_time;
}

/** Retorna el tiempo actual del sistema como entero en formato hhmmssmmmm **/
int32_t get_actual_time_integer() {

	char* time = get_actual_time();
	return atoi(time);
}

