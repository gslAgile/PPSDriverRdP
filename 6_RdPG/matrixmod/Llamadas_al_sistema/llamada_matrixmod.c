#include <string.h>
#include <fcntl.h> /* modos de apertura funciones ioctl*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

/* Librerias propias*/
#include "../Librerias_usuario/matrices.h"

#define TAM (1024*10 -1)

/* Variables globales*/


/* Funciones */
void driver_write(char cadena[256], int pfd);
void driver_read(char *cadena, int pfd);
void mostrar_I(char *cadena, int pfd);
void mostrar_H(char *cadena, int pfd);
void mostrar_MI(char *cadena, int pfd);
void mostrar_MA(char *cadena, int pfd);
void mostrar_MN(char *cadena, int pfd);
void mostrar_E(char *cadena, int pfd);
void disparar_trasnsicion(char * cadena, int pfd);
void matrixmod_crear(char pcomando[256], int f, int c, int pfd);
void matrixmod_add(char pcomando[256], struct matriz *x, int pfd);


int main()
{
	// Variables para file descriptor (de driver del kernel)
	int fd; // file descriptor para escribir/leer el driver
	char cadena[256];
	int n, opcion;

	// Variables matrices dinamicas
	struct matriz I;	// Matriz de incidencia
	struct matriz MI;	// Marcado inicial actual
	struct matriz MN;	// Marcado nuevo
	struct matriz H; 	// Matriz de incidencia H de brazos inhibidores
	char fname1[256];
	char fname2[256];
	
	/* Se establece el nombre asociado a cada uno de los vectores y matrices*/
  	/*strcpy(I.nombre, "Matriz I");
	strcpy(MI.nombre, "Vector MI");
	strcpy(H.nombre, "Vector H");*/

	/* Carga de matrices desde archivos*/
	/*cargar_matriz(&I, "MI.txt");
	cargar_matriz(&MI, "MA.txt");
	cargar_matriz(&H, "H.txt");*/

	printf("   --> La matriz leida es de dimension [%u]x[%u]", I.filas, I.columnas);

	
	/* Abrimos modulo matrixmod*/
	fd = open("/proc/matrixmod", O_RDWR); // abrimos file descriptor en modo escritura/lectura

	if(fd<0 ) /* Verificacion de creacion*/
	{
		//error
		perror("Error al abrir file descriptor.");
		exit(1);
	}

	// Creacion de matrices en modulo
	printf("\n\n");

	// Creamos RdP I en matrixmod
	matrixmod_crear("crear I ",I.filas,I.columnas,fd);
	// Cargamos los valores de la RdP I al modulo matrixmod
	matrixmod_add("add I ", &I, fd);

	sleep(2); // esperamos 2 segundos para evitar problemas de concurrencia en el kernel
	
	// Creamos RdP MI en matrixmod
	matrixmod_crear("crear MI ",MI.filas,MI.columnas,fd);
	// Cargamos los valores de la RdP MI al modulo matrixmod
	matrixmod_add("add MI ", &MI, fd);

	// Creamos matriz Mde incidencia H en matrixmod
	matrixmod_crear("crear H ",H.filas,H.columnas,fd);
	// Cargamos los valores de la RdP MI al modulo matrixmod
	matrixmod_add("add H ", &H, fd);


	/* --- MENU DE OPCIONES ---	*/
    while ( opcion != 8 )
    {
        printf( "\n   >>>_			MENU 			_<<<\n");
        printf( "\n   1. Ver matriz de incidencia de la RdP.");
        printf( "\n   2. Ver matriz de incidencia H de la RdPG.");
        printf( "\n   3. Ver marcado inicial de la RdP.");
        printf( "\n   4. Ver marcado actual de la RdP.");
        printf( "\n   5. Ver marcado nuevo de la RdP.");
        printf( "\n   6. Ver vector E de transicicones sensibilizadas en la RdP.");
        printf( "\n   7. Disparar transicion sobre RdP.");
        printf( "\n   8. Salir." );
        printf( "\n\n   Su seleccion de opcion (1-8): ");

        scanf( "%d", &opcion );

        /* Inicio del anidamiento */

        switch ( opcion )
        {
            case 1: printf( "\n   Matriz de incidencia de la RdP:\n " );
                    mostrar_I(cadena, fd);
                    break;

            case 2: printf( "\n   Matriz de incidencia de la RdP:\n " );
                    mostrar_H(cadena, fd);
                    break;

            case 3: printf( "\n   Marcado inicial de la RdP:\n" );
                    mostrar_MI(cadena, fd);
                    break;

            case 4: printf( "\n   Marcado actual de la RdP:\n " );
                    mostrar_MA(cadena, fd);
                    break;

            case 5: printf( "\n   Marcado nuevo de la RdP:\n " );
                    mostrar_MN(cadena, fd);
                    break;

            case 6: printf( "\n   Vector E de transiciones sensibilizadas:\n " );
                    mostrar_E(cadena, fd);
                    break;

            case 7: printf( "\n   Introduzca comando de disparo de transicion: ");
            		scanf( "%s", cadena);
                    disparar_trasnsicion(cadena, fd);
                    break;

            case 8: printf( "\n   Saliendo de aplicacion.\n\n");
            		break;

            default: printf("\n   Comando no valido. Intente nuevamente segun opciones de menu.\n " );
         }

         /* Fin del anidamiento */

    }

    // Close de files descriptors asociado a modulo
	close(fd);
	liberar_mem(&I);
	liberar_mem(&MI);
	liberar_mem(&H);

    return 0;
}

/*
* @param pfd: identificador file descriptor como parametro
*/
void driver_write(char cadena[256], int pfd)
{	
	// Write fichero
	if (write(pfd, cadena, strlen(cadena)) != strlen(cadena))
	{
		printf("Error de escritura sobre driver\n");
		// Close de files descriptors
		close(pfd);
		exit(1);
	}

	//printf("Escritura exitosa sobre driver\n");
}

/*
* @param pfd: identificador file descriptor como parametro
*/
void driver_read(char *cadena, int pfd)
{
	// Read fichero
	if(read(pfd, cadena, 256) < 1)
	{
		printf("Error de lectura sobre driver\n");
		// Close de files descriptors
		close(pfd);
		exit(1);
	}

	//printf("Lectura exitosa sobre driver\n");
}

/*
* @param pfd: identificador file descriptor como parametro
*/
void mostrar_I(char *cadena, int pfd)
{
	// Write sobre driver
	driver_write("mostrar I\n", pfd);

	// Read fichero
	driver_read(cadena, pfd);

	printf("\n%s\n", cadena);

	memset(cadena, '\0', 256);// se limpia cadena
}

/*
* @param pfd: identificador file descriptor como parametro
*/
void mostrar_H(char *cadena, int pfd)
{
	// Write sobre driver
	driver_write("mostrar H\n", pfd);

	// Read fichero
	driver_read(cadena, pfd);

	printf("\n%s\n", cadena);

	memset(cadena, '\0', 256);// se limpia cadena
}


void mostrar_MI(char *cadena, int pfd)
{
	// Write sobre driver
	driver_write("mostrar MI\n", pfd);

	// Read fichero
	driver_read(cadena, pfd);

	printf("\n%s\n", cadena);

	memset(cadena, '\0', 256);// se limpia cadena
}


void mostrar_MA(char *cadena, int pfd)
{
	// Write sobre driver
	driver_write("mostrar MA\n", pfd);

	// Read fichero
	driver_read(cadena, pfd);

	printf("\n%s\n", cadena);

	memset(cadena, '\0', 256);// se limpia cadena
}


void mostrar_MN(char *cadena, int pfd)
{
	// Write sobre driver
	driver_write("mostrar MN\n", pfd);

	// Read fichero
	driver_read(cadena, pfd);

	printf("\n%s\n", cadena);

	memset(cadena, '\0', 256);// se limpia cadena
}


void mostrar_E(char *cadena, int pfd)
{
	// Write sobre driver
	driver_write("mostrar E\n", pfd);

	// Read fichero
	driver_read(cadena, pfd);

	printf("\n%s\n", cadena);

	memset(cadena, '\0', 256);// se limpia cadena
}


void disparar_trasnsicion(char *cadena, int pfd)
{
	//
	char MN[256]; // cadena para almacenar marcado nuevo
	char MA[256]; // cadena para almacenar marcado actual
	char comando[256] = "STEP_CMD_"; // comando para realizar disparo

	strcat(comando, cadena); // se agrega cadena(nro transicion) al final de comando

	printf("Cadena ingresada: %s\n\n", comando);
	// Write sobre driver -> se realiza disparo
	driver_write(comando, pfd);

	// indicamos a driver que muestre MN
	driver_write("mostrar MN\n", pfd);

	// almacenamos read en MN
	driver_read(MN, pfd);

	// indicamos a driver que muestre MA
	driver_write("mostrar MA\n", pfd);

	// almacenamos read en MA
	driver_read(MA, pfd);

	// Comparamos resultados para verificar si disparo fallo o no
	if(strcmp(MA, MN) == 0)
		printf("\n	-->	Disparo exitoso.\n");

	else
		printf("\n	-->	Disparo no exitoso.\n");


	memset(cadena, '\0', 256);// se limpia cadena
}

/*
* Descripcion de funcion: 
* Parametros:
* pfd: parametro de numero de file descriptor de modulo matrixmod
* pcomando : cadena que indica comando
* int f: entero que indica filas de matriz a crear
* int c: entero que indica columnas de matriz a crear
*/

void matrixmod_crear(char pcomando[256], int f, int c, int pfd)
{
	char comando[256];
	memset(comando, '\0', 256);// se limpia cadena

	// Se copia parametro a cadena comando
	strcpy(comando, pcomando);
	char numeros[256];
	memset(numeros, '\0', 256);// se limpia cadena
	
	// Se carga en cadena numeros los numeros enteros correspondientes a las filas y columnas
	sprintf(numeros, "_%d_%d", f, c);

	// Se almacena todo en el comando para dejarlo completo
	strcat(comando, numeros);

	printf("Se ejecuta en modulo comando: %s\n", comando);

	// Se crea matriz en modulo con comando write
	driver_write(comando, pfd);
}

/*
* Descripcion de funcion: 
* Parametros:
*/

void matrixmod_add(char pcomando[256], struct matriz *x, int pfd)
{
	int i,j;
	char comando[256];
	memset(comando, '\0', 256);// se limpia cadena

	// Se copia parametro a cadena comando
	strcpy(comando, pcomando);
	char valor_aux[256];
	memset(valor_aux, '\0', 256);// se limpia cadena
	
	for(i = 0; i < x->filas; i++)
	{
		for (j = 0; j < x->columnas; j++)
		{
			// Se carga en cadena numeros los numeros enteros correspondientes a las filas y columnas
			sprintf(valor_aux, "%d_%d_%d", i, j, x->matriz[i][j]);

			// Se almacena todo en el comando para dejarlo completo
			strcat(comando, valor_aux);

			printf("Se ejecuta en modulo comando: %s\n", comando);
			// Se crea matriz en modulo con comando write
			driver_write(comando, pfd);

			memset(comando, '\0', 256);// se limpia cadena

			// Se copia parametro a cadena comando
			strcpy(comando, pcomando);
			memset(valor_aux, '\0', 256);// se limpia cadena
		}
	}
}