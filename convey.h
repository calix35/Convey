#ifndef CONVEY_H
#define CONVEY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file convey.h
 * @brief Librería para controlar un sistema clasificador de objetos por color.
 *
 * Esta librería permite controlar una banda transportadora y compuertas de
 * clasificación, así como leer el color detectado por el sistema.
 *
 * Puede trabajar en dos modos:
 * - Comunicación TCP con un simulador
 * - Comunicación serial con un sistema físico basado en Arduino
 *
 * También incluye funciones auxiliares para manejo portable de teclado.
 */

/**
 * @def COLOR_SIN_OBJETO
 * @brief Código para indicar que no hay objeto detectado.
 */
#define COLOR_SIN_OBJETO 0

/**
 * @def COLOR_RED
 * @brief Código para indicar que se detectó el color rojo.
 */
#define COLOR_RED        1

/**
 * @def COLOR_GREEN
 * @brief Código para indicar que se detectó el color verde.
 */
#define COLOR_GREEN      2

/**
 * @def COLOR_YELLOW
 * @brief Código para indicar que se detectó el color amarillo.
 */
#define COLOR_YELLOW     3

/**
 * @def COLOR_ERROR
 * @brief Código para indicar error en la lectura del color.
 */
#define COLOR_ERROR     -1

/**
 * @def MODO_NINGUNO
 * @brief Estado inicial sin conexión activa.
 */
#define MODO_NINGUNO 0

/**
 * @def MODO_TCP
 * @brief Modo de conexión por TCP.
 */
#define MODO_TCP     1

/**
 * @def MODO_SERIAL
 * @brief Modo de conexión por puerto serial.
 */
#define MODO_SERIAL  2

/**
 * @brief Inicializa la conexión con el simulador mediante TCP.
 *
 * Establece una conexión con un servidor TCP, normalmente el simulador visual
 * del sistema clasificador.
 *
 * @param host Dirección IP o nombre del host.
 * @param puerto Puerto TCP del servidor.
 * @return 0 si la conexión fue exitosa, -1 en caso de error.
 */
int inicializarConexionTCP(const char* host, int puerto);

/**
 * @brief Inicializa la conexión con el sistema físico por puerto serial.
 *
 * Abre y configura un puerto serial para comunicarse con el hardware real
 * controlado por Arduino.
 *
 * @param puerto Nombre del puerto serial.
 *               Ejemplo en macOS/Linux: "/dev/cu.usbserial-1140"
 *               Ejemplo en Windows: "COM5"
 * @param baudrate Velocidad de comunicación serial.
 * @return 0 si la conexión fue exitosa, -1 en caso de error.
 */
int inicializarConexionSerial(const char* puerto, int baudrate);

/**
 * @brief Cierra la conexión activa.
 *
 * Cierra la conexión actual, ya sea TCP o serial.
 *
 * @return 0 al finalizar correctamente.
 */
int cerrarConexion(void);

/**
 * @brief Enciende la banda transportadora.
 *
 * Envía el comando correspondiente al sistema para iniciar el movimiento
 * de la banda.
 *
 * @return 0 si la operación fue exitosa, -1 en caso de error.
 */
int encenderBanda(void);

/**
 * @brief Apaga la banda transportadora.
 *
 * Envía el comando correspondiente al sistema para detener la banda.
 *
 * @return 0 si la operación fue exitosa, -1 en caso de error.
 */
int apagarBanda(void);

/**
 * @brief Lee el color detectado por el sensor.
 *
 * Solicita al sistema la lectura actual del sensor de color.
 *
 * @return Uno de los siguientes códigos:
 * - COLOR_RED
 * - COLOR_GREEN
 * - COLOR_YELLOW
 * - COLOR_SIN_OBJETO
 * - COLOR_ERROR
 */
int leerColor(void);

/**
 * @brief Abre una compuerta de clasificación.
 *
 * @param puerta Número de compuerta a abrir.
 *               Valores esperados: 1 o 2.
 * @return 0 si la operación fue exitosa, -1 en caso de error.
 */
int abrirPuerta(int puerta);

/**
 * @brief Cierra una compuerta de clasificación.
 *
 * @param puerta Número de compuerta a cerrar.
 *               Valores esperados: 1 o 2.
 * @return 0 si la operación fue exitosa, -1 en caso de error.
 */
int cerrarPuerta(int puerta);

/**
 * @brief Cierra todas las compuertas del sistema.
 *
 * @return 0 si la operación fue exitosa, -1 en caso de error.
 */
int cerrarPuertas(void);

/**
 * @brief Verifica si hay una tecla disponible para lectura.
 *
 * Esta función permite detectar entrada de teclado sin bloquear totalmente
 * la ejecución del programa.
 *
 * @return 1 si hay una tecla disponible, 0 en caso contrario.
 */
int teclaDisponible(void);

/**
 * @brief Lee una tecla desde el teclado.
 *
 * Debe usarse normalmente después de verificar con `teclaDisponible()`.
 *
 * @return Código entero de la tecla leída, o -1 si no se pudo leer.
 */
int leerTecla(void);

/**
 * @brief Restaura el estado normal de la terminal.
 *
 * En sistemas POSIX (como macOS/Linux), algunas funciones de teclado cambian
 * temporalmente la configuración de la terminal. Esta función la restaura.
 *
 * En Windows, esta función no realiza acciones adicionales.
 */
void restaurarTerminal(void);

#ifdef __cplusplus
}
#endif

#endif /* CONVEY_H */