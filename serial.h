


/**DEFINES**/
#define BUFFER_MAZ_SIZE     120   //Tamaño maximo del buffer
#define PORT_NUMBER        10000  //Puerto de conexion del socket
#define UART_NUM            1     //Identificador del puerto serie utilizado 
#define UART_BAUDRATE     115200  //Baudrate del puerto serie 
#define TTY_TEXTO               "/dev/ttyUSB1"


extern int OpenSerie (void);

extern int ThreadEduciaa(pthread_t * threadciaa);
