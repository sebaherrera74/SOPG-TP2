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
#define UART_BAUDRATE     115200  //Baudrate del puerto serie 
#define TTY_TEXTO               "/dev/ttyUSB1"

#define ERROR_MSGE_TREAHDEDUCIAA  "No se pudo iniciar thread para iniciar comunicacion serial"
#define ERROR_MSGE_TREAHDTCP  "No se pudo iniciar thread para comunicacion con el cliente TCP"
#define BACKLOG                    10   



/*Declaracion de funciones*/
void bloquearSign( void );
void desbloquearSign(void);
void sigHandler( int sig );
int OpenSerie (void);
int ThreadEduciaa(pthread_t threadciaa);
int ThreadTCP(pthread_t threadtcp);
void * receiveUartEduciaaSendToSocket( void * parameters );
void * ComClienteTcp( void * parameters );

/*Variable globales */
char buffer [BUFFER_MAZ_SIZE];
volatile sig_atomic_t salida = 0;	// Bandera para salir del bucle cuando hay una señal 
int newfd,s;
int datoCompartido;                 //
pthread_t threadEduCiaa;  //Thread para recibir datos del puertos serie de la edu-ciaa 
pthread_t threadTcp;   //Thread para comunicacion con el cliente TCP  
 int n;
  char bufferSerial[BUFFER_MAZ_SIZE];
  char buffersocket[BUFFER_MAZ_SIZE];
  char bufferaux[BUFFER_MAZ_SIZE];  


/* declaramos las estructuras y variables para el socket */
   socklen_t addr_len;;
   struct sockaddr_in clientaddr;
   struct sockaddr_in serveraddr;

int main(void)
{   
  /*int n;
  char bufferSerial[BUFFER_MAZ_SIZE];
  char buffersocket[BUFFER_MAZ_SIZE];
  char bufferaux[BUFFER_MAZ_SIZE];  


/* declaramos las estructuras y variables para el socket */
  // socklen_t addr_len;;
   //struct sockaddr_in clientaddr;
   //struct sockaddr_in serveraddr;

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

/* Creamos socket y verificamos que se creo bien*/
	s = socket(AF_INET,SOCK_STREAM, 0);
	if (s==-1)
	{
	    printf("No se creo Socket\n");
	    exit(0);
	}
	else
	{
        printf("Se creo Socket corectamente\n");
	}


// Cargamos datos de IP:PORT del server, CARGO DIRECCIONES LOCALES
    bzero((char*) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(10000);
    //serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(inet_pton(AF_INET, "127.0.0.1", &(serveraddr.sin_addr))<=0)
    {
        fprintf(stderr,"ERROR invalid server IP\r\n");
       	exit(1);
    }
     
	// Abrimos puerto con bind()
	if (bind(s, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
		close(s);
		printf("socket bind fallado ");
		exit(1) ;
	}
	else
	{
      printf("Bind correcto\n");
	}

	// Seteamos socket en modo Listening
	if (listen (s, BACKLOG ) == -1)         // backlog=10
  	{
    	printf("error en listen");
    	exit(1);
  	}
	else
	{
		printf("Server listening\n");
	}
/*Abro puerto serie */
if(OpenSerie())
   {	
   printf("No se pudo abrir el puerto serie\n");
   return 1;
   }



/*lanzamos thread para tarea TCP de acuerdo a consideraciones del eneunciado*/
 if (ThreadTCP(threadTcp))
	{
	perror(ERROR_MSGE_TREAHDTCP);
	exit(1);
    }
	else
	{
	printf("Thread creado correctamente");
	}




/*lanzamos thread de serial edu-ciaa */
/*
    if (ThreadEduciaa(threadEduCiaa))
	{
	perror(ERROR_MSGE_TREAHDEDUCIAA);
	exit(1);
    }
	else
	{
	printf("Thread creado correctamente");
	}
*/	

/* se desbloquean las señales SIGINT y SIGTERM */
    desbloquearSign();

while(1)
{

   	usleep(10000);

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


/* funcion de apertura puerto serie*/
int OpenSerie (void) 
{
    char mensaje [BUFFER_MAZ_SIZE];
     /* abro puerto serie */
    printf("Abriendo puerto serie...\n");
    if(serial_open(UART_NUM, UART_BAUDRATE))
	{
    sprintf(mensaje, "error al abrir puerto. Esta conectada la CIAA? %s\nSaliendo...", TTY_TEXTO);
    perror(mensaje);
    return 1;        
    }
    printf("Puerto serie abierto: %s %d 8N1\n", TTY_TEXTO, UART_BAUDRATE);

    return 0;
}

int ThreadTCP(pthread_t threadtcp)
{
	
/* se crea un thread para recibir de la UART y mandar al socket */
 if( pthread_create (&threadtcp, NULL, ComClienteTcp, NULL ) == -1 )
   {
    printf( "error pthread_create" );
	return 0;
   }
    pthread_join (threadtcp, NULL);
	//printf("Termino\n");

    return 0;

}

int ThreadEduciaa(pthread_t threadciaa)
{
   printf("\nthread para iniciar comunicacion con edu-ciaa a traves de puerto serie\n");
  if(OpenSerie())
   {	
   printf("No se pudo abrir el puerto serie\n");
   return 1;
   }

/* se crea un thread para recibir de la UART y mandar al socket */
 if( pthread_create (&threadciaa, NULL, receiveUartEduciaaSendToSocket, NULL ) == -1 )
   {
    printf( "error pthread_create" );
	return 0;
   }
    pthread_join (threadciaa, NULL);
	//printf("Termino\n");

    return 0;
}
/* handler para el thread que establece comunicacion con el cliente TCP */
void * ComClienteTcp( void * parameters )
{ 
	printf("Esperando conexiones\r\n");
    
   //Ejecutamos accept() para recibir conexiones entrantes
	addr_len = sizeof(struct sockaddr_in);
    if ( (newfd = accept(s, (struct sockaddr *)&clientaddr,&addr_len)) == -1)
    {
	   perror("error en accept");
	   exit(1);
	}
	char ipClient[32];
	inet_ntop(AF_INET, &(clientaddr.sin_addr), ipClient, sizeof(ipClient));
	printf  ("server:  conexion desde:  %s\n",ipClient);
	
   while(1){
	usleep(10000);
	if( (n = read(newfd,buffersocket,120)) == -1 )
	{
		perror("Error leyendo mensaje en socket");
		exit(1);
	}
    
   else
   	{
		int out1,out2,out3,out4;   
		if(buffersocket[0]==':')
			{
				out1=buffersocket[7]-'0';  //paso de caracter a int 
				out2=buffersocket[8]-'0';
				out3=buffersocket[9]-'0';
				out4=buffersocket[10]-'0';
				sprintf(bufferaux,">OUTS:%d,%d,%d,%d\r\n",out1,out2,out3,out4);
				bufferaux[15]=0x00;
				serial_send(bufferaux,16);
				//printf("%s",bufferaux);
			}
		}
	buffersocket[n]=0x00;
	printf("Recibi %d bytes.:%s\n",n,buffersocket);
   }    
}




/* handler para el thread que recibe de la UART y manda al socket */
void * receiveUartEduciaaSendToSocket( void * parameters )
{   
    
	int nbytesrecibidos;
while(1)
	{	

	/* tiempo de refresco del polling */
	usleep(10000);		
    int aux;
	nbytesrecibidos = serial_receive( buffer, BUFFER_MAZ_SIZE ); //leo puerto serial y almaceno en buffer

//		/* se bloquea el mutex */
//		pthread_mutex_lock (&mutexData);

 /*si recibi mensajes imprimo e envio por el socket*/

		if( nbytesrecibidos > 0 && buffer[0]=='>')   //si los bytes recibidos y el primer caracter sea >
		{
		/* se imprimi un mensaje de recepción */
		//buffer[ nbytesrecibidos - 2 ] = '\0'; // se restan 2 unidades para eliminar "\r\n"
		printf( "\nrecibido por la uart: %s\n", buffer );
        printf( "bytes recibidos:%i\n", nbytesrecibidos );
        aux=buffer[14]-48;  //me dice el valor de la tecla pulsada
        printf( "Tecla Pulsada:%i\n",aux );
    
    /* verifico que las teclas pulsadas esten entre la 0,1,2,3*/
      if((aux>=0)&&(aux<=3))
		{
		  datoCompartido=aux;
		}
		else
		{
			datoCompartido=0;
		}
	    printf( "Dato compartido:%i\n",datoCompartido );

		serial_send(buffer,16);
		/* se desbloquea el mutex */
		//pthread_mutex_unlock (&mutexData);
        }
	
    }
	
	return NULL;
}