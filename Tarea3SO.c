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

typedef struct { //cola de arreglos
	unsigned int maxlen;
	unsigned int len;
	unsigned int pos_primero;
	unsigned int pos_ultimo;
	linea *elemento;
}tcola;

// funciones colas
void Initcola (tcola *C){
	C->len=C->pos_primero=C->pos_ultimo=0;
	C->maxlen=MAXSIZE;
	C->elemento=(linea *)malloc(MAXSIZE*sizeof(linea));
}

int Size(tcola *C){
	return C->len;
}

int Enqueue(tcola *C, linea elemento){
	if (C->len==C->maxlen){
		return -1; //no hay espacio disponible
	}
	if (C->pos_ultimo==C->maxlen){
		C->pos_ultimo=0;
	}
	C->elemento[C->pos_ultimo++]=elemento;
	C->len++;
	return 1; //inserccion exitosa
}

linea Dequeue(tcola *C){
	linea aux;
	if (C->len > 0){
		aux=C->elemento[C->pos_primero];
		if (C->pos_primero==C->maxlen) C->pos_primero=0;
		C->pos_primero++;
		C->len--;
		return aux;
	}
	return aux;
}

void Clear(tcola *C){
	C->len=C->pos_primero=C->pos_ultimo=0;
	C->maxlen=MAXSIZE;
	free((void *) C->elemento);
}
// fin funciones colas

void* ret;
pthread_mutex_t mutex; // MUTEX Global para que todos puedan acceder al mutex
tcola colaspaces,colaupper,colawriter; //colas que se usaran

void *Writter (void * nada) { 
	char line[MAXLINE];
	linea recepcion;
	recepcion=Dequeue(&colawriter);
	strcpy(line,recepcion.cadena);
	printf("%s", line);
	//Clear(&colaspaces); Clear(&colaupper); Clear(&colawriter);
	return NULL;
} 

void *Upper (void * nada){ 
	char line[MAXLINE], newline[MAXLINE];
	linea recepcion, envio;
	recepcion=Dequeue(&colaupper);
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
    Enqueue(&colawriter, envio);
    
    //esto es para mandar un thread a writer
//    pthread_mutex_init(&mutex, NULL);
	pthread_t threadid;
	pthread_create(&threadid, NULL, Writter, NULL);
	
//	pthread_join(threadid, &ret);
//	pthread_mutex_destroy(&mutex);
//	printf("%s",newline);
	return NULL;
} 

void *spaces(void * nada){
	int i;
	char line[MAXLINE], newline[MAXLINE];
	linea recepcion, envio;
	recepcion=Dequeue(&colaspaces);
	strcpy(line,recepcion.cadena);
	for(i=0;i<=strlen(line);i++){
		if(line[i]==' ') newline[i]='*';
		else newline[i]=line[i];
	}
    strcpy(envio.cadena,newline);
    Enqueue(&colaupper, envio);
	//enviar nuevalinea a un thread upper
//	pthread_mutex_init(&mutex, NULL);
	pthread_t threadid;
	pthread_create(&threadid, NULL, Upper, NULL);
	
//	pthread_join(threadid, &ret);
//	pthread_mutex_destroy(&mutex); // Desinicializa el mutex
//	printf("%s", newline);
	return NULL;
}

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
	while(fgets(line, MAXLINE, archivo)!=NULL){
		pthread_attr_t attr;
	    pthread_attr_init(&attr);
	    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		pthread_mutex_init(&mutex, NULL); // Inicializamos el mutex
		pthread_t threadid;

		if (Size(&colaspaces)<10){
			//Se puede encolar y trabajar con la linea directamente
	        strcpy(envios.cadena,line);
	        Enqueue(&colaspaces, envios);
			pthread_create(&threadid, NULL, spaces, NULL);
			pthread_join(threadid, &ret);	
		}
		else{
			//Se encola una vez que la linea actual se proceso y se desencolo en las funciones siguientes.
			pthread_create(&threadid, NULL, spaces, NULL);
			pthread_join(threadid, &ret);//Esperamos que termine el hilo antes de encolar de nuevo.
			strcpy(envios.cadena,line);
	        Enqueue(&colaspaces, envios);
		}
	    pthread_attr_destroy(&attr); // Borramos de la memoria los atributos

	    /* Antes de llamar a esta función TODOS los threads que usen el mutex tienen que estar cerrados. */
		pthread_mutex_destroy(&mutex); // Desinicializa el mutex

    }
    pthread_exit(NULL);
//        spaces(line);
//        printf("%s", linea);
	fclose(archivo);
	return NULL;
}

int main (int argc, char *argv[])
{
	pthread_t threadid;
	pthread_create(&threadid, NULL, reader, NULL);
	pthread_exit(NULL);
	return 0;
}
