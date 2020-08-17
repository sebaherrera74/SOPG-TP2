#include <stdio.h>
#include <stdlib.h>
#include "SerialManager.h"
#include <stdbool.h>

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
volatile sig_atomic_t salida = 1;	// Bandera para salir del bucle cuando hay una señal 
int newfd,s;
int datoCompartido;                 //
pthread_t threadEduCiaa;  //Thread para recibir datos del puertos serie de la edu-ciaa 
pthread_t threadTcp;   //Thread para comunicacion con el cliente TCP  
int n, nbytesrecibidos;;
char bufferSerial[BUFFER_MAZ_SIZE];
char buffersocket[BUFFER_MAZ_SIZE];
char bufferaux[BUFFER_MAZ_SIZE];  
bool conect=true;


/* declaramos las estructuras y variables para el socket */
   socklen_t addr_len;;
   struct sockaddr_in clientaddr;
   struct sockaddr_in serveraddr;

int main(void)
{   
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

//lanzamos thread para tarea TCP de acuerdo a consideraciones del enunciado*/
if (ThreadTCP(threadTcp))
	{
	perror(ERROR_MSGE_TREAHDTCP);
	exit(1);
    }
	else
	{
	printf("Thread creado correctamente\n");
	}

/*lanzamos thread de serial edu-ciaa -No lo use cambie la forma que venia relizando el tp*/
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
    int aux,m;
	nbytesrecibidos = serial_receive( buffer, BUFFER_MAZ_SIZE ); //leo puerto serial y almaceno en buffer
        
	if (nbytesrecibidos>0)
	{
		if (!memcmp(buffer,">TOGGLE STATE:",strlen(">TOGGLE STATE:"))) //comparo el buffer recibido sea de la pulsacion de la educiaa
		{
	    sscanf(buffer,">TOGGLE STATE:%d",&aux); // cargo en el buffer el formato y le agrego  
    	printf("\n%s\n",buffer);
	    printf( "bytes recibidos:%i\n", nbytesrecibidos );
	    
	    sprintf(buffer,":LINE%dTG\n",aux);
        if (write(newfd,buffer,strlen(buffer))==-1)  //envio al fd el buffer con el formato pedido en el tp
	    {
         perror("error escribiendo en socket");
	     exit(1);
	    }    
	}
	}	
/* tiempo de refresco del polling */
  usleep(10000);                   //Para que no sea bloqueante 
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
	salida=0;
	
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
	return 1;
   }
   return 0;
}

/* handler para el thread que establece comunicacion con el cliente TCP */
void * ComClienteTcp( void * parameters )
{
printf("Esperando conexiones\r\n"); 
while(salida) 
{	

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
  
    
	while((n = recv(newfd,buffersocket,120,0))!=0)
	{
		if (n==-1)
		{
		perror("Error leyendo mensaje en socket");
        exit(1);
		}

   		else if(buffersocket[0]==':')
		{   
			int out1,out2,out3,out4;  
			buffersocket[n]=0x00;
			out1=buffersocket[7]-'0';  //paso de caracter a int 
			out2=buffersocket[8]-'0';
			out3=buffersocket[9]-'0';
			out4=buffersocket[10]-'0';
		    sprintf(buffersocket,">OUTS:%d,%d,%d,%d\r\n",out1,out2,out3,out4);
			//bufferaux[15]=0x00;
			serial_send(buffersocket,strlen(buffersocket));
			printf("envio a educiaa: %s\n",buffersocket);
			   
		}
		else
		{
		/*imprimir que no es el formato*/
		printf("No es el formato del protocolo");
		n=0;
	}
	
}
 close(newfd);
}
return NULL;
}


/*---------No use esto en el TP-----------*/
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
		}
	
    }
	
	return NULL;
}