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
#include "parser.h"
#include "utils.h"
#include "fs.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>

int bgOpen = 0;
pid_t pbg;
int findAmp;

void checkbgDone()
{
  if((bgOpen == 1) && waitpid(pbg, NULL, WNOHANG) != 0)
    {
      bgOpen = 0;
      waitpid(pbg, NULL, 0);
    }
}

void zombieAlert(int num)
{
  int checkNum = waitpid(-1, NULL, WNOHANG); //return -1 : no child return 0 : running child(Orphan) return > 0 zombie child.
  //you can make auto zombie process killer by while loop.
  if((checkNum > 0)) write(STDERR_FILENO, "zombie alert\n", strlen("zombie alert")+1);

  exit(1);
}



int main()
{
  char command_buffer[4096] = { 0, };
  int pipeFlag; int i;
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGINT, SIG_IGN);
  signal(SIGTERM, zombieAlert); //Zombie Alert when exit by kill
  signal(SIGHUP, zombieAlert);
  signal(SIGQUIT, zombieAlert); //Zombie Alert when exit by key stroke

  //pid_t zombie = fork();
  //if(zombie == 0) exit(0); 
  //zombie test

  while (fgets(command_buffer, 4096, stdin) != NULL) {
    int argc = -1;
    char** argv = NULL;
    
    if(strcmp(command_buffer, "\n") == 0) strcpy(command_buffer, "No_Input"); //When String have no input

    parse_command(command_buffer, &argc, &argv);
    assert(argv != NULL);
    if (strcmp(argv[0], "exit") == 0) {
      FREE_2D_ARRAY(argc, argv);
      if(bgOpen == 1) 
      {
        kill(pbg, SIGKILL);
        waitpid(pbg, NULL, 0);
      } // when exit the background process killed and waited.
      break;
    }

    findAmp = findAmpersand(argv, &argc); //if & exists. 
    checkbgDone(); //check background process finish

    struct command_entry* comm_entry = fetch_command(argv[0]);
    
    if (comm_entry != NULL) 
    {
      if(findAmp == -1) 
      {
        int ret = comm_entry->command_func(argc, argv);
        if (ret != 0) {
          comm_entry->err(ret);
        }
      }
      else if(bgOpen == 1) printf("ERROR : Background Process Already Exists\n");
      else //background process run
      {
        bgOpen = 1;
        pbg = fork();
        if(pbg == 0)
        {
          setpgid(0, 0);
          int ret = comm_entry->command_func(argc, argv);
          if (ret != 0) {
            comm_entry->err(ret);
          }
          exit(0);
        }

      }
    }
    else if( strcmp(argv[0], "kill") == 0 ) //kill command
    {
      if(argv[1] == NULL) printf("please put kill [pid]\n");
      else do_kill(argv[1]);
    } 
    else if(strcmp(argv[0], "fg") == 0)
    {
      checkbgDone(); //check background process finish
      if(bgOpen == 1)
      {
        printf("Comamnd : fg execute\n");
        pid_t pgid = __getpgid(pbg);
        tcsetpgrp(STDIN_FILENO, pgid); //give terminal access to background
        waitpid(pbg, NULL, 0); 
        tcsetpgrp(STDIN_FILENO, getpgrp()); //return terminal access to mysh
        bgOpen = 0;
      }
      else
      {
        printf("ERROR: No Background Process\n");
      }
    }
    else
    {
      resolve_path(argv[0]); //path resolved
      pipeFlag = 0;
      i =0;
      while(argv[i] != NULL)
      {
        if(strcmp(argv[i], "|") == 0)
        {
          pipeFlag = i;
          break;
        }
        ++i;
      }
    
      if(pipeFlag != 0) 
      {
        doPipeExec(argv, pipeFlag); //execute custom pipe function
      } 
      else if (does_exefile_exists(argv[0])) 
      {
        if(findAmp == -1)
        {
          __pid_t pid;

          pid = fork();

           if(pid != 0) 
          {
            waitpid(pid, NULL, 0); 
          }
           else
           {
             int err = execv(argv[0], argv);
             if(err == -1)
              {
               //don't use
              }
            }
         }
         else if(bgOpen == 1)
         {
           printf("ERROR : Background Process Already Exists\n");
         }
         else //execute background process
         {
           bgOpen = 1;
           pbg = fork();

           if(pbg == 0)
           {
             setpgid(0, 0);
             execv(argv[0], argv); 

             printf("ERROR : EXEC FAIL\n"); exit(0);
           }
         }
      } 
      else 
      {
        assert(comm_entry == NULL);
        fprintf(stderr, "%s: command not found.\n", argv[0]);
      }
    }
    FREE_2D_ARRAY(argc, argv);
    
    
  }
  int buf;
  zombieAlert(buf); //zombie alert when input is "exit"
  return 0;
}
