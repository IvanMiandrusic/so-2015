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


/*
 * Un ejemplo de como se puede usar los colores, bien básico e ilustrativo:

 printf(ANSI_COLOR_RESET);		//resetea el color, vuelve a blanco
 t_log* loggerInfo = log_create("hola", "milog", 1, LOG_LEVEL_INFO);
 printf(ANSI_COLOR_CYAN"Esto es cyan\n"ANSI_COLOR_RESET);
 printf("Esto va en blanco\n");
 log_info(loggerInfo, ANSI_COLOR_BLUE "Un Log en azul" ANSI_COLOR_RESET);
 printf("Esto va en blanco\n");
 printf(ANSI_COLOR_BOLDGREEN"Esto es verde\n"ANSI_COLOR_RESET);
 printf(ANSI_COLOR_RED"Esto es red\n"ANSI_COLOR_RESET);
 printf(ANSI_COLOR_YELLOW"Esto es yellow\n"ANSI_COLOR_RESET);
 printf("Esto va en blanco\n");
 printf(ANSI_COLOR_MAGENTA);
 log_info(loggerInfo, ANSI_COLOR_BLUE "Un Log en azul con encabezado magenta" ANSI_COLOR_RESET);
 printf("Esto va en blanco\n"); */



#endif /* COLORES_H_ */
