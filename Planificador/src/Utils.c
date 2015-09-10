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

	char buffer[3];
	sprintf( buffer, "%d", num);
	buffer[2] = '\0';
	return buffer;
}
