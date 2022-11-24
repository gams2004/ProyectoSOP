/*
Nombre del archivo: usuario.c // Autores: Santiago Molina, Gabriel Martín // Finalización 13/11/2022
El objetivo de este archivo es crear cliente que defina una serie de solicitudes y las envíe a un gestor para obtener una respuesta.
Solicitudes enviadas:
1: registro
2: inicio sesión
3: mandar un tweet
4: follow
5: unfollow
6: desconexión
7: refrescar tweets

Componentes:
void mimanejador();
void mimanejadorhilo();
void funcionhilo(char*);
int main(int, char**);

Compilación del programa:
gcc -o usuario usuario.c -pthread
Ejecución del programa:
./usuario -i 1 -p pipesoli
*/
#include "usuario.h"


void mimanejador() { //Esta función emplea la variable global "pausa". Autor: Santiago Molina; sin parámetros; no hay retorno; iguala la variable global pause a false.
	pausa = false; 
}
void mimanejadorhilo() {//Esta función emplea la variable global "lock". Autor: Santiago Molina; sin parámetros; no hay retorno; se encarga de desbloquear el mutex sobre la variable global "lock".      
  pthread_mutex_unlock(&lock); 
}

void funcionhilo(char *pipe) {//Esta función emplea las variables globales "lock" e "impmenu". Autores: Santiago Molina, Gabriel Martín; Parámetros: apuntador a un arreglo de caracteres; no hay retorno; se encarga de imprimir por pantalla los tweets que le llegan del gestor correspondientes a aquellos enviados por los usuarios a los que se sigue.
  int num, cuantos, fd, tames;
  int fdhilo = open(pipe, O_RDONLY | O_NONBLOCK);
  while (true) {
    pthread_mutex_lock(&lock);
    cuantos = read(fdhilo, &num, sizeof(int));
    printf("\nNúmero de Tweets: %d.\n", num);
    if (cuantos == -1) {
      perror("proceso hilo #1.1");
      exit(1);
    }
    if (num == -1)
      printf("La opcion no se puede usar en este momento.\n");
    else if (num == 0)
      printf("No hay Tweets en este momento.\n");
    else {
      //Impresion de tweets
      for (int i = 0; i < num; i++) {
        cuantos = read(fdhilo, &tames, sizeof(int));
        char respuesta[tames];
        cuantos = read(fdhilo, &respuesta, tames);
        for (int j = 0; j < tames - 1; j++) {
          printf("%c", respuesta[j]);
        }
        printf("\n");
      }
    }
    impmenu = true;
  }
}

int main(int argc, char **argv) {//Esta función emplea las variables globales "lock", "impmenu" y "pausa". Autores: Santiago Molina, Gabriel Martín; Parámetros: numero de parámetros al ejecutar el programa por línea de comandos, arreglo de parámetros al ejecutar el programa por línea de comandos; retorna un entero al finalizar la ejecución del programa; se encarga de mostrar una pequeña interfaz por línea de comandos para definir las solicitudes que desea realizar un usuario y enviarselas al gestor.
  int fd, fd1, op;
  char respuesta;
  mode_t fifo_mode = S_IRUSR | S_IWUSR;
  int entradaUser;
  int cuantos;
  user user1;
  char pipe[20] = "miPipe";
  char *nomPipe;
  envi envi;
  bool banderas[2] = {false, false};
  int ID;
  char mipid[13];
  pthread_t thread1;
	
  // Comprobación de parámetros.
  if (argc > 5) {
    perror("Se exedió el numero de parámetros, debe ser: \n -i [ID del cliente "
           "a conectar] -p [nombre del pipe del cliente]");
    exit(1);
  } else if (argc < 5) {
    perror("Numero de parámetros incompleto, debe ser: \n -i [ID del cliente a "
           "conectar] -p [nombre del pipe del cliente]");
    exit(1);
  }

  //Almacenamiento de parametros.
  for (int i = 1; i < 5; i += 2) {
    if (strcmp("-i", argv[i]) == 0) {
      ID = strtol(argv[i + 1], NULL, 10);
      if (banderas[0] == true) {
        perror("Parametro -i duplicado.");
        exit(1);
      }
      if (ID > 80 || ID < 1) {
        perror("Parametro -i no valido, debe ser un número entre 1 y 80.");
        exit(1);
      }
      banderas[0] = true;
    } else if (strcmp("-p", argv[i]) == 0) {
      nomPipe = argv[i + 1];
      if (banderas[1] == true) {
        perror("Parametro -p duplicado.");
        exit(1);
      }
      banderas[1] = true;
    }
  }

  user1.codigo = ID;
  sprintf(mipid, "%d", getpid());
  strcat(pipe, mipid);
  strcat(pipe, "a");
  unlink(pipe);

  //Creación del pipe que va a emplear el usuario para recibir información del gestor.
  if (mkfifo(pipe, fifo_mode) == -1) {
    perror("Error creando el pipe rojo.");
    exit(1);
  }
  pthread_mutex_lock(&lock);
  pthread_create(&thread1, NULL, (void *)funcionhilo, (void *)pipe);
  fd = open(pipe, O_RDONLY | O_NONBLOCK); // pipe rojo.
  int creado = 0;
  do {
    fd1 = open(nomPipe, O_WRONLY); // pipe azul.
    if (fd1 == -1) {
      perror("No hay pipe del servidor.");
      printf(" Se volvera a intentar después.\n");
      sleep(4);
    } else
      creado = 1;
  } while (creado == 0);

  //Menú de usuario antes de conetarse.
  while (true) {
    printf("\n--Bienvenido a Mini-Twitter--");
    printf("\n1. Registrarse");
    printf("\n2. Iniciar Sesión");
    printf("\n3. Desconectarse\n");
    printf("Digite su opción: ");
    int menu1 = scanf("%d", &op);

    if (op == 2) { // El usuario va a iniciar sesión.
      envi.accion = 2;
      envi.user = ID;
      envi.pid = getpid();
      memcpy(envi.milinea, pipe, strlen(pipe) + 1);
      signal(SIGUSR1, mimanejador);
      int ret1 = write(fd1, &envi, sizeof(envi));
      while (pausa == true)
        pause();
      pausa = true;
      cuantos = read(fd, &respuesta, sizeof(char));
      if (cuantos == -1) {
        perror("proceso cliente #2.1");
        exit(1);
      }
      if (respuesta == 'N') {
        printf("\nNo se pudo iniciar sesión.\n");
      } else if (respuesta == 'S') {
        printf("\nSe pudo iniciar sesión.\n");
        pthread_mutex_unlock(&lock);
        do {
          signal(SIGUSR2, mimanejadorhilo);
          while (impmenu == false) {
            sleep(1);
          };

          //Menu usuario tras el inicio de sesión.
          printf("\n--Menu Interno--");
          printf("\n1. Mandar tweet");
          printf("\n2. Seguir a alguien");
          printf("\n3. Dejar de seguir a alguien");
          printf("\n4. Desconexión");
          printf("\n5. Refrescar Tweets\n");
          printf("Digite su opción: ");
          impmenu = false;
          int menu2 = scanf("%d", &op);
          if (op == 1) { // El usuario va a mandar un tweet.
            printf("\nEscriba su Tweet:\n");
            char *retu = fgets(
                envi.tweet, 199,
                stdin); // Este "absorbe" un salto de línea del anterior scan.
            retu = fgets(envi.tweet, 199, stdin);
            if (envi.tweet[strlen(envi.tweet) - 1] == '\n') {
              envi.tweet[strlen(envi.tweet) - 1] = 0;
            }
            envi.accion = 3;
            envi.user = ID;
            //Envío del tweet por el pipe azul del gestor.
            int ret1 = write(fd1, &envi, sizeof(envi));
            printf("Su Tweet se envió correctamente.\n");
            impmenu = true;
          } else if (op == 2) { // El usuario va a seguir a alguien.
            int user;
            printf("Digite el ID del usuario a seguir:");
            int menu2 = scanf("%d", &user);
            envi.accion = 4;
            envi.user = ID;
            envi.userFollow = user;
            signal(SIGUSR1, mimanejador);
            //Se envía información del usuario a seguir por el pipe azul del gestor.
            int ret1 = write(fd1, &envi, sizeof(envi));
            while (pausa == true)
              pause();
            pausa = true;
            cuantos = read(fd, &respuesta, sizeof(char));
            if (cuantos == -1) {
              perror("proceso cliente #4.1");
              exit(1);
            }
            if (respuesta == 'N') {
              printf("\nNo se pudo seguir al usuario.\n");
            } else if (respuesta == 'S') {
              printf("\nSe pudo seguir al usuario.\n");
            }
            impmenu = true;
          } else if (op == 3) { // El usuario va a dejar de seguir a alguien.
            int user;
            printf("Digite el ID del usuario para dejar de seguir:");
            int menu2 = scanf("%d", &user);
            envi.accion = 5;
            envi.user = ID;
            envi.userFollow = user;
            signal(SIGUSR1, mimanejador);
            //Se envia informacion del usuario a dejar de seguir por el pipe azul del gestor.
            int ret1 = write(fd1, &envi, sizeof(envi));
            while (pausa == true)
              pause();
            pausa = true;
            cuantos = read(fd, &respuesta, sizeof(char));
            if (cuantos == -1) {
              perror("proceso cliente #5.1");
              exit(1);
            }
            if (respuesta == 'N') {
              printf("\nNo se pudo dejar de seguir al usuario.\n");
            } else if (respuesta == 'S') {
              printf("\nSe pudo dejar de seguir al usuario.\n");
            }
            impmenu = true;
          } else if (op == 4) { //  El usuario va a desconectarse.
            envi.accion = 6;
            envi.user = ID;
            signal(SIGUSR1, mimanejador);
            int ret1 = write(fd1, &envi, sizeof(envi));
            while (pausa == true)
              pause();
            cuantos = read(fd, &respuesta, sizeof(char));
            if (cuantos == -1) {
              perror("proceso cliente #6.1");
              exit(1);
            }
            if (respuesta == 'N') {
              printf("\nNo se pudo desconectar.\n");
            } else if (respuesta == 'S') {
              printf("\nSe pudo desconectar.\n");
            }
            close(fd1);
            close(fd);
            exit(1);
          } else if (op == 5) { //  El usuario va a refrescar sus tweets.
            envi.accion = 7;
            envi.user = ID;
            signal(SIGUSR1, mimanejadorhilo);
            int ret1 = write(fd1, &envi, sizeof(envi));
          } else {
            printf("\nOpción no valida.\n");
          }
        } while (true);
      }
    } else if (op == 1) { //  El usuario va a registrarse.
      envi.accion = 1;
      envi.user = ID;
      envi.pid = getpid();
      memcpy(envi.milinea, pipe, strlen(pipe) + 1);
      signal(SIGUSR1, mimanejador);
      int ret1 = write(fd1, &envi, sizeof(envi));
      while (pausa == true)
        pause();
      pausa = true;
      //Se recibe respuesta del gestor para comprobar si se pudo registrar el usuario.
      cuantos = read(fd, &respuesta, sizeof(char));
      if (cuantos == -1) {
        perror("proceso cliente #1.1");
        exit(1);
      }
      if (respuesta == 'N') {
        printf("\nNo se pudo registrar.\n");
      } else if (respuesta == 'S') {
        printf("\nSe pudo registrar.\n");
      }
    } else if (op == 3) {
      exit(1);
    } else
      printf("\nOpción no válida.\n");
  }
}