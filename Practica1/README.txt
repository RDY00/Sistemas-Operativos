
Integrantes del Equipo:

1.Fernando Márquez Pérez.
2.Juan Antonio Jasso Oviedo.

Preguntas y Respuestas:
1.Pintos ocupa la función switch_threads para cambiar del procesador el hilo actual por otro, des-
cribe los pasos que se llevan a cabo en esta función. (La función se implementa en el archivo (src/threads/switch.S).

R: Para cambiar un hilo por otro basicamente se busca quitarle el CPU al hilo actual para darselo al nuevo. Para lograr lo anterior los apuntadores de registros importantes (ebp,esi,ebx,edi) de la pila o stack, pasan a la stack o pila del nuevo proceso y finalmente ya el CPU y los registros importantes están en la pila del nuevo proceso(que ahora ya es el proceso actual).


2.¿Para qué sirve el stack de un proceso?

R: Para guardar o almacenar las variables locales y los saltos que realizan las funciones.


3.¿Qué diferencia hay entre el thread idle y el thread main?

R: El hilo main inicializa muchos del resto de hilos para que un Kernel funciones, sin embargo el hilo Idle(ocioso o flojo) se usa para que cualquier evento como una presión de una tecla en un teclado o el movimiento del mouse sea cachado por él, es uno que aunque no es el main nunca se bloquea o duerme.

4.Si suponemos que solamente existe un thread en ejecución y dicho thread se bloquea (utilizando la
función thread_block). ¿De que forma se podrá despertar el thread si es el único en ejecución?

R: La forma más sencilla es usando el thread idle, el cual nunca se bloquea ni duerme, y haciendo que él mismo lleve la cuenta de ticks(unidad que usa el timer para manejar el "tiempo" de los threads) para posteriormente despertarnos.
