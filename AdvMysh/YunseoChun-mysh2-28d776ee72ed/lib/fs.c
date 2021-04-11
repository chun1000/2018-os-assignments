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
#include "fs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/wait.h>
#include <fcntl.h>



#define FILE_SERVER "/tmp/FileServer"
int syncToken = 0;

extern int bgOpen;
extern pid_t pbg;
extern int findAmp;

void* serverOut(void *data) //the server give output to client.
{
    int serverSocket;
    int clientSocket;
    
    struct sockaddr_un serverAddress;
    struct sockaddr_un clientAddress;

    if(!access(FILE_SERVER, F_OK)) unlink(FILE_SERVER);

    serverSocket = socket(PF_FILE, SOCK_STREAM, 0);

    if(serverSocket == -1)
    {
        printf("log: faill to create socket\n");
        exit(1);
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sun_family = AF_UNIX;
    strcpy(serverAddress.sun_path, FILE_SERVER);

    if(bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
    {
        printf("log: fail to bind server\n");
        exit(1);
    }

        if(listen(serverSocket, 5))
        {
            printf("log: fail to listen server\n");
            exit(1);
        }
        int clientAddressSize = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);

        if(clientSocket == -1)
        {
             printf("log: fail to find client\n");
             exit(1);
        }
        
        char** argv = (char**)data;
        pid_t p = fork();

        if(p == 0)
        {
            dup2(clientSocket, STDOUT_FILENO);
            execv(argv[0], argv);
            printf("Execv Error in Server\n");
            exit(1);
        }
        else
        {
            wait(NULL);
            close(clientSocket); close(serverSocket);
            syncToken = 1; //client execute time when EOF(socket Close) if not, sort likely function wait EOF
            return 0;
        }
}

void *clientIn(void *data) //client receive server output
{
    int clientSocket;
    struct sockaddr_un serverAddress;
    while(1)
    {
        clientSocket = socket(PF_FILE, SOCK_STREAM, 0);
        if(clientSocket != -1) break;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sun_family = AF_UNIX;
    strcpy(serverAddress.sun_path, FILE_SERVER);
    int isConnected;
    while(1)
    {
        isConnected = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
        if(isConnected != -1) break;
    } //if you do not use while, than client is faster than server connect, than error
    char** argv = (char**)data;
    while(syncToken == 0 ) {} // wait until server execute done.
    pid_t p = fork();

    if(p == 0)
    {
        dup2(clientSocket, STDIN_FILENO); //send server output to client input
        execv(argv[0], argv);
        printf("Exec in Client Error\n");
        exit(1);
    }
    else
    {
        wait(NULL);
        syncToken = 0; //reset syncToken
        return 0;
    }
}


int doUdpPipe(char **argv1, char **argv2)
{
    pthread_t thread[2];
    if(findAmp == -1) //when not background
    {
        pthread_create(&thread[0], NULL, serverOut, (void*)argv1);
        pthread_create(&thread[1], NULL, clientIn, (void*)argv2);
        pthread_join(thread[0], NULL);
        pthread_join(thread[1], NULL);
    }
    else if(bgOpen == 1) printf("ERROR : Background Process already Exists..\n");
    else // when background execution
    {
        bgOpen = 1;
        pbg = fork();

        if(pbg == 0)
        {
            setpgid(0, 0);
            pthread_create(&thread[0], NULL, serverOut, (void*)argv1);
            pthread_create(&thread[1], NULL, clientIn, (void*)argv2);
            pthread_join(thread[0], NULL);
            pthread_join(thread[1], NULL);
            exit(0);
        }
    }
}

int resolve_path(char* fileName);

void doPipeExec(char **argv, int pipeFlag) //this function do split two command and give command to multithread
{
    int freeIndex1 = 0;int freeIndex2 = 0;
    char **argv1 = (char**)malloc(sizeof(char*)*50);
    char **argv2 = (char**)malloc(sizeof(char*)*50);
    
    for(int i =0; i< 50; i++)
    {
        argv1[i] = NULL; argv2[i] = NULL;
    }

    freeIndex1 = pipeFlag;
    for(int i =0; i < pipeFlag; i++)
    {
        argv1[i] = (char*)malloc(sizeof(char)*512);
        strcpy(argv1[i], argv[i]);
    }

    int i = 0;
    int k = pipeFlag + 1;
    while(argv[k] != NULL)
    {
        argv2[i] = (char*)malloc(sizeof(char)*512);
        strcpy(argv2[i], argv[k]);
        i++; k++;
    }

    freeIndex2 = i;
    resolve_path(argv2[0]);

    doUdpPipe(argv1, argv2); //give command to multithread

    for(i =0; i < freeIndex1; i++) free(argv1[i]);
    for(i =0; i < freeIndex2; i++) free(argv2[i]);

    free(argv1); free(argv2);
    
}

int does_exefile_exists(const char* path)
{
  struct stat file_info;
  if(stat(path, &file_info) == 0) return 1;
  else return 0;
}

int resolve_path(char* fileName) //resolve path only use getenv, PATH. you can write easier by execp family.
{
  struct stat file_info;


  char *nonParsed = getenv("PATH");
  
  char parsedPath[100][512];

  int nonParsedPointer = 0;
  int parsedPathPointer = 0;
  int parsedPathNum = 0;

  while(1)
  {
    if(nonParsed[nonParsedPointer] == ':')
    {
      nonParsedPointer++;
      parsedPath[parsedPathNum][parsedPathPointer] = '\0';
      parsedPathNum++;
      parsedPathPointer = 0;
     
    }
    else
    {
      parsedPath[parsedPathNum][parsedPathPointer] = nonParsed[nonParsedPointer];
      parsedPathPointer++;
      nonParsedPointer++;
    }

    if(nonParsed[nonParsedPointer] == '\0')
    {
      parsedPath[parsedPathNum][parsedPathPointer] = '\0';
      break;
    }
  } //parse

  int i;
  char buffer[1024];
  for(i = 0; i<= parsedPathNum; i++)
  {
    strcpy(buffer, parsedPath[i]);
    strcat(buffer, "/");
    strcat(buffer, fileName);

    if(stat(buffer, &file_info) == 0) 
    {
      strcpy(fileName, buffer);
      return 1;
    }
  }
  return 0;
  //find
  
}