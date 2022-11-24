#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct enviado {
  int accion; //Solicitud del usuario.
  int user; //Id usuario
  int userFollow; //Id usuario a seguir
  char milinea[20]; //nombre del pipe
  char tweet[200]; //contenido del tweet 
  int pid;// pid del cliente
  long pid_lector;//pid del hilo que lee del cliente
} envi;
typedef struct usuario {
  int codigo; //Codigo del usuario en la red social.
  int conectado; //Estado de conexión.
  char **mensajes; //Mensajes recibidos que no se han enviado al usuario.
  int mensajesalm; //Numero de mensajes recibidos que no se han enviado al usuario.
  int pid_cliente; //pid del proceso que está ejecutando al cliente del usuario.
  char milinea[20]; //Nombre del pipe para comunicación con el proceso del cliente del usuario.
} user;

bool pausa = true, impmenu=false;
pid_t getpid(void);
pthread_mutex_t lock;
