#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "commons/string.h"
#include "parser.h"

/*FUNCIONES PRIVADAS*/
int32_t analizar_operacion_asociada(char* operacion) {

	if(string_starts_with(operacion, "iniciar")) return INICIAR;
	if(string_starts_with(operacion, "finalizar")) return FINALIZAR;
	if(string_starts_with(operacion, "entrada-salida")) return ENTRADASALIDA;
	if(string_starts_with(operacion, "leer")) return LEER;
	if(string_starts_with(operacion, "escribir")) return ESCRIBIR;
	return ERROR;
}
int esEspacio(char caracter){return(caracter==' ')?1:0;}

int esPuntoyComa(char caracter){return(caracter==';')?1:0;}

int32_t buscarPrimerParametro(char*linea){
	char* parametro=string_new();
	while(!esPuntoyComa(*linea)){//este while sirve si lees la instruccion con el ; final
		string_append_with_format(&parametro,"%c",*linea);
		linea++;
	}
	int32_t valor=atoi(parametro);
	free(parametro);
	return valor;
}

char* buscarSegundoParametro(char*linea){
	char* parametro2=string_new();
	while(!esPuntoyComa(*linea)){//este while sirve si lees la instruccion con el ; final
		string_append_with_format(&parametro2,"%c",*linea);
		linea++;
	}
	return parametro2;
}

structEscribir buscarAmbosParametros(char*linea){
	char* parametro1=string_new();
	while(!esEspacio(*linea)){
		string_append_with_format(&parametro1,"%c",*linea);
		linea++;
	}
	linea++;
	char*parametro2=buscarSegundoParametro(linea);
	structEscribir parametros;
	parametros.pagina=atoi(parametro1);
	parametros.tamanio_texto=strlen(parametro2);
	parametros.texto=parametro2;
	free(parametro1);
	return parametros;

}




