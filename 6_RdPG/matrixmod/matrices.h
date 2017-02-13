//#include <stdio.h>
//#include <linux/stdlib.h>
#include <linux/string.h>
#include <linux/vmalloc.h>

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
void cargar_matriz_cero(struct matriz *x, int filas, int columnas);
void cargar_matriz_uno(struct matriz *x, int filas, int columnas);
void sumar_matriz(struct matriz *c, struct matriz *x, struct matriz *y);
void restar_matriz(struct matriz *c, struct matriz *x, struct matriz *y);
void mult_matriz(struct matriz *c, struct matriz *x, struct matriz *y);
void transpuesta(struct matriz *c, struct matriz *x);
void transpuesta_fc(struct matriz *c, struct matriz *x, int f, int col, int id_d);
void identidad(struct matriz *c, int dimension);

// Implementacion de funciones

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

	// Reserva de Memoria para filas m
	m = (int **)vmalloc(x->filas*sizeof(int*));
	
	// Reserva de memoria para columnas de m
	for (i=0;i<x->filas;i++) 
		m[i] = (int*)vmalloc(x->columnas*sizeof(int));

	return m;
}

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
		vfree(x->matriz[i]);

	// Liberar memoria para filas m
	vfree(x->matriz);
	printk(KERN_INFO "matrixmod: Se libero memoria utilizada en matriz.\n");
	x->filas = 0;
	x->columnas = 0;
	}
	else
		printk(KERN_INFO "matrixmod: No se pudo liberar memoria.\n");
}

/*
* Descripcion de funcion: 
* Parametros:
* @param **x: Doble puntero de matriz de incidencia.
*/
void limpiar_matriz(struct matriz *x)
{
	int i,j;
	if(x->filas > 0 && x->columnas > 0)
	{
		for(i=0; i<x->filas; i++)
		{
			for(j=0; j<x->columnas; j++)
				x->matriz[i][j]= 0;
		}
	}
}

/*
* Descripcion de funcion: 
* Parametros:
* @param **x: Doble puntero de matriz de incidencia.
*/
void cargar_matriz_cero(struct matriz *x, int filas, int columnas)
{
	int i,j;
	// En caso de no estar asignadas las filas y columnas con anterioridad
	if(x->filas == 0 || x->columnas == 0){
		// Se determinan filas columnas de matriz segun parametros
		x->filas= filas;
		x->columnas= columnas;
	}
	
	// Con las filas y columnas leidas de matriz y, alojando memoria dinamicamente
	x->matriz = crear_matriz(x);
	
	for(i=0; i<x->filas; i++)
	{
		for(j=0; j<x->columnas; j++)
			x->matriz[i][j]= 0;
	}
}

/*
* Descripcion de funcion: 
* Parametros:
* @param **x: Doble puntero de matriz de incidencia.
*/
void cargar_matriz_uno(struct matriz *x, int filas, int columnas)
{
	int i,j;
	// En caso de no estar asignadas las filas y columnas con anterioridad
	if(x->filas == 0 || x->columnas == 0){
		// Se determinan filas columnas de matriz segun parametros
		x->filas= filas;
		x->columnas= columnas;
	}
	
	// Con las filas y columnas leidas de matriz y, alojando memoria dinamicamente
	x->matriz = crear_matriz(x);
	
	for(i=0; i<x->filas; i++)
	{
		for(j=0; j<x->columnas; j++)
			x->matriz[i][j]= 1;
	}
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
/*void imprimir_matriz(struct matriz *x)
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
}*/

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
	int i,j;
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
* Descripcion: Funcion que obtiene la traspuesta de una matriz en base a los parametros
* de f (filas) y c (columnas) recibidos.
* Parametros:
* @param *c: puntero de una estructura de tipo matriz en donde se almacenaran los
* 			 datos resultantes de la matriz transpuesta.
* @param *x: puntero de una estructura de tipo matriz utilizada para obtener la
* 			 matriz transpuesta.
* @param f: numero de filas deseado.
* @param  c: numero de columnas deseado.
* @param  id_d: identificador de vector disparo (fila de matriz mdisparos).

*/
void transpuesta_fc(struct matriz *c, struct matriz *x, int f, int col, int id_d)
{
	int i,j;
	if(f <= x->columnas && col <= x->filas)
	{
		c->filas = f;
		c->columnas = col;
		c->matriz = crear_matriz(c);

		for(i=0; i < c->filas; i++)
		{
			for(j=0; j < c->columnas; j++)
				c->matriz[i][j]= x->matriz[id_d][i];
		}
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
	int i,j;
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

