
//
//  cliente.c
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

#define TCP_PORT 8000
#define TIEMPO 30

int cliente; 
int next_pid;
int estado;  
int interrupcion; 

void cambio(int signal) {
    estado = 1;
    char state[] = "VERDE"; 
    write(cliente, &state, sizeof(state));
    alarm(TIEMPO);
}

void sigSemaforo(int signal) {
    estado = 0;
    kill(next_pid, SIGUSR1);
}

int main(int argc, const char * argv[])
{   
    sigset_t conjunto, pendientes;
    sigemptyset(&conjunto);
    sigaddset(&conjunto, SIGALRM);
    sigaddset(&conjunto, SIGUSR1);

    struct sockaddr_in direccion;
    char buffer[1000];
    
    ssize_t leidos, escritos;
    
    if (argc != 2) {
        printf("Use: %s IP_Servidor \n", argv[0]);
        exit(-1);
    }
    
    // Crear el socket
    cliente = socket(PF_INET, SOCK_STREAM, 0);
    // Establecer conexión
    inet_aton(argv[1], &direccion.sin_addr);
    direccion.sin_port = htons(TCP_PORT);
    direccion.sin_family = AF_INET;
    
    escritos = connect(cliente, (struct sockaddr *) &direccion, sizeof(direccion));
    
    if (escritos == 0) {
        printf("Conectado a %s:%d \n",
               inet_ntoa(direccion.sin_addr),
               ntohs(direccion.sin_port));

        sprintf(buffer,"%d",getpid());
        write(cliente, buffer, sizeof(int));
        leidos = read(cliente, &buffer, sizeof(buffer));
        next_pid = atoi(buffer);

       signal(SIGUSR1, cambio);
        signal(SIGALRM, sigSemaforo);

       while (leidos = read(cliente, &buffer, sizeof(buffer))) {
            if (strcmp(buffer, "START") == 0) {
                raise(SIGUSR1);
            } else if (strcmp(buffer, "STOP") == 0 && interrupcion != 2) {
                interrupcion = 2;
                sigprocmask(SIG_BLOCK, &conjunto, NULL);
            } else if (strcmp(buffer, "INTERMITENT") == 0 && interrupcion != 3) {
                interrupcion = 3;
               sigprocmask(SIG_BLOCK, &conjunto, NULL);
            }
            else {
                interrupcion = 0;
                sigprocmask(SIG_UNBLOCK, &conjunto, NULL);
            }
        }
    }
    
    // Cerrar sockets
    close(cliente);
    
    return 0;
}

