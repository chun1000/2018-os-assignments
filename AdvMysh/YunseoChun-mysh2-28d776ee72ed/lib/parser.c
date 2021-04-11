/**********************************************************************
 * Copyright (C) Jaewon Choi <jaewon.james.choi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 *********************************************************************/
#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


int findAmpersand(char **argv, int *argc)
{
	int i = 0;
	int pointer = -1;
	while(argv[i] != NULL)
	{	
		if(strcmp(argv[i], "&") == 0) pointer = i;
		i++;
	}

	if(pointer != -1)
	{
		if(argv[i-1] == argv[pointer])
		{
			free(argv[pointer]);
			argv[pointer] = NULL;
			*argc -= 1;
		}
		else pointer = -1;
	} // when & in last 

	return pointer;
}

void parse_command(const char* input,
                   int* argc, char*** argv)
{
	int inputIndex = 0; int spaceFlag = 1; int commandFlag = 0;
	int charIndex = 0; int stringIndex = 0;

	char buffer[150];
	char** arg = (char**)malloc(sizeof(char*) * 150);

	for (int i = 0; i<150; i++) arg[i] = NULL;

	while (!((input[inputIndex] == '\0') || (input[inputIndex] == '\n'))) 
	{
		if (input[inputIndex] == '\"')
		{
			inputIndex += 1;
			while (!(input[inputIndex] == '\"'))
			{
				buffer[charIndex++] = input[inputIndex++];
			}
			buffer[charIndex] = '\0'; charIndex = 0;
			arg[stringIndex] = (char*)malloc(sizeof(buffer));
			strcpy(arg[stringIndex++], buffer);
			inputIndex += 1;

		}
		else if (input[inputIndex] == ' ')
		{
			if (spaceFlag == 1)
			{
				inputIndex++;
			}
			else
			{
				spaceFlag = 1;
				inputIndex++;
				buffer[charIndex] = '\0'; charIndex = 0;
				arg[stringIndex] = (char*)malloc(sizeof(buffer));
				strcpy(arg[stringIndex++], buffer);
			}
		}
		else
		{
			spaceFlag = 0;
			buffer[charIndex++] = input[inputIndex++];

			if (input[inputIndex] == '\0' || input[inputIndex] == '\n')
			{
				buffer[charIndex] = '\0';
				arg[stringIndex] = (char*)malloc(sizeof(buffer));
				strcpy(arg[stringIndex++], buffer);
			}
		}
	}



 	*argv = arg;
	*argc = stringIndex;
}
