/*
 * Colores.h
 *
 *  Created on: 27/8/2015
 *      Author: utnso
 */

#ifndef COLORES_H_
#define COLORES_H_

#include "Planificador.h"

#define ENTER "\n\n\n"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_BOLDBLACK   "\033[1m\033[30m"
#define ANSI_COLOR_BOLDRED     "\033[1m\033[31m"
#define ANSI_COLOR_BOLDGREEN   "\033[1m\033[32m"
#define ANSI_COLOR_BOLDYELLOW  "\033[1m\033[33m"
#define ANSI_COLOR_BOLDBLUE    "\033[1m\033[34m"
#define ANSI_COLOR_BOLDMAGENTA "\033[1m\033[35m"
#define ANSI_COLOR_BOLDCYAN    "\033[1m\033[36m"
#define ANSI_COLOR_BOLDWHITE   "\033[1m\033[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"

char* convertToString(int32_t);
char* get_hours(char*);
char* get_minutes(char* time);
char* get_seconds(char* time);
char* get_milliseconds(char* time);
char* get_actual_time();
int32_t get_actual_time_integer();
char* generate_absolute_path(char*);


#endif /* COLORES_H_ */
