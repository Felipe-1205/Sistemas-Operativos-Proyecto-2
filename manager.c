#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "estructuras.h"

int rows;
int cols = 3, hilos = 0;
int **matrix;
grupo *vector = NULL;  // Variable global para el vector de grupos
int cantidad = 0;

// Función del hilo secundario
void *threadFunc(void *arg) {
  int pid = *(int *)arg;  // Convertir el argumento a un entero

  // Ciclo infinito para esperar comandos
  do {
    char nombre[100];
    sprintf(nombre, "escribir-%d", pid);

    // Crear el pipe con nombre si no existe
    unlink(nombre);
    mkfifo(nombre, 0666);

    // Abrir el pipe con nombre para lectura
    int pipe_fd = open(nombre, O_RDONLY);
    if (pipe_fd == -1) {
      perror("Error al abrir el pipe con nombre");
      exit(1);
    }

    int codigo;
    ssize_t bytes_read = read(pipe_fd, &codigo, sizeof(codigo));
    if (bytes_read == -1) {
      perror("Error al leer desde el pipe");
      exit(1);
    }

    // Realizar acciones dependiendo del código recibido
    if (codigo == 1) {  // Comando "list"
      close(pipe_fd);
      pipe_fd = open(nombre, O_WRONLY);
      if (pipe_fd == -1) {
        perror("Error al abrir el pipe con nombre");
        exit(1);
      }

      int valor;
      for (int i = 0; i < rows; i++) {
        if (matrix[i][1] == 1) {
          valor = matrix[i][0];
          ssize_t bytes_written = write(pipe_fd, &valor, sizeof(valor));
          if (bytes_written == -1) {
            perror("Error al escribir en el pipe");
            exit(1);
          }
        }
      }

      valor = -1;
      ssize_t bytes_written = write(pipe_fd, &valor, sizeof(valor));
      if (bytes_written == -1) {
        perror("Error al escribir en el pipe");
        exit(1);
      }
    } else if (codigo == 2) {  // Comando "list grupo"

    } else if (codigo == 3) {  // Comando "salida"
      for (int i = 0; i < rows; i++) {
        if (matrix[i][2] == pid) {
          matrix[i][1] = 0;
          matrix[i][2] = 0;
        }
      }
      close(pipe_fd);
      pipe_fd = open(nombre, O_WRONLY);
      if (pipe_fd == -1) {
        perror("Error al abrir el pipe con nombre");
        exit(1);
      }
      int valor = -1;
      ssize_t bytes_written = write(pipe_fd, &valor, sizeof(valor));
      if (bytes_written == -1) {
        perror("Error al escribir en el pipe");
        exit(1);
      }
      unlink(nombre);
      pthread_exit(NULL);
    } else if (codigo == 4) {  // Comando "crea grupo"
      grupo grupoc;
      int cuantos = read(pipe_fd, &grupoc, sizeof(grupoc));
      int nuevaCapacidad = cantidad + 1;
      sprintf(grupoc.codigo, "G%d", nuevaCapacidad);
      vector = realloc(vector, nuevaCapacidad * sizeof(grupo));
      vector[cantidad] = grupoc;
      cantidad++;
      close(pipe_fd);
      pipe_fd = open(nombre, O_WRONLY);
      if (pipe_fd == -1) {
        perror("Error al abrir el pipe con nombre");
        exit(1);
      }
      ssize_t bytes_written =
          write(pipe_fd, &nuevaCapacidad, sizeof(nuevaCapacidad));
      if (bytes_written == -1) {
        perror("Error al escribir en el pipe");
        exit(1);
      }

    } else if (codigo == 5) {  // Comando "mensaje"

    } else if (codigo == 6) {  // Comando "mensaje de grupo"
    }
  } while (1);

  
}

// Función para crear la matriz
void createMatrix() {
  matrix = (int **)malloc(rows * sizeof(int *));
  for (int i = 0; i < rows; i++) {
    matrix[i] = (int *)malloc(cols * sizeof(int));
  }
}

int main(int argc, char **argv) {
  int nmx;
  char nombre[50];

  // Validación de parámetros de línea de comandos
  if (argc != 5) {
    fprintf(
        stderr,
        "Uso: \n%s Manager –n N -p pipeNom\no\n%s talker –i ID –p pipeNom\n",
        argv[0], argv[0]);
    exit(1);
  }

  // Obtener los valores de los parámetros de línea de comandos
  if (strcmp(argv[1], "-p") == 0) {
    if (strcmp(argv[3], "-p") == 0) {
      fprintf(stderr, "No pueden haber dos -p.\n");
      exit(1);
    }
    strcpy(nombre, argv[2]);
    if (strcmp(argv[3], "-n") == 0) {
      for (int i = 0; argv[4][i] != '\0'; i++) {
        // Verificar que el valor ingresado sea un número
        if (!isdigit(argv[4][i])) {
          printf("El valor después de -n debe ser un número\n");
          return 1;
        }
      }
      nmx = atoi(argv[4]);
    } else {
      fprintf(stderr, "Hace falta un -n.\n");
      exit(1);
    }
  } else if (strcmp(argv[1], "-p") != 0) {
    if (strcmp(argv[3], "-p") != 0) {
      fprintf(stderr, "Hace falta un -p.\n");
      exit(1);
    }
    strcpy(nombre, argv[4]);
    if (strcmp(argv[1], "-n") == 0) {
      for (int i = 0; argv[2][i] != '\0'; i++) {
        // Verificar que el valor ingresado sea un número
        if (!isdigit(argv[2][i])) {
          printf("El valor después de -n debe ser un número\n");
          return 1;
        }
      }
      nmx = atoi(argv[2]);
    } else {
      fprintf(stderr, "Hace falta un -n.\n");
      exit(1);
    }
  }

  // Crear matriz para en línea
  rows = nmx;
  createMatrix();

  for (int i = 0; i < rows; i++) {
    matrix[i][0] = i + 1;
    matrix[i][1] = 0;
  }

  do {
    // Crear el pipe con nombre si no existe
    unlink(nombre);
    mkfifo(nombre, 0666);

    // Abrir el pipe con nombre para lectura
    int pipe_fd = open(nombre, O_RDONLY);
    if (pipe_fd == -1) {
      perror("Error al abrir el pipe con nombre");
      exit(1);
    }

    // Leer el ID desde el pipe
    int id;
    ssize_t bytes_read = read(pipe_fd, &id, sizeof(id));
    if (bytes_read == -1) {
      perror("Error al leer desde el pipe");
      exit(1);
    }

    // Validar si el valor es mayor al número máximo y enviar una respuesta
    close(pipe_fd);
    pipe_fd = open(nombre, O_WRONLY);
    if (pipe_fd == -1) {
      perror("Error al abrir el pipe con nombre");
      exit(1);
    }

    if (nmx < id || id == 0) {
      int a = -1;
      ssize_t bytes_written = write(pipe_fd, &a, sizeof(a));
      if (bytes_written == -1) {
        perror("Error al escribir en el pipe");
        exit(1);
      }
    } else {
      for (int i = 0; i < rows; i++) {
        if (matrix[i][0] == id) {
          if (matrix[i][1] == 1) {
            int a = -2;
            ssize_t bytes_written = write(pipe_fd, &a, sizeof(a));
            if (bytes_written == -1) {
              perror("Error al escribir en el pipe");
              exit(1);
            }
          } else if (matrix[i][1] == 0) {
            int a = 1;
            ssize_t bytes_written = write(pipe_fd, &a, sizeof(a));
            if (bytes_written == -1) {
              perror("Error al escribir en el pipe");
              exit(1);
            }
            matrix[i][1] = a;

            // Abrir el pipe con nombre para lectura
            close(pipe_fd);
            pipe_fd = open(nombre, O_RDONLY);
            if (pipe_fd == -1) {
              perror("Error al abrir el pipe con nombre");
              exit(1);
            }

            // Leer el PID desde el pipe
            int pid;
            bytes_read = read(pipe_fd, &pid, sizeof(pid));
            if (bytes_read == -1) {
              perror("Error al leer desde el pipe");
              exit(1);
            }
            printf("Manager: PID recibido: %d\n", pid);

            for (int i = 0; i < rows; i++) {
              if (matrix[i][0] == id) {
                matrix[i][2] = pid;
              }
            }

            hilos++;
            pthread_t thread[nmx];
            int ret;
            ret = pthread_create(&thread[hilos], NULL, threadFunc, &pid);
            if (ret != 0) {
              perror("Error al crear el hilo");
              return 1;
            }

            // Desvincular el hilo
            ret = pthread_detach(thread[hilos]);
            if (ret != 0) {
              perror("Error al desvincular el hilo");
              return 1;
            }

            // Cerrar el descriptor del pipe
            close(pipe_fd);
          }
        }
      }
    }

  } while (1);

  return 0;
}
