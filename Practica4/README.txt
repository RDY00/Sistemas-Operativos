INTEGRANTES DEL EQUIPO:
*Fernando Márquez Pérez
*Juan Antonio Jasso Oviedo.

PREGUNTAS Y RESPUESTAS:

1.¿Qué es un deadlock?
Es cuando todos los procesos de nuestro sistema están en un estado de espera o bloqueo y para que se termine ese estado debe ocurrir un evento que jamas ocurrirá.

2.¿La implementación de semáforos de pintos utiliza un while en lugar de un if, explica porque es
necesario esto.?
Porque si solo fuera un if, por ejemplo en sema_down, solo verificariamos una vez que el semaforo no esté ocupado y posteriormente continuariamos con la ejecución del resto del código, aunque no sea correcto hacerlo. Entonces con un while, aseguramos que se duerma el hilo hasta que las condiciones sean optimas para seguir con el proceso de sema_down u up.

3.¿Por qué es mejor utilizar primitivas de sincronización en lugar de apagar y encender la interrup-
ciones directamente?
Porque las interrupts no podrían adquirir los locks. Por lo que sería redundante manejar las interrupciones manualmente que simplemente implementar dichas primitivas.

4.En pintos un lock es un semaphore inicializado en 1 ¿Por qué no usar directamente un semaphore?
Porque un semafore podría tener que varios procesos entren o tengan acceso a la estructura compartida sumultaneamente. Mientras que el lock se asegura de que sea un solo proceso el que tenga ese acceso a la estructura.

/*Donating the priority of the current to the lock holder.*/

lock->holder->donated = true;
lock->holder->old_priority = lock->holder->priority;
t->old_priority = t-> priority;
lock->holder->priority = t->priority;
