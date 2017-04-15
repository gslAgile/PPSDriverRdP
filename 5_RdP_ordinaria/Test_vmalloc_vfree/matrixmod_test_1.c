#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm-generic/uaccess.h>
#include <linux/list.h>
#include "../matrixmod/matrices.h" // ingreso a directorio matrixmod para incluir la libreria del modulo matrixmod.c

#define SUCCESS 0;

/* Licencia del módulo */
MODULE_LICENSE("GPL");

/*Autores*/
MODULE_AUTHOR("Ing. Micolini Orlando, Garcia Cannata, Sosa Ludueña");

/* Descripcion del modulo*/
MODULE_DESCRIPTION("Test del funcionamiento de las funciones vmalloc y vfree sobre "\
					"el kernel de Linux utilizando una entrada en /proc");


#define BUFFER_LENGTH       2048
#define COMMANDSIZE			256

// Funciones:
void iniciar_matrices(void);
void crear_rdp(int *f, int *c, int pmc);
void tomar_valor(int *valor, char *entrada);
int detectar_esp(char c[2] ,char *p, int *ccf);
int detectar_char(char c[2] ,char *p);
int imprimir_matriz(char *buf, size_t len);



static struct proc_dir_entry *proc_entry; // entrada de /proc

// Variables Globales
static int Device_Open = 0; /* es un device open? */
							/* Uso para prevenir multiples accesos en el dispositivo */
struct matriz vector_m[100]; // Vector de estructuras de Matrices de prueba
int pmc; // pmc: puntero para detectar la cantidad de matrices creadas en el modulo.
int mostrar_mc; // entero identificatorio de cual de las matrices creadas mostrara para funcion read
int count_read = 0; // contador de lecturas en modulo


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
    /* No estamos listos para nuestro siguiente llamada*/
    /*
    * Decrementar el contador de uso, o de lo contrario una vez que ha
    * abierto el archivo, usted nunca se deshacera del módulo.
    */
    module_put(THIS_MODULE);
    return SUCCESS;
}



static ssize_t matrixmod_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
  int available_space = BUFFER_LENGTH-1; // espacio disponible
  int f, c; 		// f: filas
  					// c: columnas

  char kbuf[BUFFER_LENGTH];//
  char entrada[COMMANDSIZE];

  /*if ((*off) > 0)  La aplicación puede escribir en esta entrada una sola vez !! 
    return 0;*/
  
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
  if( strcmp(kbuf,"crear matrices\n") == 0) {

  	int i;
  	f = 50;
  	c = 50;
	/* Creamos matriz I segun entrada recibida*/
  	for(i=0 + pmc; i< 10 + pmc; i++)
  	{
		crear_rdp(&f, &c, i);/* i: hace referencia a vector_m[i] para detectar que se creo
							       una matriz en referencia a esa posicion*/
  	}
	pmc = pmc+10;
	 
  }else if(sscanf(kbuf,"mostrar_mc %s",entrada) == 1){
	
	tomar_valor(&mostrar_mc, entrada);// se selecciona identificador para mostrar el vector_m[i]
	printk(KERN_INFO "matrixmod_info: Se asigna Matriz %d para mostrar en funcion read().\n", mostrar_mc);

  }else
	    printk(KERN_INFO "ERROR: comando no valido!!!\n");

  memset(entrada, '\0', COMMANDSIZE); // limpiamos entrada capturada

  count_read = 0; // Reinicia el cpntador de lectura para que se puede leer ante la nueva escritura

  return len;
}

/*
* Descripcion: Esta funcion crea una matriz, verificando que los parametros de filas y columnas
* tomados desde una entrada del usuario sean correctos.(entrada: es un comando ingresado por el usuario)
* @param *f: puntero a la direccion de la variable entera donde se almacena la cantidad de filas.
* @param *c: puntero a la direccion de la variable entera donde se almacena la cantidad de columnas.
* @param pmc: posicion de matriz a crear con respecto a vector_m
*/
void crear_rdp(int *f, int *c, int pmc)
{
	/* Tomar valores de filas y columnas segun la entrada*/
  	//tomar_fc(f, c, entrada);

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
  		if(vector_m[pmc].filas == 0 && *f > 0 && vector_m[pmc].columnas == 0 && *c > 0)
		{
			vector_m[pmc].filas = *f;
			vector_m[pmc].columnas = *c;
			printk(KERN_INFO "matrixmod_info: Se asigno %d filas en matriz exitosamente!!!\n", *f);
			printk(KERN_INFO "matrixmod_info: Se asigno %d columnas en matriz exitosamente!!!\n", *c);

			if(vector_m[pmc].filas > 0 && vector_m[pmc].columnas > 0 ){
  				cargar_matriz_cero((vector_m+pmc), *f, *c);
  				printk(KERN_INFO "matrixmod_info: Matriz con id: %d creada exitosamente!!!\n", pmc);
   			}
		}
  	}
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
	int i, aux;
	aux = (int)p; // Guardamos auxiliarmente la direccion que tiene p

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

  	return (int)p;
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
	int i, aux;
	aux = (int)p; // Guardamos auxiliarmente la direccion que tiene p

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

  	return (int)p;
}


/*
* Descripcion: toma las filas y columnas, como enteros, del parametro enviado por el usuario, en
* caracteres, en base a una entrada recibida como comando.
* @param *f: puntero a la direccion de la variable entera donde se almacena la cantidad de filas.
* @param *c: puntero a la direccion de la variable entera donde se almacena la cantidad de columnas.
* @param *entrada: puntero a la direccion de la entrada recibida por el usuario.
*/
void tomar_valor(int *valor, char *entrada)
{
	char *p0; // puntero destinado para que apunte a la direccion 0 de entrada.
	char *s2; // puntero donde almacenaremos la direccion de un caracter
    char s1[10]; // cadena de char donde se almacenara la ultima porcion de la entrada que son columnas
    int ccf; // ccf: contador de cifras en filas
    int t; // t: test funcion sscanf
  	
  	ccf = 0; // inicializamos contador
  	printk(KERN_INFO "INFO: entrada capturada: %s\n", entrada);
  	p0 = entrada; // asignamos al puntero la direccion 0 de entrada
  	p0 = detectar_char("_", p0); // se determina donde empiza el primer espacio dado por "_"
  	p0 = detectar_esp("_", p0, &ccf); // avanzamos al proximo espacio para saltear las cifras de fila.

  	if(p0 == entrada)
  	{
  		printk("matrixmod_error: No se pudiron tomar los parametros de filas/columnas ingresados."\
  			   "Verifique el comando ingresado es correcto.");
  		*valor=0; // se asigna cero al valor para evitar errores
  	}

  	else{
  		s2 = entrada; // inicializamos direccion de s2
		s2 = detectar_char("_", s2)+1; // detectamos el primer espacio "_" nuevamente.
		
		/*Si filas es numero negativo*/ //--> analizar este if que no tiene sentido ya que sscanf toma numeros negativos
  		if(entrada[(int)(s2)] == "-") // se castea direccion de s2 a (int) para usar la direccion como entero
  			ccf = ccf -1;// descontamos una cifra de fila por caracter de signo negativo

		strncpy(s1, s2, ccf); // copiamos n=ccf caracteres, donde apunta s2, en s1.
		t = sscanf(s1,"%d", valor); // guardamos valor copiado en s1 como entero en f.
		if(t == 1)
		{
			printk(KERN_INFO "INFO: captura de valor: %d\n", *valor);
		}
		else
			printk(KERN_INFO "INFO: Fallo captura de valor\n");
  		
		printk(KERN_INFO "INFO: Valor ccf : %d para posicion de matriz\n", ccf);
		memset(s1, '\0', 10); // limpiamos s1
	}
}



static ssize_t matrixmod_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
  
  int nr_bytes;
  count_read++;
  //nr_bytes=0;

  /* Decirle a la aplicación que ya no hay nada que leer  "Para no copiar basura si llamas otra vez" */
  /*if ((*off) > 0){
	  printk(KERN_INFO "matrixmod: no hay nada que leer \n");
      return 0;}*/
  if((count_read % 2) == 0)
  {
  		printk(KERN_INFO "matrixmod: no hay nada que leer \n");
  		count_read = 0;
      	return 0;
  }

	if(pmc>0)
		nr_bytes=imprimir_matriz(buf, len);

  (*off)+=len;  /* Actualizo el puntero de archivo */
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
int imprimir_matriz(char *buf, size_t len)
{
	char kbuf[BUFFER_LENGTH] = "";	
	int i,j,k, nr_bytes;// i,j : recorren la matriz
			  			// k: va corriendo el puntero del buffer del kernel kbuf;
						// nr_bytes:
	k=0;
	nr_bytes=0;

	for(i=0; i<vector_m[mostrar_mc].filas; i++){
		for(j=0; j<vector_m[mostrar_mc].columnas; j++)
		{
			if(vector_m[mostrar_mc].matriz[i][j] >= 0 && k-5 < BUFFER_LENGTH ){ // k-5: para tener en cuenta una vuelta mas y el caracter final
				kbuf[k]=(char)32;/* Ascii de espacio*/ k++;
				kbuf[k]=(char)32; k++;
				kbuf[k]=(char)(vector_m[mostrar_mc].matriz[i][j] + 48); k++;
				}
	
			else if(k-5 < BUFFER_LENGTH){
				kbuf[k]=(char)32; k++;
				kbuf[k]=(char)45;/* Ascii de signo negativo*/ k++;
//				kbuf[k]=(char)(48 + (48 - (A.matriz[i][j] + 48))); k++;
				kbuf[k]=(char)(48 - vector_m[mostrar_mc].matriz[i][j]); k++;  
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
	int i;
	for(i=0; i < 100; i++)
		vector_m[i].filas= vector_m[i].columnas=0;

	pmc = 0;
	mostrar_mc = 0;
}


int init_modlist_module( void )
{
  int ret = 0;

  /* en matrixmod defenido en /proc, solo podemos usar las funciones
	 defenidas en proc_entry_fops */
  proc_entry = proc_create( "matrixmod_test_1", 0666, NULL, &proc_entry_fops); 
  if (proc_entry == NULL) {
    ret = -ENOMEM; // No hay bastante espacio
    printk(KERN_INFO "matrixmod_test_1: No se puede crear la entrada /proc\n");
	
  }else {
    printk(KERN_INFO "matrixmod_test_1: Modulo cargado.\n");
	//cargar_matriz_uno(&A, 10, 10);
	iniciar_matrices();
  }

  return ret;

}


void exit_modlist_module( void )
{
  // eliminar espacio en memoria

	int i;
	if(pmc > 0)
		for(i=0; i < pmc; i++)
			liberar_mem(vector_m + i);

	pmc = 0;

  remove_proc_entry("matrixmod_test_1", NULL); // eliminar la entrada del /proc
  printk(KERN_INFO "matrixmod_test_1: Modulo descargado.\n");
}


module_init( init_modlist_module );
module_exit( exit_modlist_module );
