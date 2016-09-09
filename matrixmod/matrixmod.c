#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm-generic/uaccess.h>
#include <linux/list.h>
#include "matrices.h"


/* Licencia del módulo */
MODULE_LICENSE("GPL");

/*Autores*/
MODULE_AUTHOR("Ing. Micolini Orlando, Garcia Cannata, Sosa Ludueña");

/* Descripcion del modulo*/
MODULE_DESCRIPTION("Implementa una Red de Petri en un módulo del kernel "\
					"administrable por una entrada en /proc");


#define BUFFER_LENGTH       2048
#define COMMANDSIZE			256

// Funciones:
int imprimir_matriz(struct matriz *m, char *buf, size_t len);
void iniciar_matrices(void);
void crear_rdp(char *entrada, char *faux, char *caux, int *f, int *c, struct matriz *m);
void agregar_valor(char *entrada, char *vaux, char *faux, char *caux, struct matriz *m);
int detectar_esp(char c[2] ,char *p, int *ccf);
int detectar_char(char c[2] ,char *p);
void tomar_fc(int *f, int *c, char *entrada);
void crear_rdp2(int *f, int *c, struct matriz *m, char *entrada, int pmc);


static struct proc_dir_entry *proc_entry; // entrada de /proc

// Variables Globales
struct matriz A; // Matriz A de prueba
struct matriz I; // Matriz de incidencia
struct matriz MA; // Vector de marcdo inicial
int mc[10]; // mc: vector para detectar la creacion de las matrices en el modulo.


// Implementacion de Funciones

static ssize_t matrixmod_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {

  int available_space = BUFFER_LENGTH-1; // espacio disponible
  int f, c, v; // f: filas
			   // c: columnas
			   // v: valor a cargar en matriz
  
  char kbuf[BUFFER_LENGTH];//
  char entrada[COMMANDSIZE];
  char vaux[15] = "valor ";
  char faux[15] = "fila ";
  char caux[15] = "columna ";
  int error=0;


  if ((*off) > 0) /* La aplicación puede escribir en esta entrada una sola vez !! */
    return 0;
  
  f=c=0;
  if (len > available_space) {
    printk(KERN_INFO "matrixmod: No hay espacio sufuciente!!\n");
    return -ENOSPC; // No queda espacio en el dispositivo
  }
  /* Transferencia de datos de usuario al espacio del núcleo 
     ptr destino : kbuf
     ptr origen : buf
     nrbytes: len
  */
  if (copy_from_user( kbuf, buf, len ))  
    return -EFAULT; // Direccion incorrecta
  
  kbuf[len]='\0'; // añadimos el fin de la cadena al copiar los datos from user space.
  *off+=len;            /* Actualizar el puntero del archivo */

  /*sscanf() return : el nº de elementos asignados*/
  if( sscanf(kbuf,"add I %s", entrada) == 1){
		
		agregar_valor(entrada, vaux, faux, caux, &I);

  }else if( sscanf(kbuf,"crear I %s",entrada) == 1) {

		crear_rdp(entrada, faux, caux, &f, &c, &I);
	 
  }else if( sscanf(kbuf,"crear MA %s",entrada) == 1) {

  		crear_rdp2(&f, &c, &MA, entrada, 1); /* 1: hace referencia a mc[1] para detectar que se creo
  											       una matriz en referencia a esa posicion, en este caso MA*/
  		/*Tomar valores de filas y columnas segun la entrada*/
  		//tomar_fc(&f, &c, entrada);
  		
  }else if ( strcmp(kbuf,"borrar I\n") == 0){ // strcmp() return : 0 -> si son iguales 
    
    if(mc[0] == 1){
		liberar_mem(&I);
		mc[0] = 0;
	}
	else
		 printk(KERN_INFO "matrixmod: La matriz I ya ha sido eliminada!!!\n");
  }else if ( strcmp(kbuf,"limpiar I\n") == 0){ // strcmp() return : 0 -> si son iguales 
	if(mc[0]==1){    
		limpiar_matriz(&I); // matriz I se limpia toda a cero
	}
	else
		printk(KERN_INFO "matrixmod: La matriz I no existe!!!\n");

  }else
	    printk(KERN_INFO "ERROR: comando no valido!!!\n");

	if(I.filas > 0 && I.columnas > 0 && mc[0] != 1){ /*Verificar si se puede colacar en funcion crear_rdp() ya que funciona*/
  		f= I.filas;
  		c= I.columnas;
  		cargar_matriz_cero(&I, f, c);
  		printk(KERN_INFO "INFO: Matriz I creada exitosamente!!!\n");
		mc[0]=1;
   	}

  memset(entrada, '\0', COMMANDSIZE); // limpiamos entrada capturada
  memset(faux, '\0', 15); // limpiamos faux 
  memset(caux, '\0', 15); // limpiamos caux
  memset(vaux, '\0', 15); // limpiamos vaux

  return len;
}


/*
* Descripcion: Esta funcion detecta la posicion de un caracter (pasado por paramentro)
* sobre la cadena a la que apunte el puntero *p
* @param c[2]: caracter a detectar
* @param *p: puntero que apunta a la cadena donde se busca el espacio ("_")
* @param *ccf: puntero que apunta a la direccion de un entero donde se almacena
* la cantidad de avances hasta encontrar el espacio.
* @return: retorna la direccion donde se encontro el espacio.
* sino se encontro se retorna la direccon inicial de *p.
*/
int detectar_esp(char c[2], char *p, int *ccf)
{
	int i;
	char *aux;
	aux = p; // Guardamos auxiliarmente la direccion que tiene p

	p = p + 1; // avanzamos un lugar suponiendo que el primero no es "_"
  	for (i = 0; i < COMMANDSIZE; i++)
  	{
  		if(strncmp(p,c, 1) == 0)
  			break;
  		else
  		{
  			p = p + 1;
  			*ccf = *ccf + 1;
  		}
  	}

  	if(i == COMMANDSIZE){
  		p = aux; // volvemos a asignar direccion de inicio ya que no se encontro ningun "_"
  		printk("ERROR: No se encontro direccion de '_'\n");
  	}

  	return p;
}

/*
* Descripcion: Esta funcion detecta la posicion de un caracter (pasado por paramentro)
* sobre la cadena a la que apunte el puntero *p
* @param c[2]: caracter a detectar
* @param *p: puntero que apunta a la cadena donde se busca el espacio ("_")
* la cantidad de avances hasta encontrar el espacio. Puedde mandar NULL si no
* se nescesita
* @return: retorna la direccion donde se encontro el espacio.
  sino se encontro se retorna la direccon inicial de *p.
*/
int detectar_char(char c[2], char *p)
{
	int i;
	char *aux;
	aux = p; // Guardamos auxiliarmente la direccion que tiene p

	/*Buscamos direccion de "_" avanzando para adelante*/
  	for (i = 0; i < COMMANDSIZE; i++)
  	{
  		if(strncmp(p,c, 1) == 0)
  			break;
  		else
  			p = p + 1;
  	}

  	if(i == COMMANDSIZE){
  		p = aux; // volvemos a asignar direccion de inicio ya que no se encontro ningun "_"
  		printk("ERROR: No se encontro direccion de '_'\n");
  	}

  	return p;
}


/*
* Descripcion: toma las filas y columnas del parametro enviado por el usuario en base
* a una entrada recibida como comando.
* @param *f: puntero a la direccion de la variable entera donde se almacena la cantidad de filas.
* @param *c: puntero a la direccion de la variable entera donde se almacena la cantidad de columnas.
* @param *entrada: puntero a la direccion de la entrada recibida por el usuario.
*/
void tomar_fc(int *f, int *c, char *entrada)
{
	char *p0; // puntero destinado para que apunte a la direccion 0 de entrada.
	char *s2; // puntero donde almacenaremos la direccion de un caracter
   	char s1[10]; // cadena de char donde se almacenara la ultima porcion de la entrada que son columnas
   	int ccf; // ccf: contador de cifras en filas
   	int t; // t: test funcion sscanf
  	
  	ccf = 0; // inicializamos variable
  	printk(KERN_INFO "INFO: entrada capturada para MA: %s\n", entrada);
  	p0 = entrada; // asignamos al puntero la direccion 0 de entrada
  	p0 = detectar_char("_", p0); // se determina donde empiza el primer espacio dado por "_"
  	p0 = detectar_esp("_", p0, &ccf); // avanzamos al proximo espacio para saltear las cifras de fila.

  	if(p0 == entrada)
  	{
  		printk("matrixmod_error: No se pudiron tomar los parametros de filas/columnas ingresados."\
  			   "Verifique el comando ingresado es correcto.");
  		*f=*c=0; // se asigna cero a las filas y columnas para detectar error en creacion de matriz
  	}

  	else
  	{
  		s2 = entrada; // inicializamos direccion de s2
		s2 = detectar_char("_", s2)+1; // detectamos el primer espacio "_" nuevamente.
  		if(entrada[(int)(s2)] == "-") // se castea direccion de s2 a (int) para usar la direccion como entero
  			ccf = ccf -1;

		strncpy( s1, s2, ccf);
		t = sscanf(s1,"%d", f);
		if(t == 1)
		{
			printk(KERN_INFO "INFO: captura de filas para MA: %d\n", *f);
		}
		else
			printk(KERN_INFO "INFO: Fallo captura de filas para MA\n");
  		
		t = sscanf(p0,"_%d", c);
		if(t == 1)
		{
			printk(KERN_INFO "INFO: captura de columnas para MA: %d\n", *c);
		}
		else
			printk(KERN_INFO "INFO: Fallo captura de columnas para MA\n");
  		
		printk(KERN_INFO "INFO: Valor ccf : %d para MA\n", ccf);
		memset(s1, '\0', 10); // limpiamos s1
	}
}


/*
* Descripcion: Esta funcion crea una matriz, verificando que los parametros de filas y columnas
* tomados desde una entrada del usuario sean correctos.(entrada: es un comando ingresado por el usuario)
* @param *f: puntero a la direccion de la variable entera donde se almacena la cantidad de filas.
* @param *c: puntero a la direccion de la variable entera donde se almacena la cantidad de columnas.
* @param *entrada: puntero a la direccion de la entrada recibida por el usuario.
* @param pmc: posicion de matriz a crear sobre el vector mc[]
*/
void crear_rdp2(int *f, int *c, struct matriz *m, char *entrada, int pmc)
{
	/* Tomar valores de filas y columnas segun la entrada*/
  	tomar_fc(f, c, entrada);

  	/* Verificamos filas y columnas*/
  	if(*f < 1 || *c < 1)
  	{
  		printk("matrixmod_error: Filas o columnas mal ingresada.\n");
  		printk("matrixmod_error: No se pudo crear matriz con id: %d.\n", pmc);
  		*f=*c=0;
  	}
  	else
  	{
  		/* Si matriz no existe, asignamos filas y columnas a matriz */
  		if(m->filas == 0 && *f > 0 && m->columnas == 0 && *c > 0 && mc[pmc] != 1)
		{
			m->filas = *f;
			m->columnas = *c;
			printk(KERN_INFO "matrixmod_info: Se asigno %d filas en matriz exitosamente!!!\n", *f);
			printk(KERN_INFO "matrixmod_info: Se asigno %d columnas en matriz exitosamente!!!\n", *c);

			if(m->filas > 0 && m->columnas > 0 && mc[pmc] != 1){
  				cargar_matriz_cero(m, *f, *c);
  				printk(KERN_INFO "matrixmod_info: Matriz con id: %d creada exitosamente!!!\n", pmc);
				mc[pmc]=1; // Matriz con id: pmc creada
   			}
		}

		/* Si matriz existe*/
		else if (mc[pmc] == 1)
			printk(KERN_INFO "matrixmod_error: Matriz con id: %d ya creada!!!\n", pmc);
  	}
}


void crear_rdp(char *entrada, char *faux, char *caux, int *f, int *c, struct matriz *m)
{
	static int t; // f: filas

	if(entrada[2]< 48 || entrada[2]>57)// filas son de 2 cifras?
		{
			if(entrada[4] > 47 && entrada[4] < 58)// columnas son de 2 cifras?			
			{
				// se trata de un caso [2 cifras]x[2 cifras]
				strncat(faux, entrada, 2);
				strncat(caux, entrada+3, 2);
		
				t = sscanf(faux,"fila %d", f);
				t = sscanf(caux, "columna %d", c);
			}
			else
			{
				// se trata de un caso [2 cifras]x[1 cifra]
				strncat(faux, entrada, 2);
				strncat(caux, entrada+3, 1);
		
				t = sscanf(faux,"fila %d", f);
				t = sscanf(caux, "columna %d", c);
			}
		}
		else if(entrada[1] < 48 || entrada[1] > 57) // filas son de una cifra?
		{
			if(entrada[3] > 47 && entrada[3] < 58)// columnas son de 2 cifras?
			{
				// se trata de un caso [1 cifra]x[2 cifras]
				strncat(faux, entrada, 1);
				strncat(caux, entrada+2, 2);
		
				t = sscanf(faux,"fila %d", f);
				t = sscanf(caux, "columna %d", c);
			}
			else
			{
				// se trata de un caso [1 cifra]x[1 cifra]
				strncat(faux, entrada, 1);
				strncat(caux, entrada+2, 1);
		
				t = sscanf(faux,"fila %d", f);
				t = sscanf(caux, "columna %d", c);
			}
			
		}else
		{
			printk(KERN_INFO "ERROR: Dimension de matriz no soportada o parametros incorrectos.\n");
			*f = *c = 0;
		}
		
		printk(KERN_INFO "INFO: entrada capturada: %s\n", entrada);
    	printk(KERN_INFO "INFO: Fila ingresada: %d\n", *f);
		printk(KERN_INFO "INFO: Columna ingresada: %d\n", *c);

		if(m->filas == 0 && *f > 0 && m->columnas == 0 && *c > 0 && mc[0] != 1)
		{
			m->filas = *f;
			m->columnas = *c;
			printk(KERN_INFO "INFO: Se asigno %d filas en matriz I exitosamente!!!\n", *f);
			printk(KERN_INFO "INFO: Se asigno %d columnas en matriz I exitosamente!!!\n", *c);
		}

		else if (mc[0] == 1) // >> Ver que no se solape con error de matriz no soportada <<
			printk(KERN_INFO "ERROR: Filas y columnas de matriz I ya asignadas!!!\n");
}


void agregar_valor(char *entrada, char *vaux, char *faux, char *caux, struct matriz *m)
{
	static int error;
	static int f, c, v, t; // f: filas
			   // c: columnas
			   // v: valor a cargar en matriz
			   // t: test funcion sscanf

	if(entrada[2] < 48 || entrada[2] > 57)// filas son de 2 cifras?
		{
			if(entrada[4] < 48 || entrada[4] > 57)// columnas de 1 cifra?
			{
				if(entrada[5] == "-") // valor negativo?
				{
					if(entrada[8] > 47 && entrada[8] < 58)// valor de 3 cifras?
					{	/* Error valor negativo de 3 cifras no soportado por modulo!!!*/
						error = 1;
						f = c = v = 0;
					}
					else if(entrada[7] > 47 && entrada[7] < 58)// valor de 2 cifras?
					{	/* Se trata de caso filas: 2 cifras - columnas: 1 cifra - valor: 2 cifras negativo*/
						strncat(faux, entrada, 2);
						strncat(caux, entrada+3, 1);
						strncat(vaux, entrada+6, 2);
		
						t = sscanf(faux,"fila %d", &f);
						t = sscanf(caux, "columna %d", &c);
						t = sscanf(vaux, "valor %d", &v);
					}
					else
					{	/* Se trata de caso filas: 2 cifras - columnas: 1 cifra - valor: 1 cifra negativo*/
						strncat(faux, entrada, 2);
						strncat(caux, entrada+3, 1);
						strncat(vaux, entrada+6, 1);
		
						t = sscanf(faux,"fila %d", &f);
						t = sscanf(caux, "columna %d", &c);
						t = sscanf(vaux, "valor %d", &v);
					}
				}
				else if(entrada[7] > 47 && entrada[7] < 58) // valor de 3 cifras ?
				{	/* Error valor de 3 cifras no soportado por modulo!!!*/
					error = 1;
					f = c = v = 0;
				}
				else if(entrada[6] > 47 && entrada[6]< 58) // valor de 2 cifras ?
				{	/* Se trata de caso filas: 2 cifras - columnas: 1 cifra - valor: 2 cifras*/
					strncat(faux, entrada, 2);
					strncat(caux, entrada+3, 1);
					strncat(vaux, entrada+5, 2);
		
					t = sscanf(faux,"fila %d", &f);
					t = sscanf(caux, "columna %d", &c);
					t = sscanf(vaux, "valor %d", &v);
				}
				else
				{	/* Se trata de caso filas: 2 cifras - columnas: 1 cifra - valor: 1 cifra*/
					strncat(faux, entrada, 2);
					strncat(caux, entrada+3, 1);
					strncat(vaux, entrada+5, 1);
		
					t = sscanf(faux,"fila %d", &f);
					t = sscanf(caux, "columna %d", &c);
					t = sscanf(vaux, "valor %d", &v);
				}
			}
			else if(entrada[5] < 48 || entrada[5] > 57) // columnas de 2 cifras?
			{
				if(entrada[6] == "-") // valor negativo?
				{
					if(entrada[9] > 47 && entrada[9] < 58)// valor de 3 cifras?
					{	/* Error valor negativo de 3 cifras no soportado por modulo!!!*/
						error = 1;
						f = c = v = 0;
					}
					else if(entrada[8] > 47 && entrada[8] < 58)// valor de 2 cifras?
					{	/* Se trata de caso filas: 2 cifras - columnas: 2 cifras - valor: 2 cifras negativo*/
						strncat(faux, entrada, 2);
						strncat(caux, entrada+3, 2);
						strncat(vaux, entrada+7, 2);
		
						t = sscanf(faux,"fila %d", &f);
						t = sscanf(caux, "columna %d", &c);
						t = sscanf(vaux, "valor %d", &v);
					}
					else
					{	/* Se trata de caso filas: 2 cifras - columnas: 2 cifras - valor: 1 cifra negativo*/
						strncat(faux, entrada, 2);
						strncat(caux, entrada+3, 2);
						strncat(vaux, entrada+7, 1);
		
						t = sscanf(faux,"fila %d", &f);
						t = sscanf(caux, "columna %d", &c);
						t = sscanf(vaux, "valor %d", &v);
					}
				}
				else if(entrada[8] > 47 && entrada[8] < 58) // valor de 3 cifras ?
				{	/* Error valor de 3 cifras no soportado por modulo!!!*/
					error = 1;
					f = c = v = 0;
				}
				else if(entrada[7] > 47 && entrada[7] < 58) // valor de 2 cifras ?
				{	/* Se trata de caso filas: 2 cifras - columnas: 2 cifras - valor: 2 cifras*/
					strncat(faux, entrada, 2);
					strncat(caux, entrada+3, 2);
					strncat(vaux, entrada+6, 2);
		
					t = sscanf(faux,"fila %d", &f);
					t = sscanf(caux, "columna %d", &c);
					t = sscanf(vaux, "valor %d", &v);
				}
				else
				{	/* Se trata de caso filas: 2 cifras - columnas: 2 cifras - valor: 1 cifra*/
					strncat(faux, entrada, 2);
					strncat(caux, entrada+3, 2);
					strncat(vaux, entrada+6, 1);
		
					t = sscanf(faux,"fila %d", &f);
					t = sscanf(caux, "columna %d", &c);
					t = sscanf(vaux, "valor %d", &v);
				}
				
			}
			else
			{
				/* Error cadena mal ingresda*/
				error = 2;
				f = c = v = 0;
			}
		}
		else if(entrada[1] < 48 || entrada[1] > 57) // filas 1 cifra ?
		{
			if(entrada[3] < 48 || entrada[3] > 57)// columnas de 1 cifra ?
			{	
				if(entrada[4] == "-") // valor negativo?
				{
					if(entrada[7] > 47 && entrada[7] < 58)// valor de 3 cifras?
					{	/* Error valor negativo de 3 cifras no soportado por modulo!!!*/
						error = 1;
						f = c = v = 0;
					}
					else if(entrada[6] > 47 && entrada[6] < 58)// valor de 2 cifras?
					{	/* Se trata de caso filas: 1 cifra - columnas: 1 cifra - valor: 2 cifras negativo*/
						strncat(faux, entrada, 1);
						strncat(caux, entrada+2, 1);
						strncat(vaux, entrada+5, 2);
		
						t = sscanf(faux,"fila %d", &f);
						t = sscanf(caux, "columna %d", &c);
						t = sscanf(vaux, "valor %d", &v);
					}
					else
					{	/* Se trata de caso filas: 1 cifra - columnas: 1 cifra - valor: 1 cifra negativo*/
						strncat(faux, entrada, 1);
						strncat(caux, entrada+2, 1);
						strncat(vaux, entrada+5, 1);
		
						t = sscanf(faux,"fila %d", &f);
						t = sscanf(caux, "columna %d", &c);
						t = sscanf(vaux, "valor %d", &v);
					}
				}
				else if(entrada[6] > 47 && entrada[6] < 58) // valor de 3 cifras ?
				{	/* Error valor de 3 cifras no soportado por modulo!!!*/
					error = 1;
					f = c = v = 0;
				}
				else if (entrada[5] > 47 && entrada[5] < 58) // valor de 2 cifras ?
				{	/* Se trata de caso filas: 1 cifra - columnas: 1 cifra - valor: 2 cifras*/
					strncat(faux, entrada, 1);
					strncat(caux, entrada+2, 1);
					strncat(vaux, entrada+4, 2);
		
					t = sscanf(faux,"fila %d", &f);
					t = sscanf(caux, "columna %d", &c);
					t = sscanf(vaux, "valor %d", &v);
				}
				else
				{	/* Se trata de caso filas: 1 cifra - columnas: 1 cifra - valor: 1 cifra*/
					strncat(faux, entrada, 1);
					strncat(caux, entrada+2, 1);
					strncat(vaux, entrada+4, 1);
		
					t = sscanf(faux,"fila %d", &f);
					t = sscanf(caux, "columna %d", &c);
					t = sscanf(vaux, "valor %d", &v);
				}
			}
			else if(entrada[4] < 48 || entrada[4] > 57) // columnas de 2 cifras?
			{
				if(entrada[5] == "-") // valor negativo?
				{
					if(entrada[8] > 47 && entrada[8] < 58)// valor de 3 cifras?
					{	/* Error valor negativo de 3 cifras no soportado por modulo!!!*/
						error = 1;
						f = c = v = 0;
					}
					else if(entrada[7] > 47 && entrada[7] < 58)// valor de 2 cifras?
					{	/* Se trata de caso filas: 1 cifras - columnas: 2 cifras - valor: 2 cifras negativo*/
						strncat(faux, entrada, 1);
						strncat(caux, entrada+2, 2);
						strncat(vaux, entrada+6, 2);
		
						t = sscanf(faux,"fila %d", &f);
						t = sscanf(caux, "columna %d", &c);
						t = sscanf(vaux, "valor %d", &v);
					}
					else
					{	/* Se trata de caso filas: 1 cifras - columnas: 2 cifras - valor: 1 cifra negativo*/
						strncat(faux, entrada, 1);
						strncat(caux, entrada+2, 2);
						strncat(vaux, entrada+6, 1);
		
						t = sscanf(faux,"fila %d", &f);
						t = sscanf(caux, "columna %d", &c);
						t = sscanf(vaux, "valor %d", &v);
					}
				}
				else if(entrada[7] > 47 && entrada[7] < 58) // valor de 3 cifras ?
				{	/* Error valor de 3 cifras no soportado por modulo!!!*/
					error = 1;
					f = c = v = 0;
				}
				else if(entrada[6] > 47 && entrada[6] < 58) // valor de 2 cifras ?
				{	/* Se trata de caso filas: 1 cifra - columnas: 2 cifras - valor: 2 cifras*/
					strncat(faux, entrada, 1);
					strncat(caux, entrada+2, 2);
					strncat(vaux, entrada+5, 2);
		
					t = sscanf(faux,"fila %d", &f);
					t = sscanf(caux, "columna %d", &c);
					t = sscanf(vaux, "valor %d", &v);
				}
				else
				{	/* Se trata de caso filas: 1 cifra - columnas: 2 cifras - valor: 1 cifra*/
					strncat(faux, entrada, 1);
					strncat(caux, entrada+2, 2);
					strncat(vaux, entrada+5, 1);
		
					t = sscanf(faux,"fila %d", &f);
					t = sscanf(caux, "columna %d", &c);
					t = sscanf(vaux, "valor %d", &v);
				}
				
			}
			else
			{	/* Error */
				error = 2;
				f = c = v = 0;
			}
			
		}
		else
		{
			f = c = v = 0;

			if(error == 1)
				printk(KERN_INFO "ERROR: Valor de 3 cifras no soportado por modulo!!!\n");
			
			else
				printk(KERN_INFO "ERROR: posicion de fila o columna no valida!!!\n");
		}
		
		printk(KERN_INFO "INFO: entrada capturada: %s\n", entrada);
    	printk(KERN_INFO "INFO: Fila ingresada: %d\n", f);
		printk(KERN_INFO "INFO: Columna ingresada: %d\n", c);
		printk(KERN_INFO "INFO: Valor ingresado: %d\n", v);
    	
		if(f < m->filas && c < m->columnas && error < 1)
		{
			m->matriz[f][c]=v;
		}
}



static ssize_t matrixmod_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
  
  int nr_bytes;
  //nr_bytes=0;

  /* Decirle a la aplicación que ya no hay nada que leer  "Para no copiar basura si llamas otra vez" */
  if ((*off) > 0){
	  printk(KERN_INFO "matrixmod: no hay nada que leer \n");
      return 0;}

  if(mc[1]==1)
  	nr_bytes=imprimir_matriz(&MA, buf, len);
    
  else
	printk(KERN_INFO "matrixmod: Matriz I ha sido eliminada o no fue creada. \n");

  (*off)+=len;  /* Actualizo el puntero de archivo */
  //vfree(kbuf);
  return nr_bytes; 
}

static const struct file_operations proc_entry_fops = {
    .read = matrixmod_read,
    .write = matrixmod_write,    
};

/*
* Descripcion: Funcion que imprime matriz por pantalla.
* Parametros:
* @param *x: puntero de una estructura de tipo matriz utilizado para mostrar los
* 		     datos de la matriz.
*/
int imprimir_matriz(struct matriz *m, char *buf, size_t len)
{
	char kbuf[BUFFER_LENGTH] = "";	
	int i,j,k, nr_bytes;// i,j : recorren la matriz
			  			// k: va corriendo el puntero del buffer del kernel kbuf;
						// nr_bytes:
	k=0;
	nr_bytes=0;

	for(i=0; i<m->filas; i++){
		for(j=0; j<m->columnas; j++)
		{
			if(m->matriz[i][j] >= 0){
				kbuf[k]=(char)32;/* Ascii de espacio*/ k++;
				kbuf[k]=(char)32; k++;
				kbuf[k]=(char)(m->matriz[i][j] + 48); k++;
				}
	
			else{
				kbuf[k]=(char)32; k++;
				kbuf[k]=(char)45;/* Ascii de signo negativo*/ k++;
//				kbuf[k]=(char)(48 + (48 - (A.matriz[i][j] + 48))); k++;
				kbuf[k]=(char)(48 - m->matriz[i][j]); k++;  
				/* Si A[i][j]=-1 --> Esto seria: 48 - (-1) = 49 que es el codigo ASCII de 1, es decir que convertimos
				   los negativos a positivos para ser mostrados de manera correcta en la entrada estandar. */
			}
		}// Fin for 1
		
		kbuf[k]=(char)10; /* Ascii salto de linea */ k++;
	} // Fin for 2
	
	nr_bytes=k+1;
		
	if (len< nr_bytes){
		printk(KERN_INFO "matrixmod: No queda espacio en el dispositivo \n");
	    return -ENOSPC; //No queda espacio en el dispositivo
	}
 
   	/* Transferencia de datos desde el espacio kernel al espacio de usuario */  
    if(copy_to_user(buf, kbuf, nr_bytes)){
		printk(KERN_INFO "matrixmod: Argumento invalido\n");
   		return -EINVAL; //Argumento invalido
	}
	
	return nr_bytes;
}


/* Asigna cero a las filas y columnas de todas las matrices*/
void iniciar_matrices(void )
{
	I.filas = I.columnas =0;
	MA.filas = MA.columnas = 0;
	mc[0]=0; // matriz I no creada
	mc[1]=0; // matriz MA no creada
}


int init_modlist_module( void )
{
  int ret = 0;

  /* en matrixmod defenido en /proc, solo podemos usar las funciones
	 defenidas en proc_entry_fops */
  proc_entry = proc_create( "matrixmod", 0666, NULL, &proc_entry_fops); 
  if (proc_entry == NULL) {
    ret = -ENOMEM; // No hay bastante espacio
    printk(KERN_INFO "matrixmod: No se puede crear la entrada /proc\n");
	
  }else {
    printk(KERN_INFO "matrixmod: Modulo cargado.\n");
	//cargar_matriz_uno(&A, 10, 10);
	iniciar_matrices();
  }

  return ret;

}


void exit_modlist_module( void )
{
  // agregar --> eliminar espacio en memoria
	if(mc[0] == 1)
		liberar_mem(&I);

	if(mc[1] == 1)
		liberar_mem(&MA);

  remove_proc_entry("matrixmod", NULL); // eliminar la entrada del /proc
  printk(KERN_INFO "matrixmod: Modulo descargado.\n");
}


module_init( init_modlist_module );
module_exit( exit_modlist_module );
