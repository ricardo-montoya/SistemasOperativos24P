#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <utmp.h>      // Añadido para getutent y struct utmp

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
    int tty;
    char terminal[MAX], *logname;
    struct utmp *utmp;

    if (argc != 2) {
        fprintf(stderr, "Forma de uso: %s usuario\n", argv[0]);
        exit(-1);
    }

    logname = getenv("LOGNAME");

    if (EQ(logname, argv[1])) {
        fprintf(stderr, "Comunicación con uno mismo no permitida\n");
        exit(0);
    }

    // Consultamos si el usuario ha iniciado una sesión
    while ((utmp = getutent()) != NULL && strncmp(utmp->ut_user, argv[1], sizeof(utmp->ut_user)) != 0);

    if (utmp == NULL) {
        printf("EL USUARIO %s NO HA INICIADO SESIÓN.\n", argv[1]);
        exit(0);
    }

    // Crear o abrir el segmento de memoria compartida
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    ftruncate(shm_fd, SHM_SIZE);

    // Mapeo de la memoria compartida
    char *shm = (char *) mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    printf("Esperando respuesta...\n");

    // Armamos el manejador de la señal SIGINT
    signal(SIGINT, fin_de_transmision);

    pid_t pid = fork();
    pid_t pid2 = fork();
    if (pid == 0) { // Proceso hijo: Envío de mensajes
        do {
            fgets(mensaje, sizeof(mensaje), stdin);
            strncpy(shm, mensaje, MAX);
        } while (!EQ(shm, "corto\n") && !EQ(mensaje_prev, "corto\n"));
    exit(0);
    }

    if (pid2 == 0){
    do{
        sleep(1);
            fflush(stdout);
            if(!EQ(mensaje_prev, shm) ){
              printf("=> %s",shm);
            strncpy(mensaje_prev, shm, MAX);
            }
      } while (!EQ(shm, "corto\n") && !EQ(mensaje_prev, "corto\n"));
    exit(0);
  }

    wait(NULL);

    // Cleanup
    printf("SE ACABO LA TRANSMISIÓN.\n");
    munmap(shm, SHM_SIZE);
    close(shm_fd);
    shm_unlink(SHM_NAME);
    exit(0);
    return 0;
}

