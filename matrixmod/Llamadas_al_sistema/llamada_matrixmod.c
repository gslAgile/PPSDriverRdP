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
void leer_fc_MI(int *f, int *c);
void cargar_matriz(int **x, char *fname);
void cargar_vector(int *x, char *fname)
void reservar_men(int **MI, int *MA, int *MN);
void add_mod();


int main()
{
	// Variables para file descriptor (de driver del kernel)
	int fd; // file descriptor para escribir/leer el driver
	char cadena[256];
	int n, opcion;

	// Variables matrices dinamicas
	int filas=0;
	int columnas =0;	
	int **MI; // Matriz de incidencia
	int *MA;  // Marcado inicial actual
	int *MN;  // Marcado nuevo
	char fname1[256];
	char fname2[256];
	
	leer_fc_MI( &filas, &columnas);
	
	printf("   --> La matriz leida es de dimension [%u]x[%u]", filas, columnas);

	int i;	 // Recorre filas
	int j;	 // Recorre columnas

	// Reservar memoria dinamicamente (en base a las filas y coloumnas leidas en file)
	reservar_mem(MI, MA, MN);

	// cargar matrices desde file
	memset(fname1, '\0', 256);// se limpia cadena
	strcpy(fname1, "MI.txt");
	cargar_matriz(MI, fname1);

	memset(fname2, '\0', 256);// se limpia cadena
	strcpy(fname2, "MA.txt");
	cargar_vector(MA, fname2);

	// Cargamos los valores de las matrices al modulo matrixmod


	

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

/*
* Descripcion de funcion: 
* Parametros:
* @param **x: Doble puntero de matriz de incidencia.
*/

void cargar_MI(int **x)
{
	FILE *fp;
    char buffer[TAM]; // Arreglo que almacenara los datos del archivo leido (fn)
	char fname[15]="MI.txt";
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
		x[filas][columnas]=atoi(aux);
		columnas =1;
		
		while( (aux = strtok( NULL, " ")) != NULL ){   // Posteriores llamadas
			x[filas][columnas]=atoi(aux);
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

void cargar_matriz(int **x, char *fname)
{
	FILE *fp;
    char buffer[TAM]; // Arreglo que almacenara los datos del archivo leido (fn)
	//char fname[15]="MI.txt";
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
		x[filas][columnas]=atoi(aux);
		columnas =1;
		
		while( (aux = strtok( NULL, " ")) != NULL ){   // Posteriores llamadas
			x[filas][columnas]=atoi(aux);
			columnas++;
		}
   		columnas=0;
		filas++;
   	}
   	fclose(fp);// cierra archivo
	
   	printf( "\n   --> Se cargo exitosamente la matriz desde (%s).\n", fname);
}


/*
* Descripcion de funcion: 
* Parametros:
* @param **x: Doble puntero de matriz de incidencia.
*/

void cargar_vector(int *x, char *fname)
{
	FILE *fp;
    char buffer[TAM]; // Arreglo que almacenara los datos del archivo leido (fn)
	//char fname[15]="MA.txt";
	int columnas=0; //
	char *aux;

    	
    	fp= fopen(fname,"r");// se abre archivo de nombre fname
    	if( !fp ){
	      	printf( "Error al intetar abrir %s \n", fname);
		}
	
	/*Leer linea por linea del contenido del archivo(fn) en el buffer */
   	if(fgets(buffer, TAM, fp) != NULL)
   	{
		aux = strtok(buffer, " "); // guardamos primera linea
		x[columnas]=atoi(aux);
		columnas =1;
		
		while( (aux = strtok( NULL, " ")) != NULL ){// Posteriores llamadas
			x[columnas]=atoi(aux);
			columnas++;
		}
   		columnas=0;
   	}
   	fclose(fp);// cierra archivo
	
//   	printf( "\n   --> Se cargo exitosamente la matriz de incidencia desde (%s).\n", fname);
}


/*
* Descripcion de funcion: 
* Parametros:
*/

reservar_men(int **MI, int *MA, int *MN)
{
	// MATRIZ DE INCIDENCIA
	// Reserva de Memoria para filas MI
	MI = (int **)malloc(filas*sizeof(int*));
	
	// Reserva de memoria para columnas de MI
	for (i=0;i<filas;i++) 
		MI[i] = (int*)malloc(columnas*sizeof(int)); 
	

	// MARCADO INICIAL
	// Reserva de memoria para MA
	MA = (int *)malloc(filas*sizeof(int));

	// MARCADO NUEVO
	// Reserva de memoria para MN
	MN = (int *)malloc(filas*sizeof(int));
}

/*
* Descripcion de funcion: 
* Parametros:
*/

void add_mod()
{
	
}