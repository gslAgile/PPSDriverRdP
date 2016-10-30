# PPSDriverRdP

## [Laboratorio de Arquitectura de Computadoras - FCEFyN - UNC](http://computacion.efn.uncor.edu/lac)

### Practica Profesional Supervisada - PPS

  **Director de PPS:** Ing. Orlando Micolini
  
  **Integrantes:** Garcia Cannata Nicolas - Sosa Ludueña Gabriel
  
  **Tema:** Desarrollo de modulos sobre el kernel de Linux


<h1 align="center" >Presentacion - matrixmod.c</h1>

<p align="center">
 
<img src="https://github.com/gslAgile/PPSDriverRdP/blob/master/matrixmod/imagenes/img_RdP_3.png" title="Red de Petri.">
<div align="center"></div>
 
</p>

 * Modulo matrixmod.c carga cualquier RdP desde archivos de usuario en el kernel de Linux (no importa dimension de matriz).
 
 * Se genera en base a la RdP matriz de disparos posibles a realizar sobre la misma.
 
 * Se puede disparar cualquiera de las transiciones sobre la RdP segun se le indique con comando.
 
 * Todo el control de la RdP se puede llevar a cabo desde programa C de usuario a traves de llamadas de sistema open, read/write, close.
 
 * El manejo de matrices es extendible y compatible con librerias de C.
 
 * Matriz dinamica en C --> [enlace externo](https://es.wikibooks.org/wiki/Programaci%C3%B3n_en_C/Matrices_Dinamicas)
   - Se realizo de la misma manera, solo que se utilizo kmalloc() y kfree() para el manejo de memoria dinamica en el kernel de Linux.
 
 * Consultas tesis/informe Emi Daniele
   - Como se pasan los parametros de las funciones que utiliza el driver?
   - Como vincula el driver cargado en el kernel con el programa de usuari realizado en C? Qe funciones (llamadas al sistema) utiliza?
 
 ### Correcciones
 * Correccion de uso de funciones open() y close() solo una vez en el programa c de usuario.
   - Se modifico codigo de modulo matrixmod.c en parametro offset que no permitia mas de una lectura escritura creando una variable que cuente las lecturas.
   - **Duda:** pq cuando realizamos un cat sobre modulo (read) se llama mas de una vez ?
   - Se modifico progrma c de usuario llamada_matrixmod.c para hacer el uso correcto del modulo matrixmod.c.
 
 * **Pendiente:** verificar la carga de matrices de archivos de usuario desde el programa de usuario C.
