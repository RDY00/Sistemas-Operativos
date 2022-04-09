INTEGRANTES DEL EQUIPO:
- Fernando Márquez Pérez
- Juan Antonio Jasso Oviedo.

PREGUNTAS Y RESPUESTAS:

1. ¿Qué es un deadlock?

Se refiere al estado de espera o bloqueo de un conjunto de procesos (podrían ser todos) por un evento que jamás ocurrirá.


2. ¿La implementación de semáforos de pintos utiliza un while en lugar de un if, explica porque es necesario esto.?

Porque tenemos que considerar el siguiente caso: sem inicial con valor 1, A hace sema_down(sem) y el sem queda en 0, luego B hace sema_down(sem) y como no puede bajarlo se queda en la lista de waiters, A hace sema_up(sem) y desbloquea a B, después un hilo C de mayor prioridad que B llega, toma el control y hace sema_down(sem) primero, sem vuelve a quedar en 0 para este punto y cuando B toma el control no debería de poder bajar sem porque es 0.

Si sólo tenemos un if, el hilo sólo verifica una vez que el semáforo sea 0 y en el caso que consideramos arriba se debe de verificar dos veces, es por ello que se hace un while. Básicamente se usa while porque debemos de considerar casos como el mencionado donde lleguen otros procesos a bajar el semáforo antes, en cuyo caso el hio debe de volver a insertarse en la lista de waiters.


3. ¿Por qué es mejor utilizar primitivas de sincronización en lugar de apagar y encender la interrupciones directamente?

Por un lado, porque apagar interrupciones sólo funciona en sistemas de un solo procesador porque el sistemas multiprocesadores sólo apagarías las de un procesador y los demás seguirían funcionando como si nada. Por otro lado, apagar las interrupciones por mucho tiempo no sólo evita la concurrencia por mucho tiempo, sino que también hace que el sistema no pueda atenden interrupciones importantes, como las de Entrada/Salida. Las primitivas de IPC sí permiten que se sigan atendiendo interrupciones porque, aunque apagen interrupciones como los semáforos de pintos, lo hacen por poco tiempo.


4.En pintos un lock es un semaphore inicializado en 1 ¿Por qué no usar directamente un semaphore? 

Por dos razones principales: 1) Los locks tienen la característica de que sólo los holders pueden liberarlo, en los semáforos cualquiera que haga sema_up libera el semáforo, aquí sólo quien tiene el lock puede hacer lock_release. 2) Los demás procesos saben quién tiene el lock y pueden reaccionar a ello (importante, por ejemplo, para la donación), con los semáforos esto no se sabe.

