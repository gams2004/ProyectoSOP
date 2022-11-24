/*
Nombre del archivo: gestor.c // Autores: Santiago Molina, Gabriel Martín // Finalización 13/11/2022
El objetivo de este archivo es crear un gestor de solicitudes que , tras leer por medio de pipes, sepa cómo gestionar dicha solicitud y enviar una respuesta adecuada al creador de la solitud.
Solicitudes administradas:
1: registro
2: inicio sesión
3: mandar un tweet
4: follow
5: unfollow
6: desconexión
7: refrescar tweets

Componentes:
void Refrescar(int, envi, user []);
void Sig_handler();
int main(int, char**);

Compilación del programa:
gcc -o gestor gestor.c -pthread
Ejecución del programa:
./gestor -n 20 -r ArchivoEntrada.txt -m D -t 5 -p pipesoli
*/
#include "gestor.h"

void Refrescar(int fd1, envi lectura, user usuarios[]) {//Esta función emplea la variable global "enviados". Autores: Santiago Molina, Gabriel Martín; Parámetros: descriptor de archivo, objeto de tipo estructura Enviado, arreglo de objetos de tipo Usuario; no hay retorno; se encarga de imprimir los tweets almacenados en el gestor cuando un usuario inicia sesión.
  int rev = write(fd1, &usuarios[lectura.user - 1].mensajesalm, sizeof(int));
	int sum;
  for (int i = 0; i < usuarios[lectura.user - 1].mensajesalm; i++) {
    sum = 0;
    while (usuarios[lectura.user - 1].mensajes[i][sum] != '\0') {
      sum++;
    }
    sum++;
    char arreglotemp[sum];
    strcpy(arreglotemp, usuarios[lectura.user - 1].mensajes[i]);
    rev = write(fd1, &sum, sizeof(int));
    rev = write(fd1, &arreglotemp, sizeof(arreglotemp));
    enviados++;
  }
  usuarios[lectura.user - 1].mensajesalm = 0;
  usuarios[lectura.user - 1].mensajes = NULL;
}

void Sig_handler() {//Este manejador de señales emplea las variables globals "reloj", "conectados", "registrados", "enviados", "recibidos" y "printf_mutex". Autor: Santiago Molina; sin parámetros; no hay retorno; se encarga de imprimir las estadisticas por pantalla.
  pthread_mutex_lock(&printf_mutex);
  printf("Usuarios conectados: %i, Registrados: %i, Tweets enviados: %i, "
         "Tweets recibidos: %i.\n",
         conectados, registrados, enviados, recibidos);
  pthread_mutex_unlock(&printf_mutex);
  signal(SIGALRM, Sig_handler);
  alarm(reloj);
}
int main(int argc, char **argv) {//Esta función emplea las variables globales "reloj", "conectados", "registrados", "enviados", "recibidos" y "printf_mutex". Autores: Santiago Molina, Gabriel Martín; Parámetros: numero de parámetros al ejecutar el programa por línea de comandos, arreglo de parámetros al ejecutar el programa por línea de comandos; retorna un entero al finalizar la ejecución del programa; se encarga de inicializar las estructuras de datos del gestor (Arreglo de usuarios, matriz de seguidores, etc) y gestionar cada lectura de solicitudes de clientes con su respectivo procedimiento.
	conectados = 0, enviados = 0, recibidos = 0, registrados = 0;
  envi lectura;
  char *cadena = NULL;
  pthread_mutex_init(&printf_mutex, NULL);
  int filas, columnas, verificador, fd1, fd, pid, n, cuantos, res, cant;
  char *arch_mat;
  char modo;
  char *pipeazul;
  pthread_t thread1;
  bool banderas[] = {false, false, false, false, false};
	//Validación de los parámetros
  if (argc > 11) {
    perror(
        "Se exedió el numero de parámetros, debe ser: \n -n [número de "
        "usuarios] -r [nombre del archivo con usuarios registrados] -m [modo "
        "de operación del gestor] -t [valor numérico para el intervalo de "
        "impresión de estadísticas] -p [nombre del pipe del servidor]");
    exit(1);
  } else if (argc < 11) {
    perror(
        "Numero de parámetros incompleto, debe ser: \n -n [número de usuarios] "
        "-r [nombre del archivo con usuarios registrados] -m [modo de "
        "operación del gestor] -t [valor numérico para el intervalo de "
        "impresión de estadísticas] -p [nombre del pipe del servidor]");
    exit(1);
  }
  for (int i = 1; i < 11; i += 2) {
    if (strcmp("-n", argv[i]) == 0) {
      int mytemp;
      mytemp = strtol(argv[i + 1], NULL, 10);
      if (mytemp <= 80 && mytemp >= 1) {
        filas = strtol(argv[i + 1], NULL, 10);
        columnas = strtol(argv[i + 1], NULL, 10);
        if (banderas[0] == true) {
          perror("Parametro -n duplicado.");
          exit(1);
        }
        banderas[0] = true;
      } else {
        perror(
            "Numero de usuarios no válido, debe ser un número entre 1 y 80.");
        exit(1);
      }
    } else if (strcmp("-r", argv[i]) == 0) {
      arch_mat = argv[i + 1];
      if (banderas[1] == true) {
        perror("Parametro -r duplicado.");
        exit(1);
      }
      banderas[1] = true;
    } else if (strcmp("-m", argv[i]) == 0) {
      char mytemp;
      mytemp = *argv[i + 1];
      if (mytemp == 'D' || mytemp == 'A') {
        modo = *argv[i + 1];
        if (banderas[2] == true) {
          perror("Parametro -m duplicado.");
          exit(1);
        }
        banderas[2] = true;
      } else {
        perror("Caracter de modo no válido, debe ser el caracter 'A' o 'B'.");
        exit(1);
      }
    } else if (strcmp("-t", argv[i]) == 0) {
      if (isdigit(*argv[i + 1])) {
        reloj = strtol(argv[i + 1], NULL, 10);
        if (banderas[3] == true) {
          perror("Parametro -t duplicado.");
          exit(1);
        }
        banderas[3] = true;
      } else {
        perror("Entrada de tiempo no válida.");
        exit(1);
      }
    } else if (strcmp("-p", argv[i]) == 0) {
      pipeazul = argv[i + 1];
      if (banderas[4] == true) {
        perror("Parametro -p duplicado.");
        exit(1);
      }
      banderas[4] = true;
    }
  }
  for (verificador = 0; verificador < 5; verificador++) {
    if (banderas[verificador] != true)
      break;
  }
  if (verificador != 5) {
    perror("No se ingresaron todos los parámetros.");
    exit(1);
  }
	//Creación del hilo de impresión de estadísticas
  pthread_create(&thread1, NULL, (void*) Sig_handler, NULL);
	//Asignación de valores a algunos atributos de los usuarios.
  user usuarios[filas];
  for (int i = 0; i < filas; i++) {
    usuarios[i].codigo = -1;
    usuarios[i].conectado = false;
    usuarios[i].mensajesalm = 0;
  }
	//Creación de la matriz de usuarios.
  int **mat = (int **)malloc(filas * sizeof(int *));
  for (int i = 0; i < columnas; i++)
    mat[i] = (int *)malloc(columnas * sizeof(int));
	//Apertura del archivo de entrada.
  FILE *Fich;
  Fich = fopen(arch_mat, "r");
  if (Fich == NULL) {
    printf("Error al abrir el archivo.\n");
    exit(EXIT_FAILURE);
  } else
    printf("Exito al abrir el archivo.\n");
  size_t numero_bytes;
  int temp = getline(&cadena, &numero_bytes, Fich);
  cant = 0;
  if (!feof(Fich)) {
    for (int i = 0; i < filas; i++) {
      for (int j = 0; j < columnas; j++) {
        if (cant >= strlen(cadena)) {
          mat[i][j] = 0;
        } else if ((cant < strlen(cadena)) && ((isdigit(cadena[cant])) != 0)) {
          int a = cadena[cant] - '0';
          mat[i][j] = a;
          cant++;
        } else if (isdigit(cadena[cant]) == 0) {
          j--;
          cant++;
        }
      }
      if (cadena[cant - 1] == '\n' || cadena[cant - 1] == '1' ||
          cadena[cant - 1] == '0') {
        if (!feof(Fich)) {
          user usuario_registrado;
         	usuario_registrado.codigo = i + 1;
          usuario_registrado.conectado = false;
          usuarios[i] = usuario_registrado;
          cant = 0;
          int ver = getline(&cadena, &numero_bytes, Fich);
          if (feof(Fich)) {
            usuario_registrado.codigo = i + 2;
            usuario_registrado.conectado = false;
            usuarios[i + 1] = usuario_registrado;
            registrados = i + 2;
          }
        }
      }
    }
  } else {
    for (int i = 0; i < filas; i++) {
      for (int j = 0; j < columnas; j++) {
        mat[i][j] = 0;
      }
    }
  }
  free(cadena);
  /* Impresion de la matriz de usuarios para comprobación.
  for (int i = 0; i < a; i++) {
     for (int j = 0; j < b; j++) {
       printf("%d ", mat[i][j]);
     }
    printf("\n");
  }
  */
  fclose(Fich);
	//Se va a crear el pipe que lee el servidor.
  unlink(pipeazul);
  mode_t fifo_mode = S_IRUSR | S_IWUSR;
  if (mkfifo(pipeazul, fifo_mode) == -1) {
    perror("Error creando el pipe azul");
    exit(1);
  }
  fd = open(pipeazul, O_RDONLY);
  printf("Server Abrio el pipe azul.\n");
	//Ciclo infinito de lectura de solicitudes de clientes.
  while (true) {
		//Lee del pipe azul.
    do {
      cuantos = read(fd, &lectura, sizeof(lectura));
			printf("Cuantos: %d \n",cuantos);
      if (cuantos == -1) {
        perror("proceso gestor");
        exit(1);
      }
    } while (cuantos == 0);
		
    if (lectura.accion == 1) { // Un usuario quiere registrarse.
      int ya_registrado = 0;
      if (lectura.user < 1 || lectura.user > filas) {
        char resultado = 'N';
        fd1 = open(lectura.milinea, O_WRONLY);
        int rev = write(fd1, &resultado, sizeof(char));
        kill(lectura.pid, SIGUSR1);
        close(fd1);
      } else {
        for (int i = 0; i < filas; i++) {
          if (usuarios[i].codigo == lectura.user) {
            ya_registrado = 1;
            break;
          }
        }
        if (ya_registrado == 0) {
          char resultado = 'S';
          usuarios[lectura.user - 1].codigo = lectura.user;
          registrados++;
          fd1 = open(lectura.milinea, O_WRONLY);
          int rev = write(fd1, &resultado, sizeof(char));
          kill(lectura.pid, SIGUSR1);
        } else {
          char resultado = 'N';
          fd1 = open(lectura.milinea, O_WRONLY);
          int rev = write(fd1, &resultado, sizeof(char));
          kill(lectura.pid, SIGUSR1);
        }
        close(fd1);
      }
    } else if (lectura.accion == 2) { // Un usuario quiere iniciar sesión.
      if (lectura.user < 1 || lectura.user > filas) {
        char resultado = 'N';
        fd1 = open(lectura.milinea, O_WRONLY);
        int rev = write(fd1, &resultado, sizeof(char));
        kill(lectura.pid, SIGUSR1);
        close(fd1);
      } else if (usuarios[lectura.user - 1].codigo == -1) {
        char resultado = 'N';
        fd1 = open(lectura.milinea, O_WRONLY);
        int rev = write(fd1, &resultado, sizeof(char));
        kill(lectura.pid, SIGUSR1);
        close(fd1);
      } else {
        int ya_conectado = 0;
        if (usuarios[lectura.user - 1].conectado == true) {
           ya_conectado = 1;
        }
        if ( ya_conectado == 0) {
          char resultado = 'S';
          usuarios[lectura.user - 1].conectado = true;
          usuarios[lectura.user - 1].pid_cliente = lectura.pid;
          strcpy(usuarios[lectura.user - 1].milinea, lectura.milinea);
          conectados++;
          fd1 = open(lectura.milinea, O_WRONLY);
          int rev = write(fd1, &resultado, sizeof(char));
          kill(usuarios[lectura.user - 1].pid_cliente, SIGUSR1);
          Refrescar(fd1, lectura, usuarios);
        } else {
          char resultado = 'N';
          fd1 = open(lectura.milinea, O_WRONLY);
          int rek = write(fd1, &resultado, sizeof(char));
          kill(lectura.pid, SIGUSR1);
        }
        close(fd1);
      }
    } else if (lectura.accion == 3) { // Un usuario quiere mandar un tweet.
      recibidos++;
      if (modo == 'D') {//Si el modo es desacoplado.
        for (int i = 0; i < columnas; i++) {
          if (mat[i][lectura.user - 1] == 1) {
            if (usuarios[i].mensajesalm == 0) {
              usuarios[i].mensajes = malloc(sizeof(char *));
            }
            usuarios[i].mensajes =
                realloc(usuarios[i].mensajes,
                        sizeof(char *) * (usuarios[i].mensajesalm + 1));
            char *tweet = malloc(sizeof(char) * (sizeof(lectura.tweet)));
            strcpy(tweet, lectura.tweet);
            usuarios[i].mensajes[usuarios[i].mensajesalm] = tweet;
            usuarios[i].mensajesalm++;
          }
        }
      } else if (modo == 'A') {//Si el modo es acoplado.
        for (int i = 0; i < columnas; i++) {
          if (mat[i][lectura.user - 1] == 1) {
            if (usuarios[i].mensajesalm == 0) {
              usuarios[i].mensajes = malloc(sizeof(char *));
            }
            usuarios[i].mensajes =
                realloc(usuarios[i].mensajes,
                        sizeof(char *) * (usuarios[i].mensajesalm + 1));
            char *tweet = malloc(sizeof(char) * (sizeof(lectura.tweet)));
            strcpy(tweet, lectura.tweet);
            usuarios[i].mensajes[usuarios[i].mensajesalm] = tweet;
            usuarios[i].mensajesalm++;
            if (usuarios[i].conectado == true) {//Si el usuario esta conectado le envía el tweet inmediatamente.
              fd1 = open(usuarios[i].milinea, O_WRONLY);
              int rev = write(fd1, &usuarios[i].mensajesalm, sizeof(int));
              for (int ty = 0; ty < usuarios[i].mensajesalm; ty++) {
                int sum = 0;
                while (usuarios[i].mensajes[ty][sum] != '\0') {
                  sum++;
                }
                sum++;
                char arreglotemp[sum];
                strcpy(arreglotemp, usuarios[i].mensajes[ty]);
                int rev = write(fd1, &sum, sizeof(int));
                rev = write(fd1, &arreglotemp, sizeof(arreglotemp));
                enviados++;
              }
              kill(usuarios[i].pid_cliente, SIGUSR2);
              usuarios[i].mensajesalm = 0;
              usuarios[i].mensajes = NULL;
              close(fd1);
            }
          }
        }
      }
    } else if (lectura.accion == 4) { // Un usuario quiere seguir a alguien.
      fd1 = open(usuarios[lectura.user - 1].milinea, O_WRONLY);
      if (lectura.userFollow < 1 || lectura.userFollow > filas) {
        char resultado = 'N';
        int rev = write(fd1, &resultado, sizeof(char));
        kill(usuarios[lectura.user - 1].pid_cliente, SIGUSR1);
      } else if (lectura.user == lectura.userFollow) {
        char resultado = 'N';
        int rev = write(fd1, &resultado, sizeof(char));
        kill(usuarios[lectura.user - 1].pid_cliente, SIGUSR1);
      } else if (usuarios[lectura.userFollow - 1].codigo == -1) {
        char resultado = 'N';
        int rev = write(fd1, &resultado, sizeof(char));
        kill(usuarios[lectura.user - 1].pid_cliente, SIGUSR1);
      } else if (mat[lectura.user - 1][lectura.userFollow - 1] != 1) {
        mat[lectura.user - 1][lectura.userFollow - 1] = 1;
        char resultado = 'S';
        int rev = write(fd1, &resultado, sizeof(char));
        kill(usuarios[lectura.user - 1].pid_cliente, SIGUSR1);
      } else {
        char resultado = 'N';
        int rev = write(fd1, &resultado, sizeof(char));
        kill(usuarios[lectura.user - 1].pid_cliente, SIGUSR1);
      }
      close(fd1);
    } else if (lectura.accion == 5) {  // Un usuario quiere dejar de seguir a alguien.
      fd1 = open(usuarios[lectura.user - 1].milinea, O_WRONLY);
      if (lectura.userFollow < 1 || lectura.userFollow > filas) {
        char resultado = 'N';
        int rev = write(fd1, &resultado, sizeof(char));
        kill(usuarios[lectura.user - 1].pid_cliente, SIGUSR1);
      } else if (lectura.user == lectura.userFollow) {
        char resultado = 'N';
        int rev = write(fd1, &resultado, sizeof(char));
        kill(usuarios[lectura.user - 1].pid_cliente, SIGUSR1);
      } else if (usuarios[lectura.userFollow - 1].codigo == -1) {
        char resultado = 'N';
        int rev = write(fd1, &resultado, sizeof(char));
        kill(usuarios[lectura.user - 1].pid_cliente, SIGUSR1);
      } else if (mat[lectura.user - 1][lectura.userFollow - 1] != 0) {
        mat[lectura.user - 1][lectura.userFollow - 1] = 0;
        char resultado = 'S';
        int rev = write(fd1, &resultado, sizeof(char));
        kill(usuarios[lectura.user - 1].pid_cliente, SIGUSR1);
      } else {
        char resultado = 'N';
        int rev = write(fd1, &resultado, sizeof(char));
        kill(usuarios[lectura.user - 1].pid_cliente, SIGUSR1);
      }
      close(fd1);
    } else if (lectura.accion == 6) {  // Un usuario quiere desconectarse.
      int ya_conectado = 0;
      if (usuarios[lectura.user - 1].conectado == true) {
        ya_conectado =1;
      }
      if (ya_conectado == 1) {
        char resultado = 'S';
        usuarios[lectura.user - 1].conectado = false;
        conectados--;
        fd1 = open(lectura.milinea, O_WRONLY);
        int rev = write(fd1, &resultado, sizeof(char));
        kill(usuarios[lectura.user - 1].pid_cliente, SIGUSR1);
      } else {
        char resultado = 'N';
        fd1 = open(lectura.milinea, O_WRONLY);
        int rek = write(fd1, &resultado, sizeof(char));
        kill(usuarios[lectura.user - 1].pid_cliente, SIGUSR1);
      }
      close(fd1);
    } else if (lectura.accion == 7) { // Un usuario quiere refrescar sus tweets.
      fd1 = open(usuarios[lectura.user - 1].milinea, O_WRONLY);
      if (modo == 'D') {
        int rev =write(fd1, &usuarios[lectura.user - 1].mensajesalm, sizeof(int));
        for (int i = 0; i < usuarios[lectura.user - 1].mensajesalm; i++) {
          int sum = 0;
          while (usuarios[lectura.user - 1].mensajes[i][sum] != '\0') {
            sum++;
          }
          sum++;
          char arreglotemp[sum];
          strcpy(arreglotemp, usuarios[lectura.user - 1].mensajes[i]);
          rev = write(fd1, &sum, sizeof(int));
          rev = write(fd1, &arreglotemp, sizeof(arreglotemp));
          enviados++;
        }
        kill(usuarios[lectura.user - 1].pid_cliente, SIGUSR1);
        usuarios[lectura.user - 1].mensajesalm = 0;
        usuarios[lectura.user - 1].mensajes = NULL;
      } else {
        int minus = -1;
        int rev = write(fd1, &minus, sizeof(int));
        kill(usuarios[lectura.user - 1].pid_cliente, SIGUSR1);
      }
      close(fd1);
    }
  }
}
