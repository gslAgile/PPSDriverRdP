#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm-generic/uaccess.h>
#include <linux/list.h>
#include "Librerias_kernel/matrices.h"

#define SUCCESS 0;

/* Licencia del módulo */
MODULE_LICENSE("GPL");

/* Autores */
MODULE_AUTHOR("Ing. Micolini Orlando, Garcia Cannata, Sosa Ludueña");

/* Descripcion del modulo */
MODULE_DESCRIPTION("Implementa una RdPG en un módulo del kernel "\
					"administrable por una entrada en /proc");

#define BUFFER_LENGTH       2048
#define COMMANDSIZE	    256

// Funciones
int imprimir_matriz(struct matriz *m, char *buf, size_t len);
void iniciar_matrices(void);
void agregar_valor(char *entrada, char *vaux, char *faux, char *caux, struct matriz *m);
int detectar_esp(char c[2] ,char *p, int *ccf);
int detectar_char(char c[2] ,char *p);
void tomar_fc(int *f, int *c, char *entrada);
void crear_rdp(int *f, int *c, struct matriz *m, int pmc);
void crear_mdisparos(int c, int mc_id);
int disparar(int id_d, char mode);
int disparar_rdpg(int id_d, char mode);
void cargar_MA(void);
void cargar_E(void);
void cargar_Q(void);
void cargar_W(void);
void cargar_B(void);
void cargar_L(void);
void cargar_Ex(void);
void tomar_transicion(char *entrada, int *transicion);
void cargar_cero(struct matriz *m);

static struct proc_dir_entry *proc_entry; // Entrada de /proc

// Variables Globales
static int Device_Open = 0; /* Es un device open? */
			    /* Uso para prevenir multiples accesos en el dispositivo */
struct matriz A; 	  // Matriz A de prueba
struct matriz I; 	  // Matriz de incidencia, asociado a mc[0]
struct matriz H; 	  // Matriz de incidencia H asociada a los brazos inhibidores, asociado a mc[6]
struct matriz MA;         // Vector de marcado actual, asociado a mc[1]
struct matriz MI; 	  // Vector de marcado inicial, asociado a mc[3]
struct matriz MN; 	  // Vector de marcado nuevo, asociado a mc[4]
struct matriz E; 	  // Vector de transiciones sensibilizadas, asociado a mc[7]
struct matriz Q;	  // Vector asociado a la funcion cero, asociado a mc[9]
struct matriz W;    // Vector asociado a la funcion uno, asociado a mc[10]
struct matriz L;    // Vector de transiciones inhibidas por arco lector L, asociado a mc[11]
struct matriz B;    // Vector de transiciones inhibidas por arco inhibidor B, asociado a mc[12]
struct matriz Ex;    // Vector de sensibilizado extendido Ex, asociado a mc[13]
struct matriz vauxiliar;  // vector donde se almacenara alguno de todos los disparos, asociado a mc[5]
struct matriz disparos;   // matriz con cada uno de los vectores disparos, asociado a mc[2]
struct matriz R;          // Matriz de incidencia R asociada a los brazos lectores, asociado a mc[8]
struct matriz vauxiliar;  // vector donde se almacenara alguno de todos los disparos, asociado a mc[5]
struct matriz disparos;   // matriz con cada uno de los vectores disparos, asociado a mc[2]
int mc[20]; // mc: vector para detectar la creacion de las matrices en el modulo.
int mostrar_mc; // entero identificatorio de cual de las matrices creadas mostrara para funcion read
int cd; // numero de vectores y elementos en vector disparo
int count_read = 0; // contador de lecturas en modulo
int cvmi = 0; // cvmi: contador de valores ingresados en MI
int cvmr = 0; // cvmr: contador de valores ingresados en matriz R
int cvmh = 0; // cvmr: contador de valores ingresados en matriz H

// Implementacion de Funciones
/*
* Llamada cuando  un proceso intentó abrir el archivo de dispositivo, como
* "cat /dev/mycharfile".
*/
static int matrixmod_open(struct inode *inode, struct file *file)
{
	if (Device_Open > 10)
	        return -EBUSY;
 
    	Device_Open++;
 
    	try_module_get(THIS_MODULE);
    	return SUCCESS;
}

/*
* Se llama cuando un proceso cierra el archivo de dispositivo.
*/
static int matrixmod_release(struct inode *inode, struct file *file)
{
    	Device_Open--;
    	/* No estamos listos para nuestro siguiente llamada */
    	/*
    	* Decrementar el contador de uso, o de lo contrario una vez que ha
    	* abierto el archivo, usted nunca se deshacera del módulo.
    	*/
    	module_put(THIS_MODULE);
    	return SUCCESS;
}

static ssize_t matrixmod_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {

  int available_space = BUFFER_LENGTH-1; // espacio disponible
  int transicion = 0; // transicion a disparar sobre RdP
  int f, c, v; // f: filas
               // c: columnas
	       // v: valor a cargar en matriz
  
  char kbuf[BUFFER_LENGTH];//
  char entrada[COMMANDSIZE];
  char vaux[15] = "valor ";
  char faux[15] = "fila ";
  char caux[15] = "columna ";
  //int error=0;
  /* if ((*off) > 0)  La aplicación puede escribir en esta entrada una sola vez !! 
    return 0; */
  
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
  
  kbuf[len]='\0'; // Añadimos el fin de la cadena al copiar los datos from user space.
  *off+=len;            /* Actualizar el puntero del archivo */

  /* sscanf() return : el nº de elementos asignados */
  if( sscanf(kbuf,"add I %s", entrada) == 1){
		
		agregar_valor(entrada, vaux, faux, caux, &I);

  }else if( sscanf(kbuf,"add H %s", entrada) == 1){
		
		agregar_valor(entrada, vaux, faux, caux, &H);
    cvmh++;
    if(cvmh == (H.filas*H.columnas)) /* Se completo el ingreso de los elementos de la matriz H ??*/
    {
      cvmh = 0;
      cargar_B();
    }

  }else if( sscanf(kbuf,"add R %s", entrada) == 1){
    
		agregar_valor(entrada, vaux, faux, caux, &R);
    cvmr++;
    if(cvmr == (R.filas*R.columnas)) /* Se completo el ingreso de los elementos de la matriz R ??*/
    {
      cvmr = 0;
      cargar_L();
    }

  }else if( sscanf(kbuf,"crear I %s",entrada) == 1) {

		/* Tomar valores de filas y columnas segun la entrada */
  	tomar_fc(&f, &c, entrada);
  		
  	/* Creamos matriz I segun entrada recibida */
		crear_rdp(&f, &c, &I, 0); /* 0: hace referencia a mc[0] para detectar que se creo
						                        una matriz en referencia a esa posicion, en este caso I */
		
		if(mc[0]) // Si se creo exitosamente Matriz I
		{ 
			int ff, cc, ffi, cci;

			crear_mdisparos(I.columnas, 2);
			ff = 1;
			cc = I.filas;
			ffi = I.filas;
			cci = I.columnas;

			/* Creamos vector de marcado actual MA, por defecto en cero sus valores hasta crear MI */
			crear_rdp(&ff, &cc, &MA, 1); /* 1: hace referencia a mc[1] para detectar que se creo
  							   una matriz en referencia a esa posicion, en este caso MA */

			/* Creamos vector de marcado nuevo MN, por defecto en cero sus valores hasta crear MI */
			crear_rdp(&ff, &cc, &MN, 4); /* 4: hace referencia a mc[4] para detectar que se creo
  						           una matriz en referencia a esa posicion, en este caso MN */

  		crear_rdp(&ffi, &cci, &H, 6); /* 6: hace referencia a mc[6] para detectar que se creo
							una matriz en referencia a esa posicion, en este caso H */

			/* Creamos vector E de transisiones sensibilizadas, por defecto en cero hasta conocer MI */
      cc = I.columnas; // Se asigna cantidad de columnas cc a la cantidad de columnas de I (equivalente a la cantidad de transiciones)
			crear_rdp(&ff, &cc, &E, 7); /* 7: hace referencia a mc[7] para detectar que se creo
                                       	una matriz en referencia a esa posicion, en este caso E */

      /* Creamos vector L, por defecto en cero sus valores hasta crear MI */
      crear_rdp(&ff, &cc, &L, 11); /* 11: hace referencia a mc[11] para detectar que se creo
                                          un vector en referencia a esa posicion, en este caso L */

      /* Creamos vector B, por defecto en cero sus valores hasta crear MI */
      crear_rdp(&ff, &cc, &B, 12); /* 12: hace referencia a mc[12] para detectar que se creo
                                          un vector en referencia a esa posicion, en este caso B */

      /* Creamos vector Ex, por defecto en cero sus valores hasta crear MI */
      crear_rdp(&ff, &cc, &Ex, 13); /* 13: hace referencia a mc[13] para detectar que se creo
                                          un vector en referencia a esa posicion, en este caso Ex */

			cc = I.filas; // Se asigna cantidad de filas de I a cc (equivalente a la cantidad de plazas)
			
      /* Creamos vector Q, por defecto en cero sus valores hasta crear MI */
			crear_rdp(&ff, &cc, &Q, 9); /* 9: hace referencia a mc[9] para detectar que se creo
  						                          un vector en referencia a esa posicion, en este caso Q */

      /* Creamos vector W, por defecto en cero sus valores hasta crear MI */
      crear_rdp(&ff, &cc, &W, 10); /* 10: hace referencia a mc[10] para detectar que se creo
                                          un vector en referencia a esa posicion, en este caso W */
	   }
  }else if( sscanf(kbuf,"crear MI %s",entrada) == 1) {

  		/*Tomar valores de filas y columnas segun la entrada*/
  		tomar_fc(&f, &c, entrada);
  		/* Creamos matriz MI segun entrada recibida*/
  		crear_rdp(&f, &c, &MI, 3); /* 3: hace referencia a mc[3] para detectar que se creo
  											               una matriz en referencia a esa posicion, en este caso MI*/
  		
  }else if( sscanf(kbuf,"crear R %s",entrada) == 1) {

      /*Tomar valores de filas y columnas segun la entrada*/
      tomar_fc(&f, &c, entrada); // posibilidad de no utilizar funcion pq existe I de donde sacar los datos
      /* Creamos matriz MA segun entrada recibida*/
      crear_rdp(&f, &c, &R, 8); /* 8: hace referencia a mc[8] para detectar que se creo
                                      una matriz en referencia a esa posicion, en este caso R */
      
  }else if( sscanf(kbuf,"add MI %s", entrada) == 1){
		
		agregar_valor(entrada, vaux, faux, caux, &MI);
    cvmi++; // incremento en la cuenta de valores de MI
		if(mc[1]) // Si MA existe
			cargar_MA(); // Se actualiza cada valor de MI(marcado inicial) en MA(maracado actual)

    if(cvmi == I.filas) // Se completo el ingreso de MI?
    {
        cvmi = 0; // se reicia la cuenta de valores, para un nuevo ingreso de vector
      	cargar_E();
        cargar_Q();
        cargar_W();
    }

  }else if ( sscanf(kbuf,"STEP_CMD%s", entrada) == 1){
	
	tomar_transicion(entrada, &transicion);
	if(transicion > -1 && transicion < I.filas) // Si realizo disparo exitosamente
	{
		if(disparar_rdpg(transicion, 'E') == 1)
			printk(KERN_INFO "matrixmod_info: El disparo fue exitoso!!!\n");

		else
		  printk(KERN_INFO "matrixmod_info: El disparo no fue exitoso!!!\n");
	}

	else
		printk(KERN_INFO "matrixmod_info: El disparo no fue exitoso!!!\n");

  }else if ( strcmp(kbuf,"borrar I\n") == 0){ // strcmp() return : 0 -> si son iguales 
    
    if(mc[0] == 1){
		liberar_mem(&I);
		mc[0] = 0;
	}
	else
		 printk(KERN_INFO "matrixmod: La matriz I ya ha sido eliminada!!!\n");
  }else if ( strcmp(kbuf,"borrar MA\n") == 0){ // strcmp() return : 0 -> si son iguales 
    
    if(mc[1] == 1){
		liberar_mem(&MA);
		mc[1] = 0;
	}
	else
		 printk(KERN_INFO "matrixmod: La matriz MA no existe!!!\n");
  }else if ( strcmp(kbuf,"borrar H\n") == 0){ // strcmp() return : 0 -> si son iguales 
    
    if(mc[6] == 1){
		liberar_mem(&H);
		mc[6] = 0;
	}
	else
		 printk(KERN_INFO "matrixmod: La matriz H no existe!!!\n");
  }else if ( strcmp(kbuf,"limpiar I\n") == 0){ // strcmp() return : 0 -> si son iguales 
	
	if(mc[0]==1){    
		limpiar_matriz(&I); // matriz I se limpia toda a cero
	}
	else
		printk(KERN_INFO "matrixmod: La matriz I no existe!!!\n");

  }else if ( strcmp(kbuf,"limpiar MA\n") == 0){ // strcmp() return : 0 -> si son iguales 
	
	if(mc[1]==1){    
		limpiar_matriz(&MA); // matriz I se limpia toda a cero
	}
	else
		printk(KERN_INFO "matrixmod: La matriz MA no existe!!!\n");

  }else if ( strcmp(kbuf,"limpiar H\n") == 0){ // strcmp() return : 0 -> si son iguales 

	if(mc[6]==1){    
		limpiar_matriz(&H); // matriz H se limpia toda a cero
	}
	else
		printk(KERN_INFO "matrixmod: La matriz H no existe!!!\n");

  }else if ( strcmp(kbuf,"mostrar I\n") == 0){ // strcmp() return : 0 -> si son iguales 
	
	mostrar_mc = 0; // se selecciona identificador para mostrar la matriz I
	printk(KERN_INFO "matrixmod_info: Se asigna %s para mostrar en funcion read().\n", I.nombre);

  }else if ( strcmp(kbuf,"mostrar MA\n") == 0){ // strcmp() return : 0 -> si son iguales 
	
	mostrar_mc = 1; // se selecciona identificador para mostrar la matriz MA
	printk(KERN_INFO "matrixmod_info: Se asigna %s para mostrar en funcion read().\n", MA.nombre);

  }else if ( strcmp(kbuf,"mostrar disparos\n") == 0){ // strcmp() return : 0 -> si son iguales 
	
	mostrar_mc = 2; // se selecciona identificador para mostrar la matriz I
	printk(KERN_INFO "matrixmod_info: Se asigna %s para mostrar en funcion read().\n", disparos.nombre);

  }else if ( strcmp(kbuf,"mostrar MI\n") == 0){ // strcmp() return : 0 -> si son iguales 
	
	mostrar_mc = 3; // se selecciona identificador para mostrar la matriz MA
	printk(KERN_INFO "matrixmod_info: Se asigna %s para mostrar en funcion read().\n", MI.nombre);

  }else if ( strcmp(kbuf,"mostrar MN\n") == 0){ // strcmp() return : 0 -> si son iguales 
	
	mostrar_mc = 4; // se selecciona identificador para mostrar la matriz MA
	printk(KERN_INFO "matrixmod_info: Se asigna %s para mostrar en funcion read().\n", MN.nombre);

  }else if ( strcmp(kbuf,"mostrar E\n") == 0){ // strcmp() return : 0 -> si son iguales 
  
  	mostrar_mc = 7; // se selecciona identificador para mostrar la matriz E
  	printk(KERN_INFO "matrixmod_info: Se asigna %s para mostrar en funcion read().\n", E.nombre);

  }else if ( strcmp(kbuf,"mostrar L\n") == 0){ // strcmp() return : 0 -> si son iguales 
  
    mostrar_mc = 11; // se selecciona identificador para mostrar la matriz L
    printk(KERN_INFO "matrixmod_info: Se asigna %s para mostrar en funcion read().\n", L.nombre);

  }else if ( strcmp(kbuf,"mostrar B\n") == 0){ // strcmp() return : 0 -> si son iguales 
  
    mostrar_mc = 12; // se selecciona identificador para mostrar la matriz B
    printk(KERN_INFO "matrixmod_info: Se asigna %s para mostrar en funcion read().\n", B.nombre);

  }else if ( strcmp(kbuf,"mostrar Ex\n") == 0){ // strcmp() return : 0 -> si son iguales 
  
    mostrar_mc = 13; // se selecciona identificador para mostrar la matriz Ex
    cargar_Ex();
    printk(KERN_INFO "matrixmod_info: Se asigna %s para mostrar en funcion read().\n", Ex.nombre);

  }else if ( strcmp(kbuf,"mostrar Q\n") == 0){ // strcmp() return : 0 -> si son iguales 
  
  	mostrar_mc = 9; // se selecciona identificador para mostrar el vector Q
  	printk(KERN_INFO "matrixmod_info: Se asigna %s para mostrar en funcion read().\n", Q.nombre);

  }else if ( strcmp(kbuf,"mostrar W\n") == 0){ // strcmp() return : 0 -> si son iguales 
  
    mostrar_mc = 10; // se selecciona identificador para mostrar el vector W
    printk(KERN_INFO "matrixmod_info: Se asigna %s para mostrar en funcion read().\n", W.nombre);

  }else if ( strcmp(kbuf,"mostrar d\n") == 0){ // strcmp() return : 0 -> si son iguales 
	
	mostrar_mc = 5; // se selecciona identificador para mostrar la matriz MA
	printk(KERN_INFO "matrixmod_info: Se asigna %s para mostrar en funcion read().\n", vauxiliar.nombre);

  }else if ( strcmp(kbuf,"mostrar H\n") == 0){ // strcmp() return : 0 -> si son iguales 
	
	mostrar_mc = 6; // se selecciona identificador para mostrar la matriz H
	printk(KERN_INFO "matrixmod_info: Se asigna %s para mostrar en funcion read().\n", H.nombre);

  }else if ( strcmp(kbuf,"mostrar R\n") == 0){ // strcmp() return : 0 -> si son iguales 
  
  	mostrar_mc = 8; // se selecciona identificador para mostrar la matriz R
  	printk(KERN_INFO "matrixmod_info: Se asigna %s para mostrar en funcion read().\n", R.nombre);

  }else if ( strcmp(kbuf,"cargar MA\n") == 0){ // strcmp() return : 0 -> si son iguales 
	
	 cargar_MA();
	 printk(KERN_INFO "matrixmod_info: Se realiza carga de MA.\n");

  }else if ( strcmp(kbuf,"mostrar mc\n") == 0){ // strcmp() return : 0 -> si son iguales 
	
	 printk(KERN_INFO "matrixmod_info: mc = [%d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n", mc[0], mc[1], mc[2], mc[3],
   mc[4], mc[5], mc[6], mc[7], mc[8], mc[9], mc[10], mc[11], mc[12], mc[13]);

  }else
	    printk(KERN_INFO "ERROR: comando no valido!!!\n");

  memset(entrada, '\0', COMMANDSIZE); // limpiamos entrada capturada
  memset(faux, '\0', 15); // limpiamos faux 
  memset(caux, '\0', 15); // limpiamos caux
  memset(vaux, '\0', 15); // limpiamos vaux

  count_read = 0; // Reinicia el cpntador de lectura para que se puede leer ante la nueva escritura

  return len;
}

/*
* Descripcion: Esta funcion detecta la posicion de un caracter (pasado por paramentro)
* sobre la cadena a la que apunte el puntero *p.
* @param c[2]: caracter a detectar.
* @param *p: puntero que apunta a la cadena donde se busca el espacio ("_").
* @param *ccf: puntero que apunta a la direccion de un entero donde se almacena
* la cantidad de avances hasta encontrar el espacio.
* @return: retorna la direccion donde se encontro el espacio
* sino se encontro se retorna la direccon inicial de *p.
*/
int detectar_esp(char c[2], char *p, int *ccf)
{
	int i, aux;
	aux = (int)p; // Guardamos auxiliarmente la direccion que tiene p

	p = p + 1; // Avanzamos un lugar suponiendo que el primero no es "_"
  	for (i = 0; i < COMMANDSIZE; i++)
  	{
  		if(strncmp(p,c, 1) == 0)
  			break;
  		else {
  			p = p + 1;
  			*ccf = *ccf + 1;
  		}
  	}

  	if(i == COMMANDSIZE) {
  		p = aux; // Volvemos a asignar direccion de inicio ya que no se encontro ningun "_"
  		printk("ERROR: No se encontro direccion de '_'\n");
  	}

  	return (int)p;
}

/*
* Descripcion: Esta funcion detecta la posicion de un caracter (pasado por paramentro)
* sobre la cadena a la que apunte el puntero *p.
* @param c[2]: caracter a detectar.
* @param *p: puntero que apunta a la cadena donde se busca el espacio ("_")
* la cantidad de avances hasta encontrar el espacio. Puedde mandar NULL si no
* se nescesita.
* @return: retorna la direccion donde se encontro el espacio
  sino se encontro se retorna la direccon inicial de *p.
*/
int detectar_char(char c[2], char *p)
{
	int i, aux;
	aux = (int)p; // Guardamos auxiliarmente la direccion que tiene p

	/* Buscamos direccion de "_" avanzando para adelante */
  	for (i = 0; i < COMMANDSIZE; i++)
  	{
  		if(strncmp(p,c, 1) == 0)
  			break;
  		else
  			p = p + 1;
  	}

  	if(i == COMMANDSIZE) {
  		p = aux; // Volvemos a asignar direccion de inicio ya que no se encontro ningun "_"
  		printk("ERROR: No se encontro direccion de '_'\n");
  	}

  	return (int)p;
}

/*
* Descripcion: toma las filas y columnas, como enteros, del parametro enviado por el usuario, en
* caracteres, en base a una entrada recibida como comando.
* @param *f: puntero a la direccion de la variable entera donde se almacena la cantidad de filas.
* @param *c: puntero a la direccion de la variable entera donde se almacena la cantidad de columnas.
* @param *entrada: puntero a la direccion de la entrada recibida por el usuario.
*/
void tomar_fc(int *f, int *c, char *entrada)
{
	char *p0; // Puntero destinado para que apunte a la direccion 0 de entrada
	char *s2; // Puntero donde almacenaremos la direccion de un caracter
   	char s1[10]; // Cadena de char donde se almacenara la ultima porcion de la entrada que son columnas
   	int ccf; // ccf: contador de cifras en filas
   	int t; // t: test funcion sscanf
  	
  	ccf = 0; // Inicializamos contador
  	printk(KERN_INFO "INFO: entrada capturada para MA: %s\n", entrada);
  	p0 = entrada; // Asignamos al puntero la direccion 0 de entrada
  	p0 = detectar_char("_", p0); // Se determina donde empiza el primer espacio dado por "_"
  	p0 = detectar_esp("_", p0, &ccf); // Avanzamos al proximo espacio para saltear las cifras de fila.

  	if(p0 == entrada) {
  		printk("matrixmod_error: No se pudiron tomar los parametros de filas/columnas ingresados."\
  			   "Verifique el comando ingresado es correcto.");
  		*f=*c=0; // Se asigna cero a las filas y columnas para detectar error en creacion de matriz
  	} else {
  		s2 = entrada; // Inicializamos direccion de s2
		s2 = detectar_char("_", s2)+1; // Detectamos el primer espacio "_" nuevamente.
		
		/* Si filas es numero negativo */ //--> Analizar este if que no tiene sentido ya que sscanf toma numeros negativos
  		if(entrada[(int)(s2)] == "-") // Se castea direccion de s2 a (int) para usar la direccion como entero
  			ccf = ccf -1;// Descontamos una cifra de fila por caracter de signo negativo

		strncpy(s1, s2, ccf); // Copiamos n=ccf caracteres, donde apunta s2, en s1.
		t = sscanf(s1,"%d", f); // Guardamos valor copiado en s1 como entero en f.
		if(t == 1) {
			printk(KERN_INFO "INFO: captura de filas para MA: %d\n", *f);
		} else
			printk(KERN_INFO "INFO: Fallo captura de filas para MA\n");
  		
		t = sscanf(p0,"_%d", c);
		if(t == 1) {
			printk(KERN_INFO "INFO: captura de columnas para MA: %d\n", *c);
		} else
			printk(KERN_INFO "INFO: Fallo captura de columnas para MA\n");
  		
		printk(KERN_INFO "INFO: Valor ccf : %d para MA\n", ccf);
		memset(s1, '\0', 10); // Limpiamos s1
	}
}

/*
* Descripcion: Esta funcion crea una matriz, verificando que los parametros de filas y columnas
* tomados desde una entrada del usuario sean correctos.(entrada: es un comando ingresado por el usuario).
* @param *f: puntero a la direccion de la variable entera donde se almacena la cantidad de filas.
* @param *c: puntero a la direccion de la variable entera donde se almacena la cantidad de columnas.
* @param pmc: posicion de matriz a crear sobre el vector mc[].
*/
void crear_rdp(int *f, int *c, struct matriz *m, int pmc)
{
	/* Tomar valores de filas y columnas segun la entrada */
  	//tomar_fc(f, c, entrada);

  	/* Verificamos filas y columnas */
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
  				printk(KERN_INFO "matrixmod_info: %s con id: %d creada exitosamente!!!\n", m->nombre, pmc);
				mc[pmc]=1; // Matriz con id: pmc creada
   			}
		}

		/* Si matriz existe */
		else if (mc[pmc] == 1)
			printk(KERN_INFO "matrixmod_error: %s con id: %d ya creada!!!\n", m->nombre, pmc);
  	}
}

/*
* Descripcion: Esta funcion crea los vectores de disparo asociados a la matriz de incidencia I
* en una matriz cuyas columnas indica la cantidad de disparos a crear y su cantidad de elementos.
* @param c: variable entera que indica la cantidad de columnas de la matriz I, para crear matriz de
* disparos con cxc elementos representando cada disparo vectorialmente.
* @param mc_id: identificador de matriz de disparos a crear.
*/
void crear_mdisparos(int c, int mc_id)
{
	cd = c; // Se asigna a cd la cantidad de columnas
	disparos.filas = cd;
	disparos.columnas = cd;

	identidad(&disparos, cd); // matriz disparos se crea como una matriz identidad c x c

	if(disparos.matriz != NULL)
		mc[mc_id] = 1; // Matriz de disparos creada con exito
	else
		printk(KERN_INFO "matrixmod_error: No se pudo crear Matriz con id: %d !!!\n", mc_id);
}

/*
* Descripcion: Esta funcion realiza el disparo sobre la RdP de acuerdo a un vector de disparo indicado
* por parametro, verificando la sensibilidad de RdP para ese disparo, actualizando o no el marcado actual.
* @param id_d: identificador de disparo a realizar sobre RdP.
* @param mode: indica el modo de disparo.
               - I: modo implicito, no actualiza el vector de marcado actual MA
               - E: modo explicito, actualiza el vector de marcado actual MA
* @return -1: si no se puede realizar disparo sobre RdP (RdP no esta sensibilizada para ese disparo).
* @return 1: si puedo realizar disparo sobre RdP (RdP estaba sensibilizada para ese disparo).
*/
int disparar(int id_d, char mode)
{
	/* Creo vector de disparo */ 
	//struct matriz vauxiliar;
	vauxiliar.filas = disparos.filas;
	vauxiliar.columnas =  1;

	transpuesta_fc(&vauxiliar, &disparos, vauxiliar.filas, 1, id_d);
	mc[5]=1; // Se creo vector disparo

	// Disparar
	int i, j, auxMN;
	for (i = 0; i < I.columnas; i++)
	{
		if(vauxiliar.matriz[i][0]==1)
		{
			for (j = 0; j < I.filas; j++)
			{
				if(mode == 'E')
				{
            MN.matriz[0][j] = MA.matriz[0][j] + I.matriz[j][i]*vauxiliar.matriz[i][0];
    				if(MN.matriz[0][j]< 0)
    					return -1;
        }
        else // modo de disparo implicito no intera almacenar el estado de MN ni MA pero si saberlo
        {
        		// Se almacena de manera auxiliar los valores del nuevo estado de la transicion, para verificar si esta sensibilizada
        		auxMN = MA.matriz[0][j] + I.matriz[j][i]*vauxiliar.matriz[i][0];
        		if(auxMN < 0)
          		return -1;
        }
			}
		}
	}

  	if(mode == 'E')
    	for (i = 0; i <I.filas ; i++)
    		MA.matriz[0][i] = MN.matriz[0][i];

	return 1;
}


/*
* Descripcion: Esta funcion realiza el disparo sobre la RdPG de acuerdo a un vector de disparo indicado
* por parametro, verificando la sensibilidad de la RdPG para ese disparo, actualizando o no el marcado actual.
* @param id_d: identificador de disparo a realizar sobre RdP.
* @param mode: indica el modo de disparo.
               - I: modo implicito, no actualiza el vector de marcado actual MA
               - E: modo explicito, actualiza el vector de marcado actual MA
* @return -1: si no se puede realizar disparo sobre RdPG (RdPG no esta sensibilizada para ese disparo).
* @return 1: si puedo realizar disparo sobre RdPG (RdPG estaba sensibilizada para ese disparo).
*/
int disparar_rdpg(int id_d, char mode)
{
  /* Creo vector de disparo representado por vauxiliar */ 
  vauxiliar.filas = disparos.filas;
  vauxiliar.columnas =  1;

  transpuesta_fc(&vauxiliar, &disparos, vauxiliar.filas, 1, id_d);
  mc[5]=1; // Se creo vector disparo

  /* Se calculo vector Ex*/
  cargar_Ex();

  // Disparar
  int i, j, auxMN, cvi = 0; /* cvi: contador de valores iguales entre MA y MN*/
  for (i = 0; i < I.columnas; i++)
  {
    if(vauxiliar.matriz[i][0]==1)
    {
      for (j = 0; j < I.filas; j++)
      {
        if(mode == 'E')
        {
            MN.matriz[0][j] = MA.matriz[0][j] + I.matriz[j][i]*vauxiliar.matriz[i][0]*Ex.matriz[0][i];
            if(MN.matriz[0][j]< 0)
              return -1;
        }
        else // modo de disparo implicito no intera almacenar el estado de MN ni MA pero si saberlo
        {
            // Se almacena de manera auxiliar los valores del nuevo estado de la transicion, para verificar si esta sensibilizada
            auxMN = MA.matriz[0][j] + I.matriz[j][i]*vauxiliar.matriz[i][0]*Ex.matriz[0][i];
            if(auxMN < 0)
              return -1;
        }
      }
    }
  }

  /* Verificacion de igualdad de estado actual y nuevo (debido a que Ex = [0 0 ... 0])*/
  for (i = 0; i <I.filas ; i++)
  {
     if (MA.matriz[0][i] == MN.matriz[0][i])
      cvi++;

     if(cvi == I.filas) /* contador igual a cantidad de plazas ?*/
     {
        cvi=0;
        // cargar MN a cero ...
        cargar_cero(&MN);
        return -1; // return -1 ya que no se realizo cambio de estado en la RdPG por Ex = [0 0 ... 0] -> caso exepcional!!!
     }
  }

  /* Actualizacion de marcado actual*/
  if(mode == 'E')
    for (i = 0; i <I.filas ; i++)
       MA.matriz[0][i] = MN.matriz[0][i];

  /* Actualizacion de vectores de RdPG*/
  cargar_E();
  cargar_Q();
  cargar_W();
  cargar_B();
  cargar_L();
  cargar_Ex();

  return 1;
}

/*
* Descripcion: Esta funcion...
* @param:
*/
void cargar_MA(void)
{
	int i;
  	if(mc[1]) /* Si MA existe */
  	{
  		for (i = 0; i < I.filas ; i++)
		{
			/* code */
			MA.matriz[0][i] = MI.matriz[0][i];
		}
  	}
}

/*
* Descripcion: Esta funcion se encarga de cargar el vector E, una vez que se completo la carga de MI.
* La funcion dispara todas las transiciones implitamente para determinar que transiciones estan sensibilizadas
* y cuales no lo estan determinando de esa manera el vector de transiciones sensibilizadas.
* @param: no se reciben parametros en la funcion.
*/
void cargar_E(void)
{
	int i;
    	for (i = 0; i < I.columnas; i++)
    	{
      		if(disparar(i,'I') == 1)
        		E.matriz[0][i] = 1;

      		else
        		E.matriz[0][i] = 0;
    	}
}

/*
* Descripcion: Esta funcion se encarga de cargar el vector Q, cuyos elementos qi son qi -> 0 si M(pi) != 0 o qi -> 1 en el resto de los casos
* estos valores se toman a partir del marcado actual MA.
* @param: no se reciben parametros en la funcion.
*/
void cargar_Q(void)
{
	int i;
    	for (i = 0; i < MA.columnas; i++)
    	{
      		if(MA.matriz[0][i] != 0)
        		Q.matriz[0][i] = 0;

      		else
        		Q.matriz[0][i] = 1;
    	}
}


/*
* Descripcion: Esta funcion se encarga de cargar el vector W, cuyos elementos wi son wi -> 1 si M(pi) != 0 o wi-> 0 en el resto de los casos
* estos valores se toman a partir del marcado actual MA.
* @param: no se reciben parametros en la funcion.
*/
void cargar_W(void)
{
  int i;
      for (i = 0; i < MA.columnas; i++)
      {
          if(MA.matriz[0][i] != 0)
            W.matriz[0][i] = 1;

          else
            W.matriz[0][i] = 0;
      }
}

/*
* Descripcion: Esta funcion se encarga de cargar el vector B, una vez que se completo la carga de MI, H y Q.
* El vector B que carga la funcion, es un vector de valores binarios de dimension mx1 (siendo m la cantidad de
* transiciones existentes en la RdP) que indica con un cero cuales transiciones estan inhibidas por el arco 
* inhibidor y con un uno las que no.
* @param: no se reciben parametros en la funcion.
*/
void cargar_B(void)
{
    int i, j;

    /* Inicializacion de valores en sensibilizados (uno binario) sobre vector B*/
    for(i=0; i < H.columnas; i++)
      if(B.matriz[0][i]!=1)
        B.matriz[0][i]=1;

    /* Calculo de valores inhibidos (cero binario) sobre vector B*/
    for(i=0; i< H.columnas; i++) /* lectura de transiciones de matriz H*/
      for(j=0; j< H.filas; j++) /* lectura de plazas de matriz H*/
          if(H.matriz[j][i] == 1)
             B.matriz[0][i] = H.matriz[j][i]*Q.matriz[0][j];

    printk(KERN_INFO "matrixmod_info: Actualizacion de vector B con exito!!!\n");
}

/*
* Descripcion: Esta funcion se encarga de cargar el vector L, una vez que se completo la carga de MI, R y W.
* El vector L que carga la funcion, es un vector de valores binarios de dimension mx1 (siendo m la cantidad de
* transiciones existentes en la RdP) que indica con un cero cuales transiciones estan inhibidas por el arco 
* y lector un uno las que no.
* @param: no se reciben parametros en la funcion.
*/
void cargar_L(void)
{
    int i, j;

    /* Inicializacion de valores en sensibilizados (uno binario) sobre vector L*/
    for(i=0; i < R.columnas; i++)
      if(L.matriz[0][i]!=1)
        L.matriz[0][i]=1;

    /* Calculo de valores inhibidos (cero binario) sobre vector L*/
    for(i=0; i< R.columnas; i++) /* lectura de transiciones de matriz R*/
      for(j=0; j< R.filas; j++) /* lectura de plazas de matriz R*/
          if(R.matriz[j][i] == 1)
             L.matriz[0][i] = R.matriz[j][i]*W.matriz[0][j];

    printk(KERN_INFO "matrixmod_info: Actualizacion de vector L con exito!!!\n");
}

/*
* Descripcion: Esta funcion se encarga de cargar el vector Ex, una vez que se completo la carga de E, B y L.
* El vector Ex que carga la funcion, es un vector de valores binarios de dimension mx1 (siendo m la cantidad de
* transiciones existentes en la RdP) que indica con un cero cuales transiciones estan inhibidas por la 
* conjuncion de todos arcos de la RdP y con un uno las que no.
* @param: no se reciben parametros en la funcion.
*/
void cargar_Ex(void)
{
    int i, j;

    /* Calculo de la conjuncion logica de todos los vectores para generar el vector Ex*/
    for(i=0; i < I.columnas; i++)
        Ex.matriz[0][i] = E.matriz[0][i]* B.matriz[0][i] * L.matriz[0][i];

    printk(KERN_INFO "matrixmod_info: Actualizacion del vector Ex con exito!!!\n");
}

/*
* Descripcion: Esta funcion se encarga de cargar la matriz/vector *m, que se indica como puntero, todos sus
* valores a cero.
* @param *m: puntero de la matriz/vectoe sobre la que se desea cargar a cero sus valores
*/
void cargar_cero(struct matriz *m)
{
  int i,j;
  for(i=0; i<m->filas; i++)
    for(j=0; j<m->columnas; j++)
      m->matriz[i][j] =0;
}


/*
* Descripcion: Esta funcion...
* @param:
*/
void tomar_transicion(char *entrada, int *transicion)
{
   	int t; // t: test funcion sscanf
  	
  	printk(KERN_INFO "INFO: entrada capturada para tomar nro de transicion: %s\n", entrada);

  	t = sscanf(entrada,"_%d", transicion);
	if(t == 1) {
		printk(KERN_INFO "matrixmod_info: nro de transicion tomada: %d\n", *transicion);
	}
	else
		printk(KERN_INFO "matrixmod_info: no se pudo tomar nro de transicion\n");
}

/*
* Descripcion: Esta funcion...
* @param:
*/
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

				} else { /* Se trata de caso filas: 1 cifra - columnas: 2 cifras - valor: 1 cifra*/
					strncat(faux, entrada, 1);
					strncat(caux, entrada+2, 2);
					strncat(vaux, entrada+5, 1);
		
					t = sscanf(faux,"fila %d", &f);
					t = sscanf(caux, "columna %d", &c);
					t = sscanf(vaux, "valor %d", &v);
				}
				
			} else {
				/* Error */	
				error = 2;
				f = c = v = 0;
			}
			
		} else {
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
    	
		if(f < m->filas && c < m->columnas && error < 1) {
			m->matriz[f][c]=v;
		}
}

static ssize_t matrixmod_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	int nr_bytes;
  	count_read++;
  	//nr_bytes=0;

  	/* Decirle a la aplicación que ya no hay nada que leer  "Para no copiar basura si llamas otra vez" */
  	/*if ((*off) > 0){
	  	printk(KERN_INFO "matrixmod: no hay nada que leer \n");
      		return 0;}*/
  	if((count_read % 2) == 0) {
  		printk(KERN_INFO "matrixmod: no hay nada que leer \n");
  		count_read = 0;
      		return 0;
  	}

  	switch(mostrar_mc) {
  	   case 0:
  		    if(mc[0]==1)
  			     nr_bytes = imprimir_matriz(&I, buf, len);
    
  		    else
			     printk(KERN_INFO "matrixmod_error: Matriz I no existe. \n");

		      break;
  	
  	   case 1:
  		    if(mc[1]==1)
  			   nr_bytes = imprimir_matriz(&MA, buf, len);
    
  		    else
			     printk(KERN_INFO "matrixmod_error: Vector MA no existe.\n");

		      break;

    	case 2:
      		if(mc[2]==1)
      			nr_bytes = imprimir_matriz(&disparos, buf, len);
        
      		else
    			   printk(KERN_INFO "matrixmod_error: Matriz mdisparos no existe.\n");
    		  
          break;

    	case 3:
      		if(mc[3]==1)
      			nr_bytes = imprimir_matriz(&MI, buf, len);
        
      		else
    			   printk(KERN_INFO "matrixmod_error: Vector MI no existe.\n");

    		  break;

    	case 4:
      		if(mc[4]==1)
      			nr_bytes = imprimir_matriz(&MN, buf, len);
        
      		else
    			   printk(KERN_INFO "matrixmod_error: Vector MN no existe.\n");

    		  break;

    	case 5:
      		if(mc[5]==1)
      			nr_bytes = imprimir_matriz(&vauxiliar, buf, len);
        
      		else
    			   printk(KERN_INFO "matrixmod_error: Vector disparo d no existe.\n");
    	   
          break;

    	case 6:
      		if(mc[6]==1)
      			nr_bytes = imprimir_matriz(&H, buf, len);
        
      		else
    			   printk(KERN_INFO "matrixmod_error: Matriz H no existe.\n");
    	   
          break;


    	case 7:
        		if(mc[7]==1)
          		nr_bytes = imprimir_matriz(&E, buf, len);
      
        		else
        			printk(KERN_INFO "matrixmod_error: Vector E no existe.\n");
    	       
            break;

    	case 8:
        		if(mc[8]==1)
          		nr_bytes = imprimir_matriz(&R, buf, len);
      
        		else
        			printk(KERN_INFO "matrixmod_error: Matriz R no existe.\n");
            
            break;

    	case 9:
        		if(mc[9]==1)
          		nr_bytes = imprimir_matriz(&Q, buf, len);
      
        		else
        			printk(KERN_INFO "matrixmod_error: Vector Q no existe.\n");
            
            break;

      case 10:
            if(mc[10]==1)
              nr_bytes = imprimir_matriz(&W, buf, len);
      
            else
              printk(KERN_INFO "matrixmod_error: Vector W no existe.\n");
            
            break;

      case 11:
            if(mc[11]==1)
              nr_bytes = imprimir_matriz(&L, buf, len);

            else
              printk(KERN_INFO "matrixmod_error: Vector L no existe.\n");

            break;

      case 12:
            if(mc[12]==1)
              nr_bytes = imprimir_matriz(&B, buf, len);

            else
              printk(KERN_INFO "matrixmod_error: Vector B no existe.\n");

            break;

      case 13:
            if(mc[13]==1)
              nr_bytes = imprimir_matriz(&Ex, buf, len);

            else
              printk(KERN_INFO "matrixmod_error: Vector Ex no existe.\n");

            break;
	
  	//default:
  	}

  	(*off)+=len;  /* Actualizo el puntero de archivo */
  	//vfree(kbuf);
  	return nr_bytes; 
}

static const struct file_operations proc_entry_fops = {
    .open = matrixmod_open,
    .release = matrixmod_release,
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
	int i,j,k, nr_bytes; // i,j : recorren la matriz
			     // k: va corriendo el puntero del buffer del kernel kbuf;
			     // nr_bytes:
	k = 0;
	nr_bytes = 0;

	for(i = 0; i<m->filas; i++)
	{
		for(j = 0; j<m->columnas; j++)
		{
			if(m->matriz[i][j] >= 0) {
				kbuf[k]=(char)32; /* Ascii de espacio */ k++;
				kbuf[k]=(char)32; k++;
				kbuf[k]=(char)(m->matriz[i][j] + 48); k++;
			} else {
				kbuf[k]=(char)32; k++;
				kbuf[k]=(char)45;/* Ascii de signo negativo */ k++;
				//kbuf[k]=(char)(48 + (48 - (A.matriz[i][j] + 48))); k++;
				kbuf[k]=(char)(48 - m->matriz[i][j]); k++;  
				/* Si A[i][j]=-1 --> Esto seria: 48 - (-1) = 49 que es el codigo ASCII de 1, es decir que convertimos
				   los negativos a positivos para ser mostrados de manera correcta en la entrada estandar. */
			}
		} // Fin for 1
		
		kbuf[k]=(char)10; /* Ascii salto de linea */ k++;
	} // Fin for 2
	
	nr_bytes=k+1;
		
	if (len< nr_bytes) {
		printk(KERN_INFO "matrixmod: No queda espacio en el dispositivo \n");
	    	return -ENOSPC; // No queda espacio en el dispositivo
	}
 
   	/* Transferencia de datos desde el espacio kernel al espacio de usuario */  
        if(copy_to_user(buf, kbuf, nr_bytes)) {
		printk(KERN_INFO "matrixmod: Argumento invalido\n");
   		return -EINVAL; // Argumento invalido
	}
	
	return nr_bytes;
}

/* Asigna cero a las filas y columnas de todas las matrices */
void iniciar_matrices(void )
{
	/* Se inicializan a cero las filas y columnas de todos los vecotres y matrices*/
  I.filas = I.columnas = 0;
	MA.filas = MA.columnas = 0;
	disparos.filas = disparos.columnas = 0;
	MI.filas = MI.columnas = 0;
	MN.filas = MN.columnas = 0;
	H.filas = H.columnas = 0;
	E.filas = E.columnas = 0;
	Q.filas = Q.columnas = 0;
  R.filas = R.columnas = 0;
  W.filas = W.columnas = 0;
  L.filas = L.columnas = 0;
  B.filas = B.columnas = 0;
  Ex.filas = Ex.columnas = 0;

	/* Se establece el nombre asociado a cada uno de los vectores y matrices */
	strcpy(I.nombre, "Matriz I");
	strcpy(MA.nombre, "Vector MA");
	strcpy(disparos.nombre, "Matriz de vectores de disparos");
	strcpy(MI.nombre, "Vector MI");
	strcpy(MN.nombre, "Vector MN");
	strcpy(vauxiliar.nombre, "Vector de disparo");
	strcpy(H.nombre, "Matriz H");
	strcpy(E.nombre, "Vector E");
	strcpy(R.nombre, "Matriz R");
  strcpy(Q.nombre, "Vector Q");
  strcpy(W.nombre, "Vector W");
  strcpy(L.nombre, "Vector L");
  strcpy(B.nombre, "Vector B");
  strcpy(Ex.nombre, "Vector Ex");

  /* Inicializacion de mc (vector detector de matrices creadas) todos en cero (no creadas) */
	mc[0] = 0; 		// matriz I no creada
	mc[1] = 0; 		// vector MA no creado
	mc[2] = 0; 		// matriz de vectores de disparos no creada
	mc[3] = 0; 		// vector MI no creado
	mc[4] = 0; 		// vector MN no creado
	mc[5] = 0; 		// no se creo vector disparo
	mc[6] = 0; 		// matriz H no creada
	mc[7] = 0;		// vector E no creado
	mc[8] = 0;    // matriz R no creada
	mc[9] = 0;		// vector Q no creado
  mc[10] = 0;   // vector W no creado
  mc[11] = 0;   // vector L no creado
  mc[12] = 0;   // vector B no creado
  mc[13] = 0;   // vector Ex no creado
	mostrar_mc = 0; 	// puntero de matriz a mostrar por defecto en cero = matriz de incidencia I
}

int init_modlist_module(void)
{
	int ret = 0;

  	/* en matrixmod defenido en /proc, solo podemos usar las funciones
	   defenidas en proc_entry_fops */
  	proc_entry = proc_create( "matrixmod", 0666, NULL, &proc_entry_fops); 
  	if(proc_entry == NULL) {
    		ret = -ENOMEM; // No hay bastante espacio
    		printk(KERN_INFO "matrixmod: No se puede crear la entrada /proc\n");
  	} else {
		printk(KERN_INFO "matrixmod: Modulo cargado en kernel.\n");
		//cargar_matriz_uno(&A, 10, 10);
		iniciar_matrices();
  	}

  	return ret;
}

void exit_modlist_module(void)
{
	// Agregar --> eliminar espacio en memoria
	if(mc[0] == 1)
		liberar_mem(&I);

	if(mc[1] == 1)
		liberar_mem(&MA);

	if(mc[2] == 1)
		liberar_mem(&disparos);

	if(mc[3] == 1)
		liberar_mem(&MI);

	if(mc[4] == 1)
		liberar_mem(&MN);

	if(mc[5] == 1)
		liberar_mem(&vauxiliar);

	if(mc[6] == 1)
		liberar_mem(&H);

  if(mc[7] == 1)
    liberar_mem(&E);

	if(mc[8] == 1)
    liberar_mem(&R);

  if(mc[9] == 1)
    liberar_mem(&Q);

  if(mc[10] == 1)
    liberar_mem(&W);

  if(mc[11] == 1)
    liberar_mem(&L);

  if(mc[12] == 1)
    liberar_mem(&B);

  if(mc[13] == 1)
    liberar_mem(&Ex);

  remove_proc_entry("matrixmod", NULL); // Eliminar la entrada del /proc
  printk(KERN_INFO "matrixmod: Modulo descargado de kernel.\n");
}

module_init(init_modlist_module);
module_exit(exit_modlist_module);

