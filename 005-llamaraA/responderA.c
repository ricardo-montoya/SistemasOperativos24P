#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX 256
#define EQ(str1, str2) (strcmp((str1), (str2)) == 0)  // Macro para comparar dos cadenas de caracteres

// Tamaño de la memoria compartida
#define SHM_SIZE MAX

// Nombre del segmento de memoria compartida
const char *SHM_NAME = "/shm_comunicacion";

char mensaje[MAX];  // Array para leer los mensajes
char mensaje_prev[MAX];  // Array para leer los mensajes

void fin_de_transmision(int sig) {
    sprintf(mensaje, "corto\n");
    // Escribimos el mensaje de finalización en la memoria compartida
    char *shm = (char *) mmap(NULL, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_open(SHM_NAME, O_RDWR, 0666), 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    strncpy(shm, mensaje, MAX);
    munmap(shm, SHM_SIZE);
    printf("FIN DE TRANSMISIÓN.\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Forma de uso: %s usuario\n", argv[0]);
        exit(-1);
    }

    char *logname = getenv("LOGNAME");

    if (EQ(logname, argv[1])) {
        fprintf(stderr, "Comunicación con uno mismo no permitida\n");
        exit(0);
    }

    // Abrir el segmento de memoria compartida
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Mapeo de la memoria compartida
    char *shm = (char *) mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Crear un manejador de señales para SIGINT
    signal(SIGINT, fin_de_transmision);

    pid_t pid = fork();
    pid_t pid2 = fork();

    if (pid == 0) { // Proceso hijo: Recepción de mensajes
        do {
      sleep(1);
            fflush(stdout);
            if(!EQ(mensaje_prev, shm)){
              printf("=> %s", shm);
              strncpy(mensaje_prev, shm, MAX);
            }
        } while (!EQ(shm, "corto\n") && !EQ(mensaje_prev, "corto\n"));
    exit(0);
    }     

    if (pid2 == 0){
    do{
        fgets(mensaje, sizeof(mensaje), stdin);
        strncpy(shm, mensaje, MAX);
        } while (!EQ(mensaje, "corto\n") && !EQ(shm, "corto\n"));
    exit(0);
    }

    wait(NULL);

    // Cleanup
    printf("FIN DE TRANSMISIÓN.\n");
    munmap(shm, SHM_SIZE);
    close(shm_fd);
    shm_unlink(SHM_NAME);
    exit(0);
    return 0;
}

