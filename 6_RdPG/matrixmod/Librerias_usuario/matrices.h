#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM (1024*10 -1)

// Estructura utilizada por libreria
struct matriz
{
	int **matriz;
	int filas;
	int columnas;
};

//Funciones de libreria
int **crear_matriz(struct matriz *x);
void liberar_mem(struct matriz *x);
void imprimir_matriz(struct matriz *x);
void sumar_matriz(struct matriz *c, struct matriz *x, struct matriz *y);
void restar_matriz(struct matriz *c, struct matriz *x, struct matriz *y);
void mult_matriz(struct matriz *c, struct matriz *x, struct matriz *y);
void transpuesta(struct matriz *c, struct matriz *x);
void identidad(struct matriz *c, int dimension);
void leer_fc_matriz(struct matriz *x, char *pfname);
void cargar_matriz(struct matriz *x, char *pfname);
void cargar_matriz_cero(struct matriz *x, int filas, int columnas);
void cout(char *cadena);




// Implementacion de funciones
/*
* Descripcion: Se libera memoria de una matriz, en base al parametro recibido.
* Parametros:
* @param *x: puntero de una estructura de tipo matriz que contiene el dato de las filas
* 			 y columnas de matriz a crear.
*
* @return: la direccion del doble puntero donde se encuentra la matriz creada.
*/
void liberar_mem(struct matriz *x)
{	
	int i = 0;
	if(x->filas > 0 || x->columnas > 0){
	// Reserva de memoria para columnas de m
	for (i=0;i<x->filas;i++) 
		free(x->matriz[i]);

	// Liberar memoria para filas m
	free(x->matriz);
	printf("Se libero memoria utilizada en matriz.\n");
	x->filas = 0;
	x->columnas = 0;
	}
	else
		printf("No se pudo liberar memoria.\n");
}

/*
* Descripcion de funcion: 
* Parametros:
* @param *x:
*/
void leer_fc_matriz(struct matriz *x, char *pfname)
{
	FILE *fp;
    char buffer[TAM]; // Arreglo que almacenara los datos del archivo leido (fn)
	char *fname = pfname;
	int filas=0; //
	int columnas = 0; //
	char *aux;

    	
    fp= fopen(fname,"r");// se abre archivo de nombre fname
   	if( !fp ){
      	printf( "Error al intetar abrir %s \n", fname);
	}
	
   	printf( "\n   --> Leyendo filas y columnas de matriz de incidencia (%s)...\n", fname);
	
	/*Leer linea por linea del contenido del archivo(fn) en el buffer */
   	while (fgets(buffer, TAM, fp) != NULL)
		filas++;

   	fclose(fp);// cierra archivo	
	
	aux = strtok(buffer, " "); // guardamos primera linea
	columnas =1;

	while( (aux = strtok( NULL, " ")) != NULL ) // Posteriores llamadas
	 columnas++;
	
	x->filas = filas;
	x->columnas = columnas;
}

/*
* Descripcion de funcion: 
* Parametros:
* @param **x: Doble puntero de matriz de incidencia.
*/

void cargar_matriz(struct matriz *x, char *pfname)
{
	// Se determinan filas columnas de matriz desde archivo	
	leer_fc_matriz(x, pfname);
	
	// Con las filas y columnas leidas se crea matriz alojando memoria dinamicamente
	x->matriz = crear_matriz(x);
	
	FILE *fp;
    char buffer[TAM]; // Arreglo que almacenara los datos del archivo leido (fn)
	char *fname = pfname;
	int filas=0; //
	int columnas =0; //
	char *aux;

    	
    fp= fopen(fname,"r");// se abre archivo de nombre fname
    if( !fp ){
	   	printf( "Error al intetar abrir %s \n", fname);
	}
	
	/*Leer linea por linea del contenido del archivo(fn) en el buffer */
   	while (fgets(buffer, TAM, fp) != NULL)
   	{
		aux = strtok(buffer, " "); // guardamos primera linea
		x->matriz[filas][columnas]=atoi(aux);
		columnas =1;
		
		while( (aux = strtok( NULL, " ")) != NULL ){   // Posteriores llamadas
			x->matriz[filas][columnas]=atoi(aux);
			columnas++;
		}
   		columnas=0;
		filas++;
   	}
   	fclose(fp);// cierra archivo
	
   	printf( "\n   --> Se cargo exitosamente la matriz de incidencia desde (%s).\n", fname);
}

/*
* Descripcion de funcion: 
* Parametros:
* @param **x: Doble puntero de matriz de incidencia.
*/

void cargar_matriz_cero(struct matriz *x, int filas, int columnas)
{
	int i,j;
	// Se determinan filas columnas de matriz desde matriz *y
	x->filas= filas;
	x->columnas= columnas;
	
	// Con las filas y columnas leidas de matriz y, alojando memoria dinamicamente
	x->matriz = crear_matriz(x);
	
	for(i=0; i<x->filas; i++)
	{
		for(j=0; j<x->columnas; j++)
			x->matriz[i][j]=0;
	}

}

/*
* Descripcion: Se aloja memoria para crear una matriz, en base al parametro recibido.
* Parametros:
* @param *x: puntero de una estructura de tipo matriz que contiene el dato de las filas
* 			 y columnas de matriz a crear.
*
* @return: la direccion del doble puntero donde se encuentra la matriz creada.
*/
int **crear_matriz(struct matriz *x)
{	
	int **m;
	
	int i = 0;
	int j = 0;

	// Reserva de Memoria para filas m
	m = (int **)malloc(x->filas*sizeof(int*));
	
	// Reserva de memoria para columnas de m
	for (i=0;i<x->filas;i++) 
		m[i] = (int*)malloc(x->columnas*sizeof(int));

	return m;
}


/*
* Descripcion: Funcion que obtiene la suma de dos matrices alojando el resultado en
* 			   en una nueva matriz.
* Parametros:
* @param *c: puntero de una estructura de tipo matriz en donde se almacenaran los
* 			 datos resultantes de la operacion suma.
* @param *x: puntero de una estructura de tipo matriz utilizada como el primer
* 			 operando de la operacion suma.
* @param *y: puntero de una estructura de tipo matriz utilizada como el segundo
* 			 operando de la operacion suma.
*/

void sumar_matriz(struct matriz *c, struct matriz *x, struct matriz *y)
{
	int i,j;
	c->filas = x->filas;
	c->columnas = x->columnas;
	c->matriz = crear_matriz(x);
	for(i=0; i<x->filas; i++){
		for(j=0; j<x->columnas; j++)	
		{
			c->matriz[i][j]= x->matriz[i][j] + y->matriz[i][j];
		}
	}
	
	//return c;
}

/*
* Descripcion: Funcion que obtiene la resta de dos matrices alojando el resultado en
* 			   en una nueva matriz.
* Parametros:
* @param *c: puntero de una estructura de tipo matriz en donde se almacenaran los
* 			 datos resultantes de la operacion resta.
* @param *x: puntero de una estructura de tipo matriz utilizada como el primer
* 			 operando de la operacion resta.
* @param *y: puntero de una estructura de tipo matriz utilizada como el segundo
* 			 operando de la operacion resta.
*/
void restar_matriz(struct matriz *c, struct matriz *x, struct matriz *y)
{
	int i,j;
	c->filas = x->filas;
	c->columnas = x->columnas;
	c->matriz = crear_matriz(x);
	for(i=0; i<x->filas; i++){
		for(j=0; j<x->columnas; j++)	
		{
			c->matriz[i][j]= x->matriz[i][j] - y->matriz[i][j];
		}
	}
}

/*
* Descripcion: Funcion que obtiene la multiplicacion de dos matrices alojando el resultado en
* 			   en una nueva matriz.
* Parametros:
* @param *c: puntero de una estructura de tipo matriz en donde se almacenaran los
* 			 datos resultantes de la operacion multiplicacion.
* @param *x: puntero de una estructura de tipo matriz utilizada como el primer
* 			 operando de la operacion multiplicacion.
* @param *y: puntero de una estructura de tipo matriz utilizada como el segundo
* 			 operando de la operacion multiplicacion.
*/
void mult_matriz(struct matriz *c, struct matriz *x, struct matriz *y)
{
	int i,j,k,total;
	total=0;
	c->filas = x->filas;
	c->columnas = y->columnas;
	c->matriz = crear_matriz(c);

	for(i=0; i<x->filas; i++){
		for(j=0; j<y->columnas; j++)
		{
			for(k=0; k<x->columnas; k++)
			{
				total+= x->matriz[i][k]*y->matriz[k][j];
			}
			c->matriz[i][j]= total;
			total=0;
		}
	}
}


/*
* Descripcion: Funcion que imprime matriz por pantalla.
* Parametros:
* @param *x: puntero de una estructura de tipo matriz utilizado para mostrar los
* 		     datos de la matriz.
*/
void imprimir_matriz(struct matriz *x)
{
	int i,j;
	// Dibujamos la Matriz en pantalla 
	for (i=0; i<x->filas; i++) 
	{ 
		printf("\n"); 
		for (j=0; j<x->columnas; j++){
		
			if(x->matriz[i][j] >= 0)
				printf("  %d", x->matriz[i][j] );
	
			else
				printf(" %d", x->matriz[i][j] );
		}
	}
	printf("\n\n");
}


/*
* Descripcion: Funcion que obtiene la traspuesta de una matriz.
* Parametros:
* @param *c: puntero de una estructura de tipo matriz en donde se almacenaran los
* 			 datos resultantes de la matriz transpuesta.
* @param *x: puntero de una estructura de tipo matriz utilizada para obtener la
* 			 matriz transpuesta.
*/
void transpuesta(struct matriz *c, struct matriz *x)
{
	int i,j,k,total;
	c->filas = x->columnas;
	c->columnas = x->filas;
	c->matriz = crear_matriz(c);
	
	for(i=0; i < x->columnas; i++)
	{
		for(j=0; j < x->filas; j++)
			c->matriz[i][j]= x->matriz[j][i];
	}
}


/*
* Descripcion: Funcion que obtiene la matriz identidad segun dimension.
* Parametros:
* @param *c: puntero de una estructura de tipo matriz en donde se almacenaran los
* 			 datos resultantes de la matriz identidad.
* @param dimension: entero que se utiliza como dimension de la matriz identidad
*					ha crear.
*/
void identidad(struct matriz *c, int dimension)
{
	int i,j,k,total;
	c->filas = dimension;
	c->columnas = dimension;
	c->matriz = crear_matriz(c);
	
	for(i=0; i < c->filas; i++)
	{
		for(j=0; j < c->columnas; j++)
		{
			if(i==j)
				c->matriz[i][j]= 1;

			else
				c->matriz[i][j]= 0;
		}
	}
}


/*
* Descripcion de funcion: 
* Parametros:
* @param *x:
*/
void cout(char *cadena)
{
	printf("%s", cadena);
}