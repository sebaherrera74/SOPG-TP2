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
#include "serial.h"
/**DEFINES**/

#define ERROR_MSGE_TREAHDEDUCIAA  "No se pudo iniciar thread para iniciar comunicacion serial"

/*Variable globales */


/*Declaracion de funciones*/
void bloquearSign( void );
void desbloquearSign(void);
void sigHandler( int sig );

volatile sig_atomic_t salida = 0;	// Bandera para salir del bucle cuando hay una señal 

int main(void)
{   
 pthread_t threadEduCiaa;                //Thread para recibir datos del puertos serie de la edu-ciaa 
 pid_t miPid; 
 struct sigaction sa;

miPid = getpid();                        // obtengo el pid de este proceso
printf("Inicio Serial Service (PID:%d)\r\n", miPid);

/* configuramos  SIGINT y SIGTERM */
    sa.sa_handler = sigHandler;
    sa.sa_flags = 0;
    if( sigemptyset( &sa.sa_mask ) == -1)
    {
        perror( "sigemptyset" );
        exit( 1 );
    }
    if( sigaction( SIGINT, &sa, NULL ) == -1 )
    {
        perror( "sigaction" );
        exit( 1 );
    }

    if( sigemptyset( &sa.sa_mask ) == -1)
    {
        perror( "sigemptyset" );
        exit( 1 );
    }
    if( sigaction( SIGTERM, &sa, NULL ) == -1 )
    {
        perror( "sigaction" );
        exit( 1 );
    }

/* bloqueamos  señales */
	bloquearSign();


 
/*aqui lanzar thread de serial edu-ciaa */

if (ThreadEduciaa(&threadEduCiaa)){
	perror(ERROR_MSGE_TREAHDEDUCIAA);
	return 1;
}

/*aqui lanzar thread de TCP entre serial service e interfaceService*/

/* se desbloquean las señales SIGINT y SIGTERM */
	desbloquearSign();

while(!salida){
	sleep(1);

}

    printf("\n\n sale While\r\n");
	exit(EXIT_SUCCESS);
	return 0;
}






/* función para bloquear signals */
void bloquearSign( void )
{
	sigset_t set;
	int s;
	sigemptyset( &set );
	sigaddset( &set, SIGINT );
	sigaddset( &set, SIGTERM );
	pthread_sigmask( SIG_BLOCK, &set, NULL );
}

/* función para desbloquear signals */
void desbloquearSign( void )
{
	sigset_t set;
	int s;
	sigemptyset( &set );
	sigaddset( &set, SIGINT );
	sigaddset( &set, SIGTERM );
	pthread_sigmask( SIG_UNBLOCK, &set, NULL );
}

/* handler para manejo de SIGINT */
void sigHandler( int sig )
{
	salida=1;
	
}