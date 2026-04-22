# Universidad Autónoma de Tamaulipas  
## Facultad de Ingeniería y Ciencias  
### Materia: Programación Estructurada  
#### Docente: Dr. Alan Díaz Manríquez

# 🎛️ Convey: Sistema Clasificador de Objetos por Color

En la Facultad de Ingeniería y Ciencias (FIC), surge la necesidad de contar con una herramienta que permita desarrollar, probar y ejecutar programas en lenguaje C para el control de un **sistema clasificador de objetos por color**, tanto en un entorno **simulado** como en un entorno **físico**.

Con este propósito se desarrolló **Convey**, una librería en C que abstrae la comunicación con el sistema clasificador y permite que los alumnos se enfoquen en la lógica del programa, utilizando primitivas sencillas para controlar la banda transportadora, leer el color detectado y operar las compuertas de clasificación.

Este proyecto está pensado como base para prácticas y proyectos finales de la materia de **Programación Estructurada**, permitiendo aplicar conocimientos como:

- Uso de estructuras condicionales y repetitivas
- Modularidad mediante funciones
- Manejo de arreglos
- Historial circular
- Estadísticas de ejecución
- Menús interactivos
- Comunicación con dispositivos externos
- Portabilidad entre simulador y hardware real

Cada equipo será responsable de desarrollar el programa principal en C que interactúe con el sistema clasificador por medio de la librería **Convey**.

---

## Menú de Contenidos

- [Descripción General](#descripción-general)
- [Arquitectura del Proyecto](#arquitectura-del-proyecto)
- [Librería `convey`](#librería-convey)
  - [Primitivas disponibles](#primitivas-disponibles)
  - [Conexión por TCP](#conexión-por-tcp)
  - [Conexión por Serial](#conexión-por-serial)
  - [Funciones de teclado portable](#funciones-de-teclado-portable)
- [Compilación](#compilación)
- [ConveySimulator](#conveysimulator)
- [Documentación](#documentación)
- [Evaluación del Proyecto](#evaluación-del-proyecto)
- [Cronograma de Exposición de Proyectos](#cronograma-de-exposición-de-proyectos)
- [Archivos Entregables](#-archivos-entregables)
- [Créditos](#créditos)

---

## Descripción General

El sistema clasificador consta de los siguientes elementos principales:

- Una **banda transportadora**
- Un **sensor de color**
- Dos **compuertas**
- Tres destinos de clasificación:
  - **Puerta 1**
  - **Puerta 2**
  - **Caja final / Box_3**
- Un sistema de control físico basado en **Arduino**
- Un entorno visual de prueba mediante **ConveySimulator**

El objetivo del programa desarrollado por el alumno es leer el color detectado por el sistema y decidir a qué destino debe enviarse cada objeto, utilizando las primitivas proporcionadas por la librería `convey`.

La lógica básica de clasificación es:

- **Rojo** → Abrir **Puerta 1**
- **Verde** → Abrir **Puerta 2**
- **Amarillo** → No abrir ninguna compuerta, el objeto continúa hacia **Box_3**

---

## Arquitectura del Proyecto

La librería Convey está diseñada para trabajar en dos modos:

### 1. Modo Simulado

El programa en C se conecta por **TCP** a **ConveySimulator**, el cual simula visualmente:

- El movimiento de la banda
- El paso de objetos
- La lectura del sensor
- La apertura/cierre de compuertas
- La clasificación final

Arquitectura general:

```text
Programa en C --> TCP --> ConveySimulator
```

---

### 2. Modo Físico

El programa en C se conecta por **serial** a un sistema físico controlado con Arduino.

Arquitectura general:

```text
Programa en C --> Serial --> Arduino / Sistema físico
```

---

## Librería `convey`

La librería `convey` abstrae toda la comunicación con el sistema, ya sea simulado o físico. Esto permite que el alumno escriba un solo programa principal en C y cambie únicamente la forma de inicializar la conexión.

Archivos principales:

- `convey.h`
- `convey.c`

---

### Primitivas disponibles

La librería expone las siguientes funciones:

#### Inicialización

```c
int inicializarConexionTCP(const char* host, int puerto);
int inicializarConexionSerial(const char* puerto, int baudrate);
int cerrarConexion(void);
```

#### Control del sistema

```c
int encenderBanda(void);
int apagarBanda(void);
int leerColor(void);
int abrirPuerta(int puerta);
int cerrarPuerta(int puerta);
int cerrarPuertas(void);
```

#### Utilidades de teclado portable

```c
int teclaDisponible(void);
int leerTecla(void);
void restaurarTerminal(void);
```

Estas funciones permiten detectar teclas sin bloquear totalmente la ejecución del programa y funcionan de manera portable en Windows y macOS/Linux.

---

### Conexión por TCP

Para utilizar Convey con el simulador:

```c
inicializarConexionTCP("127.0.0.1", 5000);
```

Esto requiere que **ConveySimulator** esté ejecutándose previamente y escuchando en el puerto configurado.

---

### Conexión por Serial

Para utilizar Convey con el sistema físico:

#### macOS / Linux

```c
inicializarConexionSerial("/dev/cu.usbserial-1140", 115200);
```

#### Windows

```c
inicializarConexionSerial("COM5", 115200);
```

> **Nota:** En Windows, si el puerto es `COM10` o mayor, puede ser necesario utilizar el formato:
>
> ```c
> inicializarConexionSerial("\\\\.\\COM10", 115200);
> ```

---

### Funciones de teclado portable

La librería incluye funciones auxiliares para detectar teclas en modo consola de forma multiplataforma:

- `teclaDisponible()`  
  Verifica si el usuario ha presionado alguna tecla.

- `leerTecla()`  
  Lee la tecla presionada.

- `restaurarTerminal()`  
  Restaura el estado de la terminal al finalizar el programa (especialmente importante en macOS/Linux).

#### Ejemplo básico:

```c
while (1) {
    if (teclaDisponible()) {
        int c = leerTecla();
        if (c == 'q' || c == 'Q') {
            break;
        }
    }
}
restaurarTerminal();
```

---

## Compilación

### macOS / Linux

```bash
clang main.c convey.c -o programa
```

o

```bash
gcc main.c convey.c -o programa
```

### Windows (MinGW)

```bash
gcc main.c convey.c -o programa.exe -lws2_32
```

> En Windows, la librería `ws2_32` es necesaria para la comunicación TCP.

---

## ConveySimulator

**ConveySimulator** es la aplicación visual asociada a la librería `convey`, diseñada para permitir el desarrollo y prueba del sistema sin necesidad de utilizar el hardware físico.

Entre sus características se incluyen:

- Simulación de banda transportadora
- Simulación de sensor de color
- Simulación de compuertas
- Interfaz visual con paneles de depuración
- Indicadores de color y estado
- Comunicación TCP con programas en C
- Soporte para modo ventana y ejecución independiente

---

### Instalador

Más adelante este proyecto incluirá un instalador de **ConveySimulator** para facilitar su distribución y ejecución sin necesidad de abrir Unity manualmente.

Se contempla soporte futuro para:

- Instalador en **macOS**
- Instalador en **Windows**
- Ejecución simplificada del simulador
- Integración con ejemplos base

> Esta funcionalidad será agregada posteriormente.

---

## Documentación

La documentación técnica de la librería `convey` será generada con **Doxygen** y estará disponible en la carpeta `docs/`.

### Enlace planeado

- [Documentación HTML](docs/index.html)

> La documentación será añadida posteriormente.

---

## Evaluación del Proyecto

El programa será evaluado con base en aspectos como:

- Implementación correcta de la lógica de clasificación
- Uso adecuado de funciones
- Uso de estructuras condicionales y repetitivas
- Implementación de un menú interactivo
- Generación de estadísticas
- Uso de historial circular
- Correcta operación de modo automático y modo manual
- Claridad del programa y organización del código
- Correcta compilación y ejecución

> **Nota importante:** Aunque el proyecto puede trabajarse en equipo, la **evaluación será individual**.  
> Cada alumno deberá demostrar de forma clara su comprensión del funcionamiento del programa durante la presentación o revisión.

---

## Cronograma de Exposición de Proyectos

Las exposiciones se llevarán a cabo de manera **presencial**, utilizando la computadora del alumno o equipo conectada al sistema correspondiente.

> **Lugar:** Por definir

> **Fecha y hora de entrega:** DD/MM/AAAA - HH:MM horas

> El archivo fuente y el reporte deberán entregarse por el medio que indique el docente.

> 📌 **Es requisito para entrega que el código compile correctamente**

### Fechas y horarios de presentación

| Fecha       | Horario         | Integrantes del equipo |
|-------------|-----------------|------------------------|
| DD/MM/AAAA  | HH:MM - HH:MM   |                        |
| DD/MM/AAAA  | HH:MM - HH:MM   |                        |
| DD/MM/AAAA  | HH:MM - HH:MM   |                        |
| DD/MM/AAAA  | HH:MM - HH:MM   |                        |
| DD/MM/AAAA  | HH:MM - HH:MM   |                        |
| DD/MM/AAAA  | HH:MM - HH:MM   |                        |
| DD/MM/AAAA  | HH:MM - HH:MM   |                        |
| DD/MM/AAAA  | HH:MM - HH:MM   |                        |
| DD/MM/AAAA  | HH:MM - HH:MM   |                        |
| DD/MM/AAAA  | HH:MM - HH:MM   |                        |

(Los tiempos son aproximados; **es responsabilidad del alumno estar pendiente de la fecha y hora que le corresponde**)

---

## 📄 Archivos Entregables

Este proyecto podrá realizarse en **equipo** (según lo indique el docente). Cada equipo o alumno deberá entregar al menos:

1. `main.c` – Programa principal en C que implementa la lógica requerida.
2. `reporte.pdf` – Documento con la descripción del proyecto.

### Estructura sugerida del reporte

- **Portada**
  - Nombres completos de los integrantes
  - Matrículas y grupo
  - Nombre del proyecto

- **1. Descripción general del proyecto**
  - ¿Qué hace el sistema?
  - ¿Cómo se organiza el programa?
  - ¿Qué modos implementa?

- **2. Lógica de clasificación**
  - Cómo se toma la decisión según el color detectado
  - Qué ocurre con cada compuerta

- **3. Estructuras de programación utilizadas**
  - Funciones
  - Condicionales
  - Ciclos
  - Arreglos
  - Historial circular

- **4. Estadísticas e historial**
  - Cómo se almacenan
  - Cómo se muestran

- **5. Dificultades enfrentadas y aprendizajes**
  - Qué problemas se presentaron
  - Qué soluciones se implementaron

- **6. Evidencia visual (opcional)**
  - Capturas de pantalla
  - Fotografías del prototipo o simulador

---

## Créditos

Este proyecto fue desarrollado como base para prácticas y proyectos integradores de la materia de **Programación Estructurada**, enfocándose en la construcción de lógica de control para un sistema clasificador de objetos por color, tanto en entorno simulado como físico.
