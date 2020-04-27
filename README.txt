SISTEMI OPERATIVI - PROGETTO DI LABORATORIO 2019/2020 - BIKAYA

PHASE 1.5 GRUPPO lso20az20

Il programma vuole gestire la corretta funzionalità del nucleo del Sistema Operativo che deve gestire:
- inizializzazione del sistema
- scheduling dei processi
- gestione delle syscall (solo la sys 3)
- gestione degli interrupt (implementate in maniera soltanto parziale)

1.Esecuzione
Per compilare e linkare il programma bisogna eseguire il comando 'make -f' seguito dal target 'umpsmake' o 'uarmmake' a seconda della macchina che si vuole utilizzare.

Dopo aver eseguito questo comando vengono creati i file oggetto delle librerie necessarie e il kernel, da utilizzare per il corretto funzionamento della macchina selezionata.

Nel caso si voglia utilizzare il software uMPS2 bisogna aprirlo e creare una nuova configurazione per la macchina virtuale, inserire i file 'kernel.core.umps'e 'kernel.stab.umps' dove necessario, accendere la macchina ed eseguire.

Nel caso si voglia utilizzare il software uARM bisogna aprirlo, inserire il file 'kernel', accendere la macchina ed eseguire.

Per vedere il corretto funzionamento, aprire il 'Terminal 0'.

Per cancellare i file oggetto e ricompilare per un'altra macchina da quella precedentemente utilizzata, eseguire il comando 'make clean'.

2.Membri del gruppo
Martina Sosto
Sara Vorabbi
Andrea Vicenzi
