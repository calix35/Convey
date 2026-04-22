#include <stdio.h>
#include "convey.h"

/*
 * Ejemplo basico de uso de la libreria Convey.
 * Este programa:
 * 1. Se conecta al simulador por TCP
 * 2. Enciende la banda
 * 3. Lee un color
 * 4. Actua segun el color detectado
 * 5. Apaga la banda y cierra la conexion
 */

int main(void)
{
    int color;

    if (inicializarConexionTCP("127.0.0.1", 5000) != 0) {
        printf("Error: no se pudo conectar al simulador.\n");
        return 1;
    }

    printf("Conexion establecida correctamente.\n");

    if (encenderBanda() != 0) {
        printf("Error al encender la banda.\n");
        cerrarConexion();
        restaurarTerminal();
        return 1;
    }

    printf("Banda encendida.\n");

    color = leerColor();

    printf("Color detectado: %s\n", colorATexto(color));

    if (color == COLOR_RED) {
        printf("Abriendo puerta 1...\n");
        abrirPuerta(1);
    } else if (color == COLOR_GREEN) {
        printf("Abriendo puerta 2...\n");
        abrirPuerta(2);
    } else if (color == COLOR_YELLOW) {
        printf("No se abre ninguna puerta. El objeto va a Box_3.\n");
        cerrarPuertas();
    } else if (color == COLOR_SIN_OBJETO) {
        printf("No hay objeto frente al sensor.\n");
    } else {
        printf("Ocurrio un error al leer el color.\n");
    }

    apagarBanda();
    cerrarPuertas();
    cerrarConexion();
    restaurarTerminal();

    printf("Programa finalizado.\n");

    return 0;
}