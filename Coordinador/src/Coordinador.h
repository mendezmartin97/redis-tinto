/*
 * Coordinador.h
 *
 *  Created on: 4 abr. 2018
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <sys/queue.h>
#include <commons/txt.h>
#include <netdb.h>
#include <sys/socket.h>
#include <semaphore.h>

///// DEFINES

#define MYPORT 3490    // Puerto al que conectarán los usuarios
#define ARCHIVO_CONFIGURACION "configuracion.config"
#define BACKLOG 10     // Cuántas conexiones pendientes se mantienen en cola
#define TAMANIO_CLAVE 41
#define TBLOQUEO 50
#define TAMANIO_NOMBREPROCESO 40
#define KEYS_POSIBLES 25


// ESTRUCTURAS Y ENUMS

typedef struct{
	int new_fd;
	sem_t * semaforo;
	struct node_t * colaProcesos;
	char nombreProceso[TAMANIO_NOMBREPROCESO];
}parametrosConexion;  // Aqui dejamos los descriptores y un semaforo para los hilos que lo necesiten


typedef enum{
	ESI = 1,
	PLANIFICADOR = 2,
	COORDINADOR = 3,
	INSTANCIA = 4

}tTipoDeProceso;

typedef enum{
	CONECTARSE = 1
}tTipoDeMensaje;

typedef enum{
	OPERACION_GET = 1,
	OPERACION_SET = 2,
	OPERACION_STORE = 3
}tTipoOperacion;

typedef enum{
	OK = 1,
	BLOQUEO = 2,
	ERROR = 3,
	DESBLOQUEO = 4
}tResultadoOperacion;

typedef enum{
	LSU = 1,
	EL = 2,
	KE = 3
}tAlgoritmoDistribucion;


typedef struct{
	tResultadoOperacion resultado;
	char clave[TAMANIO_CLAVE];
}__attribute__((packed)) tResultado;

typedef struct{
	tTipoDeProceso tipoProceso;
	tTipoDeMensaje tipoMensaje;
	int idProceso;
	char nombreProceso[TAMANIO_NOMBREPROCESO];
}__attribute__((packed)) tHeader; // Header que recibimos de los procesos para identificarlos

typedef struct {
  tTipoOperacion tipo;
  int tamanioValor;
}__attribute__((packed)) OperaciontHeader; // Header que vamos a recibir de parte del ESI para identificar la operacion

typedef struct {
	tTipoOperacion tipo;
	char clave[TAMANIO_CLAVE];
	char* valor;
}__attribute__((packed)) OperacionAEnviar; // Operacion que vamos a enviar a la instancia

typedef struct {
	char * data;
	int size;

}__attribute__((packed)) stream;

typedef struct {
	char clave[TAMANIO_CLAVE];
	char esi[TAMANIO_NOMBREPROCESO];
}tBloqueo; //podriamos agregar pid?

typedef struct{
	tResultadoOperacion tipoNotificacion;
	char clave[TAMANIO_CLAVE];
	char esi[TAMANIO_NOMBREPROCESO];
}tNotificacionPlanificador;

typedef struct{
	parametrosConexion * informacion;
	int cantidadEntradasMaximas;
	int entradasUsadas;
}tInstancia;

// VARIABLES GLOBALES

int PUERTO;
char* IP;
tAlgoritmoDistribucion ALGORITMO;
char* ALGORITMO_CONFIG;
int ENTRADAS;
int TAMANIO_ENTRADAS;
int RETARDO;
t_log * logger;
tNotificacionPlanificador * notificacion;
char CLAVE[TAMANIO_CLAVE];

parametrosConexion * planificador;
t_list* colaInstancias;
t_list* colaESIS;
t_list* colaMensajes;
t_list* colaResultados;
t_list* colaBloqueos;

pthread_mutex_t mutex;


// FUNCIONES

void sigchld_handler(int s);
int main(void);
int EscucharConexiones(int sockfd);
int IdentificarProceso(tHeader* headerRecibido, parametrosConexion* parametros);
int configure_logger();
int exit_gracefully(int return_nr);
int *gestionarConexion(parametrosConexion *parametros);
int *conexionESI(parametrosConexion* parametros);
int *conexionPlanificador(parametrosConexion* parametros);
int *conexionInstancia(parametrosConexion* parametros);
int AnalizarOperacion(int tamanioValor,OperaciontHeader* header, parametrosConexion* parametros,
		OperacionAEnviar* operacion);
int ManejarOperacionGET(parametrosConexion* parametros, OperacionAEnviar* operacion);
int ManejarOperacionSET(int tamanioValor, parametrosConexion* parametros, OperacionAEnviar* operacion);
int ManejarOperacionSTORE(parametrosConexion* parametros, OperacionAEnviar* operacion);
int InicializarListasYColas();
bool yaExisteLaClave(void *claveDeLista,char * clave);
bool EncontrarEnLista(t_list * lista, char * claveABuscar);
bool LePerteneceLaClave(t_list * lista, tBloqueo * bloqueoBuscado);
int LeerArchivoDeConfiguracion();
int SeleccionarInstancia(char * clave);
int SeleccionarPorEquitativeLoad();
static void destruirBloqueo(tBloqueo *bloqueo);
void RemoverDeLaLista(t_list * lista, char * claveABuscar);
int MandarAlFinalDeLaLista(t_list * lista, tInstancia * instancia);
tInstancia* BuscarInstanciaMenosUsada(t_list * lista);

