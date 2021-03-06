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
#include <signal.h>

///// DEFINES

#define ARCHIVO_CONFIGURACION "configuracion.config"
#define BACKLOG 10     // Cuántas conexiones pendientes se mantienen en cola
#define TAMANIO_CLAVE 41
#define TBLOQUEO 50
#define TAMANIO_NOMBREPROCESO 40
#define KEYS_POSIBLES 26


// ESTRUCTURAS Y ENUMS

typedef struct{
	int new_fd;
	sem_t * semaforo;
	sem_t * semaforoCompactacion;
	//struct node_t * colaProcesos;
	char nombreProceso[TAMANIO_NOMBREPROCESO];
	int cantidadEntradasMaximas;
	int entradasUsadas;
	int pid;
	t_list * claves;
	int conectada;
	int DeboRecibir;
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
	LISTAR = 1,
	BLOQUEAR = 2,
	DESBLOQUEAR = 3,
	KILL = 4,
	STATUS = 5
}tSolicitudesDeConsola;

typedef enum{
	OK = 1,
	BLOQUEO = 2,
	ERROR = 3,
	DESBLOQUEO = 4,
	STATUSDORRPUTO = 5,
	BORRANDO = 6,
	DESCONEXION = 7
}tResultadoOperacion;

typedef enum{
	LSU = 1,
	EL = 2,
	KE = 3
}tAlgoritmoDistribucion;

typedef enum{
	OPERACION_VALIDA = 1,
	OPERACION_INVALIDA= -1
}tValidezOperacion;

typedef enum{
	SOLICITAR_VALOR = 1,
	OPERAR = 2,
	COMPACTAR = 3,
	CUALQUIER_COSA = 4
}tOperacionInstancia;


typedef struct{
	tResultadoOperacion resultado;
	char clave[TAMANIO_CLAVE];
	tTipoOperacion tipoOperacion;
}__attribute__((packed)) tResultado;

/*
typedef struct{
	tResultadoOperacion resultado;
	char clave[TAMANIO_CLAVE];
}__attribute__((packed)) tResultado;
*/

typedef struct{
	tResultadoOperacion resultado;
	int tamanioValor;
}__attribute__((packed)) tResultadoInstancia;

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
	//char esi[TAMANIO_NOMBREPROCESO];
	int pid;
}tBloqueo; //podriamos agregar pid?

typedef struct{
	tResultadoOperacion tipoNotificacion;
	char clave[TAMANIO_CLAVE];
	int pid;
}tNotificacionPlanificador;

typedef struct{
	parametrosConexion * informacion;
	int cantidadEntradasMaximas;
	int entradasUsadas;
}tInstancia;

typedef struct{
	parametrosConexion * informacion;
	char clave[TAMANIO_CLAVE];
}tProcesoBloqueadoEsperandoClave;

typedef struct{
	int entradas;
	int tamanioEntradas;
	int cantidadClaves;
}tInformacionParaLaInstancia;

typedef struct{
	int entradasUsadas;
}tEntradasUsadas;

typedef struct{
	int cantidadClavesBloqueadas;
}tClavesBloqueadas;

typedef struct{
	tSolicitudesDeConsola solicitud;
}tSolicitudPlanificador;

typedef struct{
	int tamanioValor;
	char proceso[TAMANIO_NOMBREPROCESO];
}tStatusParaPlanificador;

typedef struct{
	tOperacionInstancia operacion;
}__attribute__((packed)) tOperacionInstanciaStruct;


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
tTipoOperacion OPERACION_ACTUAL;

parametrosConexion * ESIActual;
parametrosConexion * ESIABorrar;
parametrosConexion * planificador;
t_list* colaInstancias;
t_list* colaESIS;
t_list* colaMensajes;
t_list* colaResultados;
t_list* listaBloqueos;
t_list* clavesTomadas;
t_list* procesosBloqueadosEsperandoClave;
//t_list* colaMensajesParaPlanificador;

static volatile int keepRunning = 1;

pthread_mutex_t mutex;
sem_t semaforoInstancia;
sem_t semaforoESI;
int estoyBorrando;

tResultadoOperacion estadoConexion;

// FUNCIONES

void sigchld_handler(int s);
void intHandler(int dummy);
int main(int argc, char *argv[]);

int EscucharConexiones(int sockfd);
int IdentificarProceso(tHeader* headerRecibido, parametrosConexion* parametros);

int *gestionarConexion(parametrosConexion *parametros);
int *conexionESI(parametrosConexion* parametros);
int *conexionPlanificador(parametrosConexion* parametros);
int *conexionInstancia(parametrosConexion* parametros);
int *escucharMensajesDelPlanificador(parametrosConexion* parametros);

int ConexionESISinBloqueo(OperacionAEnviar* operacion, parametrosConexion* parametros);

int AnalizarOperacion(int tamanioValor,OperaciontHeader* header, parametrosConexion* parametros,
		OperacionAEnviar* operacion);
int ManejarOperacionGET(parametrosConexion* parametros, OperacionAEnviar* operacion);
int ManejarOperacionSET(int tamanioValor, parametrosConexion* parametros, OperacionAEnviar* operacion);
int ManejarOperacionSTORE(parametrosConexion* parametros, OperacionAEnviar* operacion);

bool yaExisteLaClave(void *claveDeLista,char * clave);
bool EncontrarEnLista(t_list * lista, char * claveABuscar);
bool LePerteneceLaClave(t_list * lista, tBloqueo * bloqueoBuscado);

int InicializarListasYColas();
int configure_logger();
int exit_gracefully(int return_nr);
int LeerArchivoDeConfiguracion(char *argv[]);
int verificarParametrosAlEjecutar(int argc, char *argv[]);
int RecibirClavesBloqueadas(parametrosConexion* parametros);

int SeleccionarInstancia(char * clave);
int SeleccionarPorEquitativeLoad(char * clave);
int SeleccionarPorLeastSpaceUsed(char * clave);
int SeleccionarPorKeyExplicit(char* clave);

char * SimulacionSeleccionarPorEquitativeLoad(char* clave);
char * SimulacionSeleccionarPorLeastSpaceUsed(char * clave);
char * SimulacionSeleccionarPorKeyExplicit(char* clave);

parametrosConexion* BuscarInstanciaMenosUsada();

int MandarAlFinalDeLaLista(t_list * lista, parametrosConexion * instancia);
int EnviarClaveYValorAInstancia(tTipoOperacion tipo, int tamanioValor,parametrosConexion* parametros, OperaciontHeader* header,OperacionAEnviar* operacion);
int RemoverInstanciaDeLaLista(parametrosConexion* parametros);
int AgregarClaveBloqueada(parametrosConexion* parametros);
int RemoverClaveDeClavesTomadas(char * clave);
int RemoverClaveDeLaListaBloqueos(char * claveABuscar);
int RemoverClaveDeClavesPropias(char * clave, parametrosConexion *parametros);
int BuscarSiLaInstanciaSeEstaReincorporando(parametrosConexion * parametros);

int EncontrarAlESIYEliminarlo(int id);
int LiberarLasClavesDelESI(parametrosConexion * parametros);
int BuscarClaveEnInstanciaYEnviar(char * clave);

bool EstaConectada(parametrosConexion * instancia);
bool EncontrarClaveEnClavesBloqueadas(t_list * lista, char * claveABuscar);
bool laClaveTuvoUnGETPrevio(char * clave,parametrosConexion * parametros);
bool EncontrarEnESIDistinto(t_list * lista, char * claveABuscar, parametrosConexion * parametros);

static void destruirResultado(tResultado * resultado);
static void destruirOperacionAEnviar(OperacionAEnviar * operacion);
static void destruirBloqueo(tBloqueo *bloqueo);
static void destruirInstancia(parametrosConexion * parametros);
static void borrarClave(char * clave);

int MandarInstanciasACompactar(parametrosConexion * instancia);
int MandarInstanciaACompactar(parametrosConexion * parametros);
int EliminarClaveDeInstancia(parametrosConexion * instancia, char * clave);
bool TieneLaClave(parametrosConexion * esi, char * clave);

int EliminarClaveDeBloqueos(char * claveABorrar);
parametrosConexion* BuscarInstanciaMenosUsadaSimulacion();
int VerificarSiLaInstanciaSigueViva(parametrosConexion * instancia);
int STATUSParaInstanciaConectada(parametrosConexion * instancia, char * clave);
int RecibirFinalizacionDeCompactacion(parametrosConexion * instancia);
parametrosConexion* BuscarInstanciaQuePoseeLaClave(char * clave);


