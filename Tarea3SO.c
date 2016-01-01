#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAXLINE 128
#define MAXSIZE 10

typedef struct{//Estructura utilizada para almacenar los datos del arreglo a utilizar con threads.
	char cadena[MAXLINE];
}linea;

//Estructura utilizada para almacenar datos de la cola
typedef struct { //cola de arreglos
	unsigned int maxlen;
	unsigned int len;
	unsigned int pos_primero;
	unsigned int pos_ultimo;
	linea *elemento;
}tcola;

// Funciones colas:

//Inicializar cola
void Initcola (tcola *C){
	C->len=C->pos_primero=C->pos_ultimo=0;
	C->maxlen=MAXSIZE;
	C->elemento=(linea *)malloc(MAXSIZE*sizeof(linea));
}

//Encolar un elemento de tipo linea
int Enqueue(tcola *C, linea elemento){
	if (C->len==C->maxlen){
		return -1; //no hay espacio disponible
	}
	if (C->pos_ultimo==C->maxlen){
		C->pos_ultimo=0; //La ultima posicion pasa a estar en la posicion 0 si llegamos al final del espacio
	}
	C->elemento[C->pos_ultimo++]=elemento;
	C->len++;
	return 1; //insercion exitosa
}
//Desencolar un elemento de tipo linea
linea Dequeue(tcola *C){
	linea aux;
	if (C->len > 0){
		if (C->pos_primero==C->maxlen) C->pos_primero=0;
		aux=C->elemento[C->pos_primero];
		C->pos_primero++;
		C->len--;
		return aux;
	}
	//linea problema;
	return Dequeue(C);
}

//Funcion para limpiar la memoria asociada a las colas
void Clear(tcola *C){
	C->len=C->pos_primero=C->pos_ultimo=0;
	C->maxlen=MAXSIZE;
	free((void *) C->elemento);
}
//Funcion para imprimir elemento por elemento en la cola.
void Printcola(tcola *C){
	int i;
	linea aux;
	for (i=0; i < 10; i++){
		aux=C->elemento[i];
		printf("elemento %d: %s", i, aux.cadena);
	}
	printf("\nlen: %d-----pos_primero: %d-----pos_ultimo: %d\n",C->len, C->pos_primero, C->pos_ultimo);
}
// fin funciones colas

void* ret;
pthread_mutex_t mutex; // MUTEX Global para que todos puedan acceder al mutex
tcola colaspaces,colaupper,colawriter; //colas que se usaran
int nlineas=0;


//Funcion que escribe en la salida estandar luego de utilizar las funciones anteriores
void *Writter (void * nada) { 
	char line[MAXLINE];
	linea recepcion;
	recepcion=Dequeue(&colawriter); //Se desencola el elemento frontal para imprimirlo
	strcpy(line,recepcion.cadena);
	printf("%s", line);
	nlineas++;
	if (strcmp(line,"")==0){ //""Flag" para saber si no hay mas lineas para leer
		printf("numero total de lineas= %d",nlineas-1);
		Clear(&colaspaces); Clear(&colaupper); Clear(&colawriter); // Se limpia la memoria una vez que no hay más lineas para leer
	}
	return NULL;
} 

//Funcion que cambia a mayusculas la linea recibida
void *Upper (void * nada){ 
	char line[MAXLINE], newline[MAXLINE];
	linea recepcion, envio;
    
	recepcion=Dequeue(&colaupper); //Se recibe la linea en la posicion frontal de la cola
	strcpy(line,recepcion.cadena);
	int i = 0;
	while(line[i]!='\0')
	{ 
		if (line[i]>=97 && line[i]<=122) newline[i]=line[i]-32;
		else if (line[i]==-79) newline[i]=-111;
		else newline[i]=line[i];
		i++;
	}
    strcpy(envio.cadena,newline);
    Enqueue(&colawriter, envio); //Se encola para utilizarse en la funcion writter
    
	pthread_t threadid;
	pthread_create(&threadid, NULL, Writter, NULL);
	return NULL;
} 

//Funcion para cambiar los espacios por asteriscos.
void *spaces(void * nada){
	int i;
	char line[MAXLINE], newline[MAXLINE];
	linea recepcion, envio;
    
//    printf("\n\n colaspaces antes de dequeue:\n");
//    Printcola(&colaspaces);
    
	recepcion=Dequeue(&colaspaces);//Desencolamos para utilizar el elemento frontal en esta funcion
	
    
	strcpy(line,recepcion.cadena);
	for(i=0;i<=strlen(line);i++){
		if(line[i]==' ') newline[i]='*';
		else newline[i]=line[i];
	}
    strcpy(envio.cadena,newline);
    Enqueue(&colaupper, envio);
    
	pthread_t threadid;
	pthread_create(&threadid, NULL, Upper, NULL);
	return NULL;
}
//Funcion para leer del archivo de entrada
void *reader(void * nada){
	char line[MAXLINE];
	linea envios;
	FILE *archivo;
	archivo= fopen("unarchivo.txt","r");
	Initcola(&colaspaces);
	Initcola(&colaupper);
	Initcola(&colawriter);
	if (archivo==NULL){
		printf("el archivo \"unarchivo.txt\" no se encuentra en el directorio");
		return NULL;
    }
	while(fgets(line, MAXLINE, archivo)!=NULL){ //Se lee linea por linea
		pthread_attr_t attr; // Se inicializa el atributo joinable necesario para conservar orden de threads
	    pthread_attr_init(&attr);
	    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		pthread_mutex_init(&mutex, NULL); // Inicializamos el mutex
		pthread_t threadid;
	    strcpy(envios.cadena,line);
	    Enqueue(&colaspaces, envios); //Encolamos para mandar esta linea al thread spaces
		pthread_create(&threadid, NULL, spaces, NULL); 
		pthread_join(threadid, &ret); //Esperamos que termine el thread para continuar
		
	    pthread_attr_destroy(&attr); // Borramos de la memoria los atributos

	    /* Antes de llamar a esta función TODOS los threads que usen el mutex tienen que estar cerrados. */
		pthread_mutex_destroy(&mutex); // Desinicializa el mutex
    }
    linea flag;
	strcpy(flag.cadena,""); //Flag para saber si terminamos de leer
    Enqueue(&colaspaces, flag);
	pthread_t threadid;
	pthread_create(&threadid, NULL, spaces, NULL);
    //pthread_exit(NULL);
//        spaces(line);
//        printf("%s", linea);
	fclose(archivo);
	return NULL;
}

int main (int argc, char *argv[])
{   //Inicializamos la thread de la funcione reader
	pthread_t threadid;
	pthread_create(&threadid, NULL, reader, NULL);
	pthread_exit(NULL);
	return 0;
}

