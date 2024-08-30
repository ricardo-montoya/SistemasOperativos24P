#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define MAX 256

// Macro para comparar dos cadenas de caracteres.
#define EQ(str1, str2) (strcmp((str1), (str2)) == 0)

// Descriptores de fichero de las tuberías mediante las que nos vamos a comunicar.
int fifo_12, fifo_21;
char nombre_fifo_12[MAX], nombre_fifo_21[MAX];

// Array para leer los mensajes.
char mensaje[MAX];

// Manejador de la señal SIGINT.
void fin_de_transmision(int sig);

int main(int argc, char *argv[]) {
    char *logname, *getenv();
    int iniciar_comunicacion = 0;

    // Análisis de los argumentos de la línea de órdenes.
    if (argc != 2) {
        fprintf(stderr, "Forma de uso: %s usuario\n", argv[0]);
        exit(-1);
    }

    // Lectura del nombre del usuario.
    logname = getenv("LOGNAME");

    // Comprobación para que un usuario no se responda a sí mismo.
    if (EQ(logname, argv[1])) {
        fprintf(stderr, "Comunicación con uno mismo no permitida\n");
        exit(0);
    }

    // Formación del nombre de las tuberías de comunicación.
    sprintf(nombre_fifo_12, "/tmp/%s_%s", argv[1], logname);
    sprintf(nombre_fifo_21, "/tmp/%s_%s", logname, argv[1]);

    // Crear las tuberías si no existen.
    if (mkfifo(nombre_fifo_12, 0666) == -1 && errno != EEXIST) {
        perror("Error al crear la tubería fifo_12");
        exit(-1);
    }
    if (mkfifo(nombre_fifo_21, 0666) == -1 && errno != EEXIST) {
        perror("Error al crear la tubería fifo_21");
        exit(-1);
    }

    printf("Tuberías creadas.\n");

    // Apertura de las tuberías en modo lectura/escritura para evitar deadlock.
    fifo_12 = open(nombre_fifo_12, O_RDWR);
    if (fifo_12 == -1) {
        perror("Error al abrir la tubería fifo_12");
        exit(-1);
    }
    printf("Tubería %s abierta.\n", nombre_fifo_12);

    fifo_21 = open(nombre_fifo_21, O_RDWR);
    if (fifo_21 == -1) {
        perror("Error al abrir la tubería fifo_21");
        close(fifo_12);
        exit(-1);
    }
    printf("Tubería %s abierta.\n", nombre_fifo_21);

    // Armamos el manejador de la señal SIGINT.
    signal(SIGINT, fin_de_transmision);

    // Preguntar si este usuario debería iniciar la comunicación
    printf("¿Quieres iniciar la comunicación? (s/n): ");
    char respuesta;
    scanf(" %c", &respuesta);

    if (respuesta == 's' || respuesta == 'S') {
        iniciar_comunicacion = 1;
    }

    if (iniciar_comunicacion) {
        do {
            printf("<== Ingrese mensaje: ");
            fgets(mensaje, sizeof(mensaje), stdin);
            write(fifo_21, mensaje, strlen(mensaje) + 1);
        } while (!EQ(mensaje, "cambio\n") && !EQ(mensaje, "corto\n"));
    }

    // Bucle de recepción de mensajes.
    while (1) {
        printf("==> Esperando mensaje...\n");
        ssize_t bytes_read = read(fifo_12, mensaje, MAX);

        if (bytes_read > 0) {
            printf("Mensaje recibido: %s", mensaje);

            if (EQ(mensaje, "cambio\n")) {
                do {
                    printf("<== Ingrese mensaje: ");
                    fgets(mensaje, sizeof(mensaje), stdin);
                    write(fifo_21, mensaje, strlen(mensaje) + 1);
                } while (!EQ(mensaje, "cambio\n") && !EQ(mensaje, "corto\n"));
            }
        } else if (bytes_read == -1) {
            perror("Error al leer de la tubería fifo_12");
            break;
        }
    }

    printf("FIN DE TRANSMISIÓN.\n");
    close(fifo_12);
    close(fifo_21);
    exit(0);
}

// fin_de_transmision: rutina de tratamiento de la señal SIGINT.
// Al pulsar Ctrl+C entramos en esta rutina, que se encarga de enviar el mensaje "corto\n" al usuario con el que estamos hablando.
void fin_de_transmision(int sig) {
    sprintf(mensaje, "corto\n");
    write(fifo_21, mensaje, strlen(mensaje) + 1);
    printf("FIN DE TRANSMISIÓN.\n");
    close(fifo_12);
    close(fifo_21);
    exit(0);
}

