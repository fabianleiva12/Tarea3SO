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

typedef struct{//Estructura utilizada para almacenar los datos del arreglo a utilizar con threads.
	char cadena[MAXLINE];
}linea;

//IMPLEMENTACION DE LA COLA ********************************

struct Node {
	linea data;
	struct Node* next;
};
// Two glboal variables to store address of front and rear nodes. 
struct Node* front = NULL;
struct Node* rear = NULL;

// To Enqueue an integer
void Enqueue(linea x) {
	struct Node* temp = 
		(struct Node*)malloc(sizeof(struct Node));
	temp->data =x; 
	temp->next = NULL;
	if(front == NULL && rear == NULL){
		front = rear = temp;
		return;
	}
	rear->next = temp;
	rear = temp;
}

// To Dequeue an integer.
void Dequeue() {
	struct Node* temp = front;
	if(front == NULL) {
		printf("Queue is Empty\n");
		return;
	}
	if(front == rear) {
		front = rear = NULL;
	}
	else {
		front = front->next;
	}
	free(temp);
}

linea Front() {
	if(front == NULL) {
		printf("Queue is empty\n");
		return front->data;
	}
	return front->data;
}

void Print() {
	struct Node* temp = front;
	while(temp != NULL) {
		printf("%s ",temp->data.cadena);
		temp = temp->next;
	}
	printf("\n");
}


//FIN DE IMPLEMENTACION DE LA COLA

void Upper (char texto[MAXLINE]) 
{ 
	int i = 0;
    while(texto[i]!='\0') 
	{ 
		texto[i]-=32; 
		printf("%c",texto[i]); 
		i++; 
	} 
	printf("\n");
} 

void Writter (void *parametro) 
{ 
	char line[MAXLINE];
	strcpy(line,((linea *)parametro)->cadena);
	printf("%s\n", line);
} 

void *spaces(void *parametro){
	int i;
	char line[MAXLINE];
	strcpy(line,((linea *)parametro)->cadena);
	char newline[MAXLINE];
	for(i=0;i<=strlen(line);i++){
		if(line[i]==' ') newline[i]='*';
		else newline[i]=line[i];
	}
	//enviar nuevalinea a un thread upper
	printf("%s", newline);
	return NULL;
}

void *reader(void * nada){
	char line[MAXLINE];
	FILE *archivo;
	archivo= fopen("unarchivo.txt","r");
	if (archivo==NULL){
		printf("el archivo \"unarchivo.txt\" no se encuentra en el directorio");
		return NULL;
    }
	while(fgets(line, MAXLINE, archivo)!=NULL){
        //aqui mandar la linea a un thread que la modifique
        pthread_t threadid;
		linea *parametro=(linea *)malloc(sizeof(linea));
		strcpy(parametro->cadena,line);
		pthread_create(&threadid, NULL, spaces, parametro);
//        spaces(line);
//        printf("%s", linea);
    }
	fclose(archivo);
	return NULL;
}

int main (int argc, char *argv[])
{
	pthread_t threadid;
	pthread_create(&threadid, NULL, reader, NULL);	
	pthread_exit(NULL);

}


