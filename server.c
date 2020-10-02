//
//  main.c
//  sockets_varios_clientes
//
//  Created by Vicente Cubells Nonell on 28/09/15.
//  Copyright © 2015 Vicente Cubells Nonell. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#define TCP_PORT 8000
#define SEMAFOROS 4

int main(int argc, const char * argv[])
{
    struct sockaddr_in direccion;
    char buffer[1000];
    
    int servidor, cliente;
    ssize_t leidos, escritos;
    int continuar = 1;

    pid_t pid;
    
    int * pids = malloc(SEMAFOROS*sizeof(int));
    int * clientes = malloc(SEMAFOROS*sizeof(int));
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
    listen(servidor, 10);
    
    escritos = sizeof(direccion);
    
    printf("PID %d\n",getpid());
    // Aceptar conexiones
    int i = 0;
    while (i<SEMAFOROS)
    {

        *(clientes+i) = accept(servidor, (struct sockaddr *) &direccion, &escritos);
        
        printf("Aceptando conexiones en %s:%d \n",
               inet_ntoa(direccion.sin_addr),
               ntohs(direccion.sin_port));
        
        read(*(clientes+i),&buffer,sizeof(buffer));
        *(pids+i) = atoi(buffer);
        printf("Semaforo %d con PID %d creado\n",i+1,*(pids+i));
        i++;
        pid = fork();
        
        if (pid == 0) continuar = 0;
        
    }
    int * aux = pids;
    int * auxclient = clientes;

    for(aux=pids+2; auxclient<clientes+SEMAFOROS; aux++, auxclient++){
        if(aux==pids+SEMAFOROS){
            sprintf(buffer, "%d", *(pids));
            write(*auxclient, &buffer, sizeof(buffer));
        }
        else{
            sprintf(buffer, "%d", *(aux));
            write(*auxclient, &buffer, sizeof(buffer));
        }
    }
    if (pid == 0) {
        
        close(servidor);
        
        if (cliente >= 0) {
            
            // Leer datos del socket
            while (leidos = read(cliente, &buffer, sizeof(buffer))) {
                printf("Llegue aqui nenes");
                write(fileno(stdout), &buffer, leidos);
                
                /* Leer de teclado y escribir en el socket */
                leidos = read(fileno(stdin), &buffer, sizeof(buffer));
                write(cliente, &buffer, leidos);
                write(fileno(stdout), &buffer, leidos);
            }
        }
        free(aux);
        close(cliente);
    }
    
    else if (pid > 0)
    {
        while (wait(NULL) != -1);
        
        // Cerrar sockets
        close(servidor);
        
    }
    free(pids);
    
    return 0;
}


