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

void *threadFunc(void *arg) {
  do {
    char nombre[100];
    sprintf(nombre, "escuchar-%d", (int)getpid());
    // Crear el pipe con nombre si no existe
    unlink(nombre);
    mkfifo(nombre, 0666);
    // Abrir el pipe con nombre para lectura
    int pipe_fd = open(nombre, O_RDONLY);
    if (pipe_fd == -1) {
      perror("Error al abrir el pipe con nombre");
      exit(1);
    }
    int id;
    ssize_t bytes_read = read(pipe_fd, &id, sizeof(id));
    if (bytes_read == -1) {
      perror("Error al leer desde el pipe");
      exit(1);
    }
  } while (1);
}

int main(int argc, char **argv) {
  int id, valido;
  char nombre[50];
  // validacion de parametros
  if (argc != 5) {  // COMPROBAMOS QUE SE META LA CANTIDAD CORRECTA DE DATOS
    fprintf(
        stderr,
        "Uso: \n%s Manager –n N -p pipeNom\no\n%s talker –i ID –p pipeNom\n",
        argv[0], argv[0]);
    exit(1);
  }

  if (strcmp(argv[1], "-p") == 0) {
    if (strcmp(argv[3], "-p") == 0) {
      fprintf(stderr, "No pueden haber dos -p.\n");
      exit(1);
    }
    strcpy(nombre, argv[2]);
    if (strcmp(argv[3], "-i") == 0) {
      for (int i = 0; argv[4][i] != '\0';
           i++) {  // verifica que el valor ingresado sea un numero
        if (!isdigit(argv[4][i])) {
          printf("El valor despues de -i debe ser un número\n");
          return 1;
        }
      }
      id = atoi(argv[4]);
    } else {
      fprintf(stderr, "Hace falta un -i.\n");
      exit(1);
    }
  } else if (strcmp(argv[1], "-p") != 0) {
    if (strcmp(argv[3], "-p") != 0) {
      fprintf(stderr, "Hace falta un -p.\n");
      exit(1);
    }
    strcpy(nombre, argv[4]);
    if (strcmp(argv[1], "-i") == 0) {
      for (int i = 0; argv[2][i] != '\0';
           i++) {  // verifica que el valor ingresado sea un numero
        if (!isdigit(argv[2][i])) {
          printf("El valor despues de -i debe ser un número\n");
          return 1;
        }
      }
      id = atoi(argv[2]);
    } else {
      fprintf(stderr, "Hace falta un -i.\n");
      exit(1);
    }
  }
  // Abrir el pipe con nombre para escritura
  int pipe_fd = open(nombre, O_WRONLY);
  if (pipe_fd == -1) {
    perror("Error al abrir el pipe con nombre");
    exit(1);
  }
  // Escribir el ID en el pipe
  ssize_t bytes_written = write(pipe_fd, &id, sizeof(id));
  if (bytes_written == -1) {
    perror("Error al escribir en el pipe");
    exit(1);
  }

  // abrir pipe en lectura y esperar que diga si es valido o no
  close(pipe_fd);
  pipe_fd = open(nombre, O_RDONLY);
  if (pipe_fd == -1) {
    perror("Error al abrir el pipe con nombre");
    exit(1);
  }
  ssize_t bytes_read = read(pipe_fd, &valido, sizeof(valido));
  if (bytes_read == -1) {
    perror("Error al leer desde el pipe");
    exit(1);
  }
  if (valido == -2) {
    printf("el id ya inicio sesion\n");
    exit(1);
  } else if (valido == -1) {
    printf("el id esta fuera de rango\n");
    exit(1);
  }
  // Abrir el pipe con nombre para escritura
  close(pipe_fd);
  pipe_fd = open(nombre, O_WRONLY);
  if (pipe_fd == -1) {
    perror("Error al abrir el pipe con nombre");
    exit(1);
  }
  // Escribir el PID en el pipe
  int pid = getpid();
  bytes_written = write(pipe_fd, &pid, sizeof(pid));
  if (bytes_written == -1) {
    perror("Error al escribir en el pipe");
    exit(1);
  }
  printf("Talker: PID enviado: %d\n", pid);
  // creamos un hilo que va a estar escuchando un piepe del manager
  pthread_t thread;
  int ret;
  ret = pthread_create(&thread, NULL, threadFunc, NULL);
  if (ret != 0) {
    perror("Error al crear el hilo");
    return 1;
  }
  ret = pthread_detach(thread);
  if (ret != 0) {
    perror("Error al desvincular el hilo");
    return 1;
  }
  // empieza el menu
  do {
    char entrada[200];
    char *arguments[100];
    int contador = 0;
    printf("===== MENÚ =====\n");
    printf("List\n");
    printf("List GID\n");
    printf("Group ID1, ID2, ID3…IDN\n");
    printf("Sent msg IDi\n");
    printf("Sent msg GroupID\n");
    printf("Salir\n");
    printf("Ingrese una opción: ");
    fgets(entrada, sizeof(entrada), stdin);

    char *token = strtok(entrada, " \n");
    while (token != NULL) {
      arguments[contador] =
          malloc(strlen(token) + 1);  // Asignar memoria para cada argumento
      strcpy(arguments[contador], token);
      contador++;
      token = strtok(NULL, " \n");
    }
    char escritura[100];
    sprintf(escritura, "escribir-%d", (int)getpid());
    // Abrir el pipe con nombre para escritura
    int pipe_escr = open(escritura, O_WRONLY);
    if (pipe_escr == -1) {
      perror("Error al abrir el pipe con nombre");
      exit(1);
    }
    printf("%s\n", arguments[0]);
    if (strcmp(arguments[0], "List") == 0) {
      if (arguments[2] == NULL) {
        int codigo = 1;
        bytes_written = write(pipe_escr, &codigo, sizeof(codigo));
        if (bytes_written == -1) {
          perror("Error al escribir en el pipe");
          exit(1);
        }
        int usuario = 0;
        do {
          // Abrir el pipe con nombre para lectura
          close(pipe_escr);
          pipe_escr = open(escritura, O_RDONLY);
          if (pipe_escr == -1) {
            perror("Error al abrir el pipe con nombre");
            exit(1);
          }
          ssize_t bytes_read = read(pipe_escr, &usuario, sizeof(usuario));
          if (bytes_read == -1) {
            perror("Error al leer desde el pipe");
            exit(1);
          }
          if (usuario == -1) {
            break;
          }
          printf("usuario de ID %d Esta conectado\n", usuario);
        } while (1);

      } else {
        int codigo = 2;
        bytes_written = write(pipe_escr, &codigo, sizeof(codigo));
        if (bytes_written == -1) {
          perror("Error al escribir en el pipe");
          exit(1);
        }
      }
    } else if (strcmp(arguments[0], "Group") == 0) {
      printf("1\n");
      int codigo = 4;
      bytes_written = write(pipe_escr, &codigo, sizeof(codigo));
      if (bytes_written == -1) {
        perror("Error al escribir en el pipe");
        exit(1);
      }
      printf("2\n");

    } else if (strcmp(arguments[0], "Sent") == 0) {
      int codigo = 0;
      int tamaño = 0;
      while (arguments[tamaño] != NULL) {
        tamaño++;
      }
      for (int i = 0; arguments[tamaño - 1][i] != '\0';
           i++) {  // verifica que el valor ingresado sea un numero
        if (!isdigit(arguments[tamaño - 1][i])) {
          codigo = 6;
        }
      }
      if (codigo == 0) {
        codigo = 5;
      }
      bytes_written = write(pipe_escr, &codigo, sizeof(codigo));
      if (bytes_written == -1) {
        perror("Error al escribir en el pipe");
        exit(1);
      }

    } else if (strcmp(arguments[0], "Salir") == 0) {
      int codigo = 3;
      bytes_written = write(pipe_escr, &codigo, sizeof(codigo));
      if (bytes_written == -1) {
        perror("Error al escribir en el pipe");
        exit(1);
      }

      int valor;
      close(pipe_escr);
      pipe_escr = open(escritura, O_RDONLY);
      if (pipe_escr == -1) {
        perror("Error al abrir el pipe con nombre");
        exit(1);
      }
      ssize_t bytes_read = read(pipe_escr, &valor, sizeof(valor));
      if (bytes_read == -1) {
        perror("Error al leer desde el pipe");
        exit(1);
      }
      if(valor==-1){
        unlink(escritura);
        exit(1);
      }
    }
  } while (1);
  // Cerrar el descriptor del pipe
  close(pipe_fd);

  return 0;
}