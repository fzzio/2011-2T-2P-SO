 /*			PROYECTO DE SISTEMAS OPERATIVOS
 *			    II Parcial - 2011 2T
 * 
 * INTEGRANTES:
 * 	- Andrea Obando N.
 * 	- Fabricio Orrala P.
 * -----------------------------------------------------------------------
 * ARCHIVO:
 *      proyecto.c
 * 
 * DESCRIPCION:
 * 	Este archivo contiene las implementacion del proyecto de
 *	sincronizacion.
 * 
 * OBSERVACIONES:
 *	Para compilar:
 *		gcc -g -o sincronizacion proyecto.c -lpthread -Wall
 *	Para ejecutar:
 *		./sincronizacion
 *	Para limpiar:
 *		rm -rf ./sincronizacion
 */

#include <errno.h>
#include <math.h> 
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_BODEGA 3
#define TAM_BUFFER 10

//Declaracion de constantes de materiales
#define MAT_FORMICA 0
#define MAT_GOMA 1
#define MAT_MADERA 2

int tb_formica = 0, tb_goma = 0, tb_madera = 0; // Total tableros

// Recursos
int rec_insumos = 0;
int int_insumos[3] = {0};
char *str_insumos[3] = {"FORMICA", "GOMA", "MADERA"};
int rec_bodega[MAX_BODEGA] = {0}; //Bodega donde se guarda el material
int count = 0;

// Funciones
void *producirTablero(void *material);

int verificaRecursosBodega(int recursoA, int recursoB);
void almacenarRecurso(int recurso);
void tomarRecurso(int recurso);

// Semaforo binario
pthread_mutex_t fabrica_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condicion_bodega = PTHREAD_COND_INITIALIZER;

// Programa Principal
int main (int argc, char *argv[]){
	pthread_t pFactoriaF, pFactoriaG, pFactoriaM; //Procesos Factorias (Formica, Goma, Madera)
	pthread_attr_t attr;

	system("clear;");
	printf("\n\t\t* * * Sistemas Operativos - Proyecto del II Parcial * * *\n\n");
	printf("\t- Ingrese numero de unidades de insumos (Formica, Goma, Madera): "); scanf("%d", &rec_insumos);
	int_insumos[MAT_FORMICA] = rec_insumos;
	int_insumos[MAT_GOMA] = rec_insumos;
	int_insumos[MAT_MADERA] = rec_insumos;
	
	pthread_mutex_init(&fabrica_lock, NULL);
 	pthread_cond_init (&condicion_bodega, NULL);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	// Creando hilos
	pthread_create(&pFactoriaF, &attr, producirTablero, (void *)MAT_FORMICA);
	pthread_create(&pFactoriaG, &attr, producirTablero, (void *)MAT_GOMA);
	pthread_create(&pFactoriaM, &attr, producirTablero, (void *)MAT_MADERA);

	// Inicializando hilos
	pthread_join(pFactoriaF, NULL);
	pthread_join(pFactoriaG, NULL);
	pthread_join(pFactoriaM, NULL);

	// Resultados
	printf("\n\n======================================================\n");
	printf("\t\tRESULTADOS\n");
	printf("======================================================\n");
	printf("Produccion de Tableros:\n");
	printf("- La factoria Formica: %d tableros.\n", tb_formica);
	printf("- La factoria Goma: %d tableros.\n", tb_goma);
	printf("- La factoria Madera: %d tableros.\n", tb_madera);
	printf("\nMaterial excedente:\n");
	printf("- Sobro %d unidades de Formica\n", int_insumos[MAT_FORMICA]);
	printf("- Sobro %d unidades de Goma\n", int_insumos[MAT_GOMA]);
	printf("- Sobro %d unidades de Madera\n", int_insumos[MAT_MADERA]);
	printf("======================================================\n");
	
	// Limpiar y Destruir
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&fabrica_lock);
	pthread_cond_destroy(&condicion_bodega);
	pthread_exit(NULL);
	return (0);
}



void *producirTablero(void *material){
	int mat = (long) material; //Se extrae el material
	
	// mientras existan recursos
	while(int_insumos[MAT_FORMICA] > 0 && int_insumos[MAT_GOMA] > 0 && int_insumos[MAT_MADERA] > 0){
		printf("\n\tDISPONIBLE--> Formica = %d, Goma = %d, Madera = %d.\t||", int_insumos[MAT_FORMICA], int_insumos[MAT_GOMA], int_insumos[MAT_MADERA]);
		printf("\tBODEGA--> %d Formica, %d Goma, %d Madera\n", rec_bodega[MAT_FORMICA], rec_bodega[MAT_GOMA], rec_bodega[MAT_MADERA]);
		
		switch(mat){
			case MAT_FORMICA:
				printf("\n\tProduciendo [%s]\n", str_insumos[mat]);
				pthread_mutex_lock(&fabrica_lock);					
					if(verificaRecursosBodega(MAT_GOMA, MAT_MADERA) == 1){
						printf("\t\t\t*Hay recursos, produce tablero Formica*\n");
						if(tb_formica == rec_insumos){
							pthread_cond_wait(&condicion_bodega, &fabrica_lock);
						}else{
							rec_bodega[MAT_FORMICA] --;
							rec_bodega[MAT_GOMA] --;
							rec_bodega[MAT_MADERA] --;
							tb_formica ++;
						}						
					}else{
						printf("\t\t\t*No hay recursos, SE ALMACENA Formica en bodega*\n");
						tomarRecurso(MAT_FORMICA);
						almacenarRecurso(MAT_FORMICA);
					}
				pthread_mutex_unlock(&fabrica_lock);
				sleep(1);
				break;
			case MAT_GOMA:
				printf("\n\tProduciendo [%s]\n", str_insumos[mat]);
				pthread_mutex_lock(&fabrica_lock);
					if(verificaRecursosBodega(MAT_FORMICA, MAT_MADERA) == 1){
						printf("\t\t\t*Hay recursos, produce tablero Goma*\n");
					   	if(tb_goma == rec_insumos){
							pthread_cond_wait(&condicion_bodega, &fabrica_lock);
						}else{
							rec_bodega[MAT_FORMICA] --;
							rec_bodega[MAT_GOMA] --;
							rec_bodega[MAT_MADERA] --;
							tb_goma ++;
						}
					}else{
						printf("\t\t\t*No hay recursos, SE ALMACENA Goma en bodega*\n");
						tomarRecurso(MAT_GOMA);
						almacenarRecurso(MAT_GOMA);
					}
				pthread_mutex_unlock(&fabrica_lock);
				sleep(1);
				break;
			case MAT_MADERA:
				printf("\n\tProduciendo [%s]\n", str_insumos[mat]);
				pthread_mutex_lock(&fabrica_lock);
					if(verificaRecursosBodega(MAT_FORMICA, MAT_GOMA) == 1){
						printf("\t\t\t*Hay recursos, produce tablero Madera*\n");
					   	if(tb_goma == rec_insumos){
							pthread_cond_wait(&condicion_bodega, &fabrica_lock);
						}else{
							rec_bodega[MAT_FORMICA] --;
							rec_bodega[MAT_GOMA] --;
							rec_bodega[MAT_MADERA] --;
							tb_madera ++;
						}
					}else{
						printf("\t\t\t*No hay recursos, SE ALMACENA Madera en bodega*\n");
						tomarRecurso(MAT_MADERA);
						almacenarRecurso(MAT_MADERA);
					}
				pthread_mutex_unlock(&fabrica_lock);
				sleep(1);
				break;
		}
	}
	pthread_exit(NULL);
}

int verificaRecursosBodega(int recursoA, int recursoB){
	int resp = 0;
		if((recursoA == MAT_GOMA && recursoB == MAT_MADERA) && (rec_bodega[MAT_GOMA]>0 && rec_bodega[MAT_MADERA]>0)){
			resp = 1;
		}else if((recursoA == MAT_FORMICA && recursoB == MAT_MADERA) && (rec_bodega[MAT_FORMICA]>0 && rec_bodega[MAT_MADERA]>0)){
			resp = 1;
		}else if((recursoA == MAT_FORMICA && recursoB == MAT_GOMA) && (rec_bodega[MAT_FORMICA]>0 && rec_bodega[MAT_GOMA]>0)){
			resp = 1;
		}
	return resp;
}

void almacenarRecurso(int recurso){
	pthread_cond_signal(&condicion_bodega);
	switch(recurso){
		case MAT_FORMICA:
			rec_bodega[MAT_FORMICA] ++;
			break;
		case MAT_GOMA:
			rec_bodega[MAT_GOMA] ++;
			break;
		case MAT_MADERA:
			rec_bodega[MAT_MADERA] ++;
			break;
	}
	return;
}

void tomarRecurso(int recurso){
	switch(recurso){
		case MAT_FORMICA:
			int_insumos[MAT_FORMICA] --;
			break;
		case MAT_GOMA:
			int_insumos[MAT_GOMA] --;
			break;
		case MAT_MADERA:
			int_insumos[MAT_MADERA] --;
			break;
	}
	return;
}
