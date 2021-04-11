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
#define _POSIX_SOURCE
#include "commands.h"
#include "fs.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>


static struct command_entry commands[] =
{
  {
    "pwd",
    do_pwd,
    err_pwd
  },
  {
    "cd",
    do_cd,
    err_cd
  }
};

struct command_entry* fetch_command(const char* command_name)
{
  if(strcmp(command_name, "pwd") == 0) return &commands[0];
  else if(strcmp(command_name, "cd") == 0) return &commands[1];
  else return NULL;
}

int do_kill(char *pid)
{
  int pidNum = atoi(pid);
  if(pidNum == 0)
  {
    return -1;
  }
  else
  {
    kill(pidNum, SIGKILL);
    printf("kill [%d] execute\n", pidNum);
  }
}

int do_pwd(int argc, char** argv)
{
  char buffer[256];
  char *returnVal;

  returnVal = getcwd(buffer, 256);
  strcat(buffer, "\n");
  write(STDOUT_FILENO, buffer, sizeof(char)*strlen(buffer));
  //printf("%s\n", buffer);

  if(returnVal != NULL) return 0;
  else return -1;
}

void err_pwd(int err_code)
{
  // TODO: Fill it.
}

int do_cd(int argc, char** argv)
{
  int isDirectoryExists = chdir(argv[1]);
  if(isDirectoryExists == 0)
  {
    return 0;
  }
  else
  {
    if(does_exefile_exists(argv[1]) == 1) return 2;
    else return 1;
  }
}

void err_cd(int err_code)
{
  char buf[256];
  if(err_code == 1) fprintf(stderr, "cd: no such file or directory\n");
  else if (err_code == 2)  fprintf(stderr, "cd: not a directory\n");
}
