//
//  main.c
//  sockets_varios_clientes
//
//  Created by Vicente Cubells Nonell on 28/09/15.
//  Copyright © 2015 Vicente Cubells Nonell. All rights reserved.
//

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TCP_PORT 8000
#define SEMAFOROS 4

int cliente_semaforo; //Accesso global a un semaforo cliente dado
int interrupcion; //Interrupción actual de consola ([o]Ninguna, [1]Interrupción Rojo, [2]Interrupción Intermitente)

void printState(int semaforo) {
    printf("Cambio de estado\n");
    printf("----------------\n");
    for (int i=0; i<SEMAFOROS; ++i) {
        if (i == semaforo)
            printf("Semaforo %d: VERDE\n", i+1);
        else 
            printf("Semaforo %d: ROJO\n", i+1);
    }
    printf("\n");
}

void stopSignal(int signal) {
    char interruption[] = "STOP"; 
    write(cliente_semaforo, &interruption, sizeof(interruption));
}

void intermitenSignal(int signal) {
    char interruption[] = "INTERMITENT"; 
    write(cliente_semaforo, &interruption, sizeof(interruption));
}

void consoleInterruption(int signal) {
    char state[15];

    if (signal == 20 && interrupcion != 1) {
        interrupcion = 1;
        strcpy(state, "ROJO");
    } else if (signal == 2 && interrupcion != 2) {
        interrupcion = 2;
        strcpy(state, "INTERMITENTE");
    } else {
        interrupcion = 0;
        printf("\nReanudando\n\n");
        return;
    }

    printf("\nInterrupción\n");
    printf("------------\n");
    for (int i=0; i<SEMAFOROS; ++i) {
        printf("Semaforo %d: %s\n", i+1, state);
    }
}

int main(int argc, const char * argv[])
{
    signal(SIGTSTP, consoleInterruption);
    signal(SIGINT, consoleInterruption);

    struct sockaddr_in direccion;
    char buffer[1000];
    
    int servidor;
    
    ssize_t leidos, escritos;
    int continuar = 1;
    pid_t pid;
    
    if (argc != 2) {
        printf("Use: %s IP_Servidor \n", argv[0]);
        exit(-1);
    }
    
    // Crear el socket
    servidor = socket(PF_INET, SOCK_STREAM, 0);
    // Enlace con el socket
    inet_aton(argv[1], &direccion.sin_addr);
    direccion.sin_port = htons(TCP_PORT);
    direccion.sin_family = AF_INET;
    
    bind(servidor, (struct sockaddr *) &direccion, sizeof(direccion));
    
    // Escuhar
    listen(servidor, SEMAFOROS);
    
    escritos = sizeof(direccion);
    
    char semaforo[SEMAFOROS][50];
    ssize_t pidInputSizes[SEMAFOROS];
    int * pids = malloc(SEMAFOROS*sizeof(int));
    int * clientes = malloc(SEMAFOROS*sizeof(int));
    int i = 0;

    while (i < SEMAFOROS)
    {
        *(clientes+i) = accept(servidor, (struct sockaddr *) &direccion, &escritos);
        
        printf("Aceptando conexiones en %s:%d \n",
            inet_ntoa(direccion.sin_addr),
            ntohs(direccion.sin_port));

        pid = fork();
        if (pid == 0) {
            cliente_semaforo = *(clientes+i); 
            signal(SIGTSTP, stopSignal);
            signal(SIGINT, intermitenSignal);

            close(servidor);
            if (cliente_semaforo >= 0) {
                while(leidos = read(*(clientes+i), &buffer, sizeof(buffer))) {
                    printState(i); 
                }
            }
            
            close(cliente_semaforo);
        }
        else {
             pidInputSizes[i] = read(*(clientes + i), &semaforo[i], sizeof(semaforo[i]));
            // *(pids + i) = read(*(clientes+i), &semaforo[i], sizeof(semaforo[i]));
        }
    i++;
}

    if (pid > 0) {
       for (int i = 0; i < SEMAFOROS; ++i) {
            int nextClient = (i + 1) % SEMAFOROS;
            write(*(clientes + i), &semaforo[nextClient], pidInputSizes[nextClient]);
            //  write(*(clientes + i), &semaforo[nextClient], *(pids + nextClient));
        }
       
        char init_message[] = "START"; 
        write(*(clientes), &init_message, sizeof(init_message));

        while (wait(NULL) != -1);
        
        // Cerrar sockets
        close(servidor);
        
    }
    return 0;
}

