#include <string.h>
#include <fcntl.h> /* modos de apertura funciones ioctl*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

/* Variables globales*/


/* Funciones */
void driver_write(char cadena[256], int pfd);
void driver_read(char *cadena, int pfd);
void mostrar_I(char *cadena, int pfd);
void mostrar_MI(char *cadena, int pfd);
void mostrar_MA(char *cadena, int pfd);
void mostrar_MN(char *cadena, int pfd);
void disparar_trasnsicion(char * cadena, int pfd);


int main()
{
	int fd; // file descriptor para escribir/leer el driver
	char cadena[256];
	int n, opcion;

	/* Abrimos modulo matrixmod*/
	fd = open("/proc/matrixmod", O_RDWR); // abrimos file descriptor en modo escritura/lectura

	if(fd<0 ) /* Verificacion de creacion*/
	{
		//error
		perror("Error al abrir file descriptor.");
		exit(1);
	}

	/* --- MENU DE OPCIONES ---	*/
    while ( opcion != 6 )
    {
        printf( "\n   >>>_			MENU 			_<<<\n");
        printf( "\n   1. Ver matriz de incidencia de la RdP.");
        printf( "\n   2. Ver marcado inicial de la RdP.");
        printf( "\n   3. Ver marcado actual de la RdP.");
        printf( "\n   4. Ver marcado nuevo de la RdP.");
        printf( "\n   5. Disparar transicion sobre RdP.");
        printf( "\n   6. Salir." );
        printf( "\n\n   Introduzca opcion (1-6): ");

        scanf( "%d", &opcion );

        /* Inicio del anidamiento */

        switch ( opcion )
        {
            case 1: printf( "\n   Matriz de incidencia de la RdP:\n " );
                    mostrar_I(cadena, fd);
                    break;

            case 2: printf( "\n   Marcado inicial de la RdP:\n" );
                    mostrar_MI(cadena, fd);
                    break;

            case 3: printf( "\n   Marcado actual de la RdP:\n " );
                    mostrar_MA(cadena, fd);
                    break;

            case 4: printf( "\n   Marcado nuevo de la RdP:\n " );
                    mostrar_MN(cadena, fd);
                    break;

            case 5: printf( "\n   Introduzca comando de disparo de transicion: ");
            		scanf( "%s", cadena);
                    disparar_trasnsicion(cadena, fd);
                    break;

            case 6: printf( "\n   Saliendo de aplicacion.\n\n");
            		break;

            default: printf("\n   Comando no valido. Intente nuevamente segun opciones de menu.\n " );
         }

         /* Fin del anidamiento */

    }

    // Close de files descriptors asociado a modulo
	close(fd);

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