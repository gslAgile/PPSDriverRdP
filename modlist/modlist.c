#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm-generic/uaccess.h>
#include <linux/list.h>

/* Licencia del módulo */
MODULE_LICENSE("GPL");

/*Autores*/
MODULE_AUTHOR("Ing. Micolini Orlando, Garcia Cannata, Sosa Ludueña");

/* Descripcion del modulo*/
MODULE_DESCRIPTION("Implementa una lista de enteros en un módulo"\
					"de kernel administrable por una entrada en /proc");


#define BUFFER_LENGTH       100

static struct proc_dir_entry *proc_entry; // entrada de /proc

struct list_item{
  int data;
  struct list_head links;
};
static struct list_head my_list; // nodo fantasma

void cleanup(void){
  struct list_item *item= NULL;
  struct list_head *cur_node=NULL;
  struct list_head *aux=NULL;
  
  list_for_each_safe(cur_node, aux, &my_list){
      item=list_entry(cur_node, struct list_item, links);
      list_del(cur_node);
      vfree(item);
  }
}
static ssize_t modlist_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {

  int available_space = BUFFER_LENGTH-1; // espacio disponible
  int num; //
  char kbuf[BUFFER_LENGTH];//

  struct list_item *item = NULL;
  struct list_head *cur_node = NULL;

  if ((*off) > 0) /* La aplicación puede escribir en esta entrada una sola vez !! */
    return 0;
  
  if (len > available_space) {
    printk(KERN_INFO "modlist: No hay espacio sufuciente!!\n");
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
  if( sscanf(kbuf,"add %d",&num) == 1) { 
    item=(struct list_item *) vmalloc(sizeof (struct list_item));
    item->data = num;
    //static inline void list_add_tail(struct list_head *new, struct list_head *head)
    list_add_tail(&item->links, &my_list);
  }else if( sscanf(kbuf,"remove %d",&num) == 1) { 
    struct list_head *aux = NULL;
    // iterrar sobre una lista de forma segura para no eliminar la entrada
    list_for_each_safe(cur_node, aux, &my_list) { 
      /* item points to the structure wherein the links are embedded */ 
      item = list_entry(cur_node, struct list_item, links);
      if((item->data) == num){
        list_del(cur_node);
        vfree(item);
      }
    }
  }else if ( strcmp(kbuf,"cleanup\n") == 0){ // strcmp() return : 0 -> si son iguales 
    cleanup();
  }else{
    printk(KERN_INFO "ERROR: comando no valido!!!\n");
  }
  return len;
}

static ssize_t modlist_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
  
  int nr_bytes;
  struct list_item *item = NULL; 
  struct list_head *cur_node = NULL; 
  
  char kbuf[BUFFER_LENGTH] = "";
  char *list_string = kbuf;

  /* Decirle a la aplicación que ya no hay nada que leer  "Para no copiar basura si llamas otra vez" */
  if ((*off) > 0)
      return 0;
  
  
  list_for_each(cur_node, &my_list) { 
    /* puntero al espacio de la estructura en la que los enlaces están alojados */ 
    item = list_entry(cur_node, struct list_item, links); // item: espacio
    list_string += sprintf(list_string, "%d\n", item->data);
  }

  nr_bytes=list_string-kbuf;
    
  if (len<nr_bytes)
    return -ENOSPC; //No queda espacio en el dispositivo
  
    /* Transferencia de datos desde el espacio kernel al espacio de usuario */  
  if (copy_to_user(buf, kbuf, nr_bytes))
    return -EINVAL; //Argumento invalido
    
  (*off)+=len;  /* Actualizo el puntero de archivo */
  //vfree(kbuf);
  return nr_bytes; 
}

static const struct file_operations proc_entry_fops = {
    .read = modlist_read,
    .write = modlist_write,    
};



int init_modlist_module( void )
{
  int ret = 0;
  INIT_LIST_HEAD(&my_list);

  /* en modlist defenido en /proc, solo podemos usar las funciones
	 defenidas en proc_entry_fops */
  proc_entry = proc_create( "modlist", 0666, NULL, &proc_entry_fops); 
  if (proc_entry == NULL) {
    ret = -ENOMEM; // No hay bastante espacio
    printk(KERN_INFO "modlist: No se puede crear la entrada /proc\n");
  }else {
    printk(KERN_INFO "modlist: Modulo cargado.\n");
  }

  return ret;

}


void exit_modlist_module( void )
{
  cleanup();
  remove_proc_entry("modlist", NULL); // eliminar la entrada del /proc
  printk(KERN_INFO "modlist: Modulo descargado.\n");
}


module_init( init_modlist_module );
module_exit( exit_modlist_module );
