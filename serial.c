#include <stdio.h>
#include <stdlib.h>
#include "SerialManager.h"
#include "serial.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>

/*funciones*/
int OpenSerie (void);
void * receiveUartEduciaaSendToSocket( void * parameters );

char buffer [BUFFER_MAZ_SIZE];



/* funcion de apertura puerto serie*/
int OpenSerie (void) {
    char mensaje [BUFFER_MAZ_SIZE];
     /* abro puerto serie */
    printf("Abriendo puerto serie...");
    if(serial_open(UART_NUM, UART_BAUDRATE)) {
        sprintf(mensaje, "error al abrir puerto. Esta conectada la CIAA? %s\nSaliendo...", TTY_TEXTO);
        perror(mensaje);
        return 1;        
    }
    printf("Puerto serie abierto: %s %d 8N1\n", TTY_TEXTO, UART_BAUDRATE);

    return 0;
}


int ThreadEduciaa(pthread_t* threadciaa){
	
printf("thread para iniciar comunicacion con edu-ciaa a traves de puerto serie\n");

if(OpenSerie()){
        printf("No se pudo abrir el puerto serie\n");
        return 1;
     }

/* se crea un thread para recibir de la UART y mandar al socket */
if( pthread_create (threadciaa, NULL, receiveUartEduciaaSendToSocket, NULL ) == -1 )
{
	perror( "pthread_create" );
	return 0;
}
    pthread_join (threadciaa, NULL);
	//printf("Termino\n");

    return 0;
}

/* handler para el thread que recibe de la UART y manda al socket */
void * receiveUartEduciaaSendToSocket( void * parameters )
{
	int nbytesrecibidos;
while(1)
	{	
	 nbytesrecibidos = serial_receive( buffer, BUFFER_MAZ_SIZE );

//		/* se bloquea el mutex */
//		pthread_mutex_lock (&mutexData);

/*si recibi mensajes imprimo e envio por el socket*/

		if( nbytesrecibidos > 0 )//&& newSockfd > 0 )
		{
		/* se imprimi un mensaje de recepción */
		buffer[ nbytesrecibidos - 2 ] = '\0'; // se restan 2 unidades para eliminar "\r\n"
		printf( "recibido por la uart: %s\n", buffer );
		/* se envian los mensajes al socket */
		if( write( newfd, buffer, strlen( buffer ) ) == -1 )
		{
			perror( "socket_write" );
			return NULL;
		}
		}

		/* se desbloquea el mutex */
		//pthread_mutex_unlock (&mutexData);

		/* tiempo de refresco del polling */
		usleep(10000);
	}
	
	return NULL;
}