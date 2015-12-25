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


//IMPLEMENTACION DE LA COLA ********************************

struct Node {
	int data;
	struct Node* next;
};
// Two glboal variables to store address of front and rear nodes. 
struct Node* front = NULL;
struct Node* rear = NULL;

// To Enqueue an integer
void Enqueue(int x) {
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

int Front() {
	if(front == NULL) {
		printf("Queue is empty\n");
		return;
	}
	return front->data;
}

void Print() {
	struct Node* temp = front;
	while(temp != NULL) {
		printf("%d ",temp->data);
		temp = temp->next;
	}
	printf("\n");
}


//FIN DE IMPLEMENTACION DE LA COLA


struct linea{//Estructura utilizada para almacenar los datos del arreglo a utilizar con threads.
	char cadena[MAXLINE];
	};


void GetInput()
{
	//char str1[20];
	//printf("Introduce cadena de caracteres: ");
   	//scanf("%s", &str1);
}

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


int main (int argc, char *argv[])
{
	char str[MAXLINE] = "caca";
	Upper(str);

}