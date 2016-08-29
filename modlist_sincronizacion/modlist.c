#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm-generic/uaccess.h>
#include <linux/list.h>
#include <linux/list_sort.h>
#include <linux/spinlock.h>

/*Configuracion del proyecto*/

/* Licencia del módulo */
MODULE_LICENSE("GPL");

/*Autores*/
MODULE_AUTHOR("Ing. Micolini Orlando, Garcia Cannata, Sosa Ludueña");

/* Descripcion del modulo*/
MODULE_DESCRIPTION("Implementa una lista de enteros en un módulo"\
					"de kernel administrable por una entrada en /proc utilizando spinlocks para la sincronizacion.");

/*Prototipos*/
void cleanup(void);
static ssize_t modlist_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);
static ssize_t modlist_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
int init_modlist_module( void );
void exit_modlist_module( void );
int cmp(void *priv, struct list_head *a, struct list_head *b);



//<<<<<<<<<<<<<<<<<============================>>>>>>>>>>>>>>>>>>>//
//<<<<<<<<<<<<<<<<<<<< Definiciones Globales >>>>>>>>>>>>>>>>>>>>>//
//<<<<<<<<<<<<<<<<<============================>>>>>>>>>>>>>>>>>>>//
#define BUFFER_LENGTH       100

static const struct file_operations proc_entry_fops = {
    .read = modlist_read,
    .write = modlist_write,
};

static struct proc_dir_entry *proc_entry; // entrada de /proc

struct list_item{
  int data;
  struct list_head links;
};

static struct list_head my_list; // nodo fantasma

/* Inicia el spinlocks */
DEFINE_SPINLOCK(mtx); // mutex


//<<<<<<<<<<<<<<<<<============================>>>>>>>>>>>>>>>>>>>//
//<<<<<<<<<<<<<<<<< Implementecaion de metodos >>>>>>>>>>>>>>>>>>>//
//<<<<<<<<<<<<<<<<<============================>>>>>>>>>>>>>>>>>>>//

void cleanup(void){
  struct list_item *item= NULL;
  struct list_head *cur_node=NULL;
  struct list_head *aux=NULL;

  spin_lock(&mtx); /* Se adquiere el spinlocks */
     list_for_each_safe(cur_node, aux, &my_list){
         item=list_entry(cur_node, struct list_item, links);
         list_del(cur_node);
         vfree(item);
     }
  spin_unlock(&mtx); /* Se libera el spinlocks */

}

int cmp(void *priv, struct list_head *a, struct list_head *b) {
  int ret=-2;

  struct list_item *itemA = NULL;
  struct list_item *itemB = NULL;

  itemA = list_entry(a, struct list_item, links);
  itemB = list_entry(b, struct list_item, links);

  if((itemA != NULL) || (itemB != NULL)) {
    if(itemA->data < itemB->data)
      ret = -1;
    else if(itemA->data > itemB->data)
      ret = 1;
    else
      ret = 0;
  }
  return ret;
}


static ssize_t modlist_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {

  int available_space = BUFFER_LENGTH-1;
  int num;
  char kbuf[BUFFER_LENGTH];

  struct list_item *item = NULL;
  struct list_head *cur_node = NULL;

  if ((*off) > 0) /* La aplicación puede escribir en esta entrada una sola vez !! */
    return 0;

  if (len > available_space) {
    printk(KERN_INFO "modlist: not enough space!!\n");
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
  *off+=len;            /* Actualizo el puntero del archivo */

  /*sscanf() return : el nº de elementos asignados*/
  if( sscanf(kbuf,"add %d",&num) == 1) {
    item=(struct list_item *) vmalloc(sizeof (struct list_item));
    item->data = num;
    
		spin_lock(&mtx);/* Se adquiere el spinlocks */
       //static inline void list_add_tail(struct list_head *new, struct list_head *head)
       list_add_tail(&item->links, &my_list);
    spin_unlock(&mtx);/* Se libera el spinlocks */

  }else if( sscanf(kbuf,"remove %d",&num) == 1) {
    struct list_head *aux = NULL;
    
		spin_lock(&mtx);/* Se adquiere el spinlocks */
       // iterrar sobre una lista de forma segura para no eliminar la entrada
       list_for_each_safe(cur_node, aux, &my_list) {
         /* item points to the structure wherein the links are embedded */
         item = list_entry(cur_node, struct list_item, links);
         if((item->data) == num){
           list_del(cur_node);
           vfree(item);
         }
       }
    spin_unlock(&mtx);/* Se libera el spinlocks */

  }else if ( strcmp(kbuf,"cleanup\n") == 0){ // strcmp() return : 0 -> si son iguales
    cleanup();
  }else if(strcmp(kbuf,"sort\n") == 0) {
    /**
     * list_sort - ordena una lista
     * @priv: dato privado, opaque to list_sort(), pasado por @cmp
     * @head: la lista para ordenar
     * @cmp: la funcion de comparacion de elementos
     *
     * Esta funcion implementa un "merge sort", que tiene una 
	 * complejidad de orden O(nlog(n))
     *
     * La funcion de comparacion @cmp debe retornar un valor negativo si @a
     * debe ordenarce antes de @b, y un valor positivo si @a debe ordernarce
     * despues de @b. Si @a y @b son iguales, y su orden relativo original
     * se conserva, la funcion @cmp debe retornar 0.
    */

    spin_lock(&mtx);/* Se adquiere el spinlocks */
      list_sort(NULL, &my_list, *cmp);
    spin_unlock(&mtx);/* Se libera el spinlocks */

  }else{
    printk(KERN_INFO "ERROR: comando no valido!!!\n");
  }
  return len;
}

static ssize_t modlist_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {

  int nr_bytes=0;//,cont=0;
  struct list_item *item = NULL;
  struct list_head *cur_node = NULL;
  int count=0;

  char kbuf[BUFFER_LENGTH] = "";
  char aux[10];
  //char *kbuf = (char*)vmalloc(sizeof(char)*len); // no es muy eficiente porque cada página ocupa 32K
  char *list_string = kbuf;

  /* Decirle a la aplicación que ya no hay nada que leer  "Para no copiar basura si llamas otra vez" */
  if ((*off) > 0)
      return 0;

  spin_lock(&mtx);/* Se adquiere el spinlocks */
     list_for_each(cur_node, &my_list) {
       /* item points to the structure wherein the links are embedded */
       item = list_entry(cur_node, struct list_item, links);
	if(sprintf(aux,"%d\n", item->data)+nr_bytes/*count*/ >= BUFFER_LENGTH-1)
		break;
	printk(KERN_INFO "strlen ==> %d\n",(int)strlen(list_string));
	printk(KERN_INFO "buff ==> %d\n",BUFFER_LENGTH);

        count= sprintf(list_string, "%d\n", item->data);
	list_string+=count;
	nr_bytes+=count;
     }
  spin_unlock(&mtx);/* Se libera el spinlocks */

  if (len<nr_bytes) {
    //vfree(kbuf);
    return -ENOSPC; //No queda espacio en el dispositivo
  }

  /* Transferencia de datos desde el espacio kernel al espacio de usuario */ 
  if (copy_to_user(buf, kbuf, nr_bytes)) {
    //vfree(kbuf);
    return -EINVAL; //Argumento invalido
  }

  (*off)+=len;  //* Actualizo el puntero de archivo */
  //vfree(kbuf);
  return nr_bytes;
}





int init_modlist_module( void )
{
  int ret = 0;
  INIT_LIST_HEAD(&my_list);

  /* en modlist defenido en /proc, solo podemos usar las funciones defenidas en proc_entry_fops */
  proc_entry = proc_create( "modlist", 0666, NULL, &proc_entry_fops);
  if (proc_entry == NULL) {
    ret = -ENOMEM; // No hay bastante espacio
    printk(KERN_INFO "modlist: No se puede crear la entrada en /proc\n");
  }else {
    printk(KERN_INFO "Modlist: module cargado\n");
  }

  return ret;

}


void exit_modlist_module( void )
{
  cleanup();
  remove_proc_entry("modlist", NULL); // eliminar la entrada del /proc
  printk(KERN_INFO "modlist: Module descargado.\n");
}

module_init( init_modlist_module );
module_exit( exit_modlist_module );
