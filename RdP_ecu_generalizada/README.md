# PPSDriverRdP

## [Laboratorio de Arquitectura de Computadoras - FCEFyN - UNC](http://computacion.efn.uncor.edu/lac)

### Practica Profesional Supervisada - PPS

  **Director de PPS:** Ing. Orlando Micolini
  
  **Integrantes:** Garcia Cannata Nicolas - Sosa Ludueña Gabriel
  
  **Tema:** Desarrollo de modulos sobre el kernel de Linux


<h1 align="center" >Resumen RdP Generalizada</h1>

**Referencia resumen:** [enlace externo](http://sedici.unlp.edu.ar/bitstream/handle/10915/56579/Documento_completo.pdf-PDFA.pdf?sequence=1)

Solucion formal que que garantiza y facilita el desarrollo e implementacion de sistemas informaticos que implementan hilos para explotar las arquitecturas multicore.

<p align="center">
 
<img src="https://github.com/gslAgile/PPSDriverRdP/blob/master/RdP_ecu_generalizada/RdP_extendida.png" title="Ejemplo RdP Generalizada">
<div align="center"></div>
 
</p>

Para ampliar la capacidad semántica de las redes se incluyen eventos, guardas, distintos tipos de brazos (inhibidores, lectores, etc.) y semánticas temporales. **Esto amplía el dominio de los problemas, el tamaño y la complejidad.**

**La idea de RdP Generalizada: Es generalizar la ecuación de estado de la RdP, usando el concepto de inhibición de disparos, para obtener las distintas ampliaciones semánticas e implementarlas en un sistema combinacional.**

## Fundamentos y Notacion
* **Ecuacion de estado:  Mj+1 = Mj + I*d**

El estado nuevo que determinado a partir del estado actual mas el producto de la matriz de incidencia con el vector disparo de la transicion que se desea disparar.

* **Disparo de una transicion y Transicion sensibilizada -> E[] = {0,1}**

Para ejecutar de manera correcta el disparo la una transicion la **transicion debe estar sensibilizada.**
Es el vector de transiciones sensibilizadas cuyo dominio es 0 y 1, donde 0 nos indica que la transicion no esta sensibilizada y un uno que si esta sensibilizada.


## Extencion de la ecuacion de estado

* **Arcos (o brazos) inhibidores, lectores y reset**

Nuevas conexiones que se dan entra cada par individual de plaza con transicion (pi, tj). 

<p align="center">
 
<img src="https://github.com/gslAgile/PPSDriverRdP/blob/master/RdP_ecu_generalizada/Tipos_de_arcos.png" title="Tipos de arcos/brazos">
<div align="center"></div>
 
</p>

* **Guardas g(ti) - variable logica (valores true, false ?)**

Si **g(ti)** es una guarda de ti (variable logica asociada a ti), entonces para que dispare la transicion ti debe estar sensibilizada y el valor de la guarda debe ser verdadero.

* **Eventos Ci - contador**

Los eventos asociados a la transicion **ti** se almacenan en una cola **Ci**. La cola **Ci** es un contador que se incrementa cuando llega un evento (**por ejemplo?**) y se decrementa cuando la transicion asociada se dispara. Para que la transicion **ti** dispare se requiere que este sensibilizada y que en la cola **Ci** exista al menos un evento.

* **Tiempo**

Limitaciones de tiempo sobre los disparos, como un intervalo de tiempo asociado a cada trnasicion.
Los instantes de tiempo inicial y final [**alfa**i, **beta**i] en que base de tiempo se miden ?

* **Politica de seleccion de disparos**

Responde la pregunta ¿De todas las transiciones sensibilizadas, cual debo disparar primero?

 - **Selecion aleatoria** da como consecuencia un resultado de un **sistema no deteministico.**

 - Pero buscamos **sistemas deterministicos** para lo cual utilizaremos prioridades, probabilidades, arcos (o brazos), etc.

* **Conclusion de caracteristicas de extencion**

  - Aumenta la capacidad de expresion de la RdP.
  - Se permiten modelar prioridas en las RdP.
  - Se permite comunicar la RdP con el medio haciendolas **RdP no autonomas.(?)**

## Ecuacion de estado extendida

* **Consideraciones para la generalizacion de la ecuacion**

Para que una transicion este sensibilizada ahora se tiene que cumplir que
  - Si tiene **arco inhibidor** que la plaza no tenga tokens.
  - Si tiene **arco lector** que la plaza tenga uno o mas tokens.
  - Si tiene **guarda asociada** que el valor de la guarda sea verdadero.
  - Si tiene **evento asociado** que tenga uno o mas eventos en la cola asociada.
  - Si tiene **etiqueta con intervalo de tiempo** que el contador se encuentre en el intervalo.
 
* **Nueva ecuacion de estado**

Se dispara si se cumple la conjuncion logica de todas la condiciones enumeradas.
Se generan vector de valores binarios que indican las transiciones inhibidas (por el arco, guarda, evento, tiempo, reset) con un cero y un uno las que no.
Se generan entonces los vectores de las condiciones utilizadas y que pueden ser

  - **Vector de transiciones des-sensibilizadas por arco inhibidor B = H x Q**
  - **Vector de transiciones des-sensibilizadas por arco lector L = R x W**
  - **Vector de transiciones des-sensibilizadas por guarda G**
  - **Vector de transiciones des-sensibilizadas por evento V**
  - **Vector de transiciones des-sensibilizadas por tiempo Z = Tim(q, intervalos)**
  - **Vector de transiciones reset A**

* **Vector de sensibilizado extendido Ex**

El vector **Ex** se obtiene de la conjuncion logica de todos los vectores anteriores.
**Ex = E & B & L & V & G & Z**

**Nueva ecuacion de estado: Mj+1 = Mj + I x (d & Ex)#A**

**IMPORTACIA DE NUEVA ECUACION DE ESTADO:** es que es de simple implementacion como circuito combinacional en una FPGA.
<p align="center">
 
<img src="https://github.com/gslAgile/PPSDriverRdP/blob/master/RdP_ecu_generalizada/matrices_RdP_generalizada.png" title="Matrices de RdP generalizada">
<div align="center"></div>
 
</p>
