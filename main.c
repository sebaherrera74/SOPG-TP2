#include <stdio.h>
#include <stdlib.h>
#include "SerialManager.h"

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

/**DEFINES**/
#define BUFFER_MAZ_SIZE     120   //Tamaño maximo del buffer
#define PORT_NUMBER        10000  //Puerto de conexion del socket
#define UART_NUM            1     //Identificador del puerto serie utilizado 
#define  UART_BAUDRATE     115200 //Baudrate del puerto serie 

/*Variable globales */
char buffer[BUFFER_MAZ_SIZE];     // buffer para guardar los densajes de la Uart y del socket 
pthread_t thread;                 //Treahd para recibir datos del puertos serie de la edu-ciaa 

/*Declaracion de funciones*/

/* handler para el thread que recibe de la UART de la edu-ciaa y manda al socket */
void *  receiveUartEduciaaSendToSocket  (void * parameters );

int main(void)
{

	printf("Inicio Serial Service\r\n");

/*Abre el puerto serial*/
if( serial_open( UART_NUM, UART_BAUDRATE ) == 1 ){
	exit( 1 );
}
	
/* se crea un thread para recibir de la UART y mandar al socket */
if( pthread_create ( &thread, NULL, receiveUartEduciaaSendToSocket, NULL ) == -1 )
{
	perror( "pthread_create" );
	exit( 1 );
}
    pthread_join (thread, NULL);
	printf("Termino\n");


	exit(EXIT_SUCCESS);
	return 0;
}

/* handler para el thread que recibe de la UART y manda al socket */
void * receiveUartEduciaaSendToSocket( void * parameters )
{
	int n;
while(1)
	{	
		/* se leen los mensajes enviados por la EDU-CIAA */
		n = serial_receive( buffer, BUFFER_MAZ_SIZE );

//		/* se bloquea el mutex */
//		pthread_mutex_lock (&mutexData);

		if( n > 0 )//&& newSockfd > 0 )
		{
			/* se imprimi un mensaje de recepción */
			buffer[ n - 2 ] = '\0'; // se restan 2 unidades para eliminar "\r\n"
			printf( "recibido por la uart: %s\n", buffer );
			
			/* se envian los mensajes al socket */
			//if( write( newSockfd, buffer, strlen( buffer ) ) == -1 )
			///{
			//	perror( "socket_write" );
			//	return NULL;
			//}
		}

		/* se desbloquea el mutex */
		//pthread_mutex_unlock (&mutexData);

		/* tiempo de refresco del polling */
		usleep( 1000 );
	}
	
	return NULL;
}

