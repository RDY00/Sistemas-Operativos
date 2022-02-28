INTEGRANTES DEL EQUIPO:
* Fernando Márquez Pérez
* Juan Antonio Jasso Oviedo

PREGUNTAS Y RESPUESTAS:

1. Pintos ocupa la función switch_threads para cambiar del procesador el hilo actual por otro, describe los pasos que se llevan a cabo en esta función. (La función se implementa en el archivo (src/threads/switch.S).

R: Intercambia los contextos del hilo actual con el siguiente:
* Primero se guardan en el stack los registros importantes del contexto del hilo actual (ebx, ebp, esi, edi) para que pueda reanudar su ejecución donde se quedó, cuando se "despierte".
* Después de guarda el stack pointer del hilo actual (almacenado en su bloque de control) y se recupera el del nuevo hilo. Así mismo, de su stack se recupera el estado de sus registros para que pueda reaundar su ejecución donde se había quedado (o a empezar si era nuevo).
* Por último se inicia el nuevo hilo. Se pide que ambos estén ejecutando la función para poder guardar registros al stack, intercambiar estos stacks y recuperar la nueva información de ellos.

2. ¿Para qué sirve el stack de un proceso?

R: Para guardar o almacenar información local del proceso, como son: las variables locales, las llamadas/saltos de la funciones y el estado del mismo proceso.

3.¿Qué diferencia hay entre el thread idle y el thread main?

R: El hilo main es el primer hilo que se crea y es quien inicializa a otros hilos para que el Kernel funcione. El hilo idle (ocioso o flojo), por otro lado, es el hilo encargado de manejar ciertos eventos, como una presión de una tecla en un teclado o el movimiento del mouse, este hilo es particular en el sentido de que nunca se duerme pero nunca esta "totalmente activo" (está en un estado de halt) y siempre queda como el hilo en ejecución cuando no hay más hilos o cuando todos están bloqueados/dormidos.

4.Si suponemos que solamente existe un thread en ejecución y dicho thread se bloquea (utilizando la función thread_block). ¿De que forma se podrá despertar el thread si es el único en ejecución?

R: Como se menciono antes, quien entraría a ejecución en ese casi sería el thread idle y él se encargaría de despertar/desbloquear al hilo bloqueado.
