#include "convey.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <conio.h>
    #pragma comment(lib, "ws2_32.lib")
    static SOCKET tcp_sockfd = INVALID_SOCKET;
    static HANDLE serial_handle = INVALID_HANDLE_VALUE;
#else
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <termios.h>
    #include <sys/select.h>
    static int tcp_sockfd = -1;
    static int serial_fd = -1;
#endif

#define RESP_SIZE 256

static int modoConexion = MODO_NINGUNO;

#ifndef _WIN32
static struct termios oldt;
static int terminalConfigurada = 0;
#endif

/* ============================================================
 * UTILIDADES GENERALES
 * ============================================================ */

static void limpiarFinLinea(char* s)
{
    if (s == NULL) return;

    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

/* ============================================================
 * TCP
 * ============================================================ */

static int enviarComandoTCP(const char* comando, char* respuesta, size_t maxlen)
{
    if (comando == NULL || respuesta == NULL || maxlen == 0)
        return -1;

#ifdef _WIN32
    if (tcp_sockfd == INVALID_SOCKET)
        return -1;
#else
    if (tcp_sockfd < 0)
        return -1;
#endif

    char buffer[RESP_SIZE];
    int n;

    snprintf(buffer, sizeof(buffer), "%s\n", comando);

#ifdef _WIN32
    if (send(tcp_sockfd, buffer, (int)strlen(buffer), 0) <= 0)
        return -1;

    n = recv(tcp_sockfd, respuesta, (int)(maxlen - 1), 0);
#else
    if (send(tcp_sockfd, buffer, strlen(buffer), 0) <= 0)
        return -1;

    n = (int)recv(tcp_sockfd, respuesta, maxlen - 1, 0);
#endif

    if (n <= 0)
        return -1;

    respuesta[n] = '\0';
    limpiarFinLinea(respuesta);

    return 0;
}

int inicializarConexionTCP(const char* host, int puerto)
{
    if (host == NULL || puerto <= 0)
        return -1;

    cerrarConexion();

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return -1;

    tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sockfd == INVALID_SOCKET)
        return -1;
#else
    tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sockfd < 0)
        return -1;
#endif

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons((unsigned short)puerto);

    if (inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0)
        return -1;

#ifdef _WIN32
    if (connect(tcp_sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
        return -1;
#else
    if (connect(tcp_sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        return -1;
#endif

    modoConexion = MODO_TCP;
    return 0;
}

/* ============================================================
 * SERIAL - WINDOWS
 * ============================================================ */
#ifdef _WIN32

static DWORD baudrateWindows(int baudrate)
{
    switch (baudrate) {
        case 9600:   return CBR_9600;
        case 19200:  return CBR_19200;
        case 38400:  return CBR_38400;
        case 57600:  return CBR_57600;
        case 115200: return CBR_115200;
        default:     return CBR_9600;
    }
}

static int configurarSerialWindows(HANDLE hSerial, int baudrate)
{
    DCB dcbSerialParams;
    COMMTIMEOUTS timeouts;

    memset(&dcbSerialParams, 0, sizeof(dcbSerialParams));
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams))
        return -1;

    dcbSerialParams.BaudRate = baudrateWindows(baudrate);
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity   = NOPARITY;
    dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;
    dcbSerialParams.fRtsControl = RTS_CONTROL_ENABLE;
    dcbSerialParams.fOutxCtsFlow = FALSE;
    dcbSerialParams.fOutxDsrFlow = FALSE;
    dcbSerialParams.fOutX = FALSE;
    dcbSerialParams.fInX  = FALSE;

    if (!SetCommState(hSerial, &dcbSerialParams))
        return -1;

    timeouts.ReadIntervalTimeout         = 50;
    timeouts.ReadTotalTimeoutConstant    = 100;
    timeouts.ReadTotalTimeoutMultiplier  = 10;
    timeouts.WriteTotalTimeoutConstant   = 100;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts))
        return -1;

    PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

    return 0;
}

static int enviarComandoSerialWindows(const char* comando, char* respuesta, size_t maxlen)
{
    if (comando == NULL || respuesta == NULL || maxlen == 0)
        return -1;

    if (serial_handle == INVALID_HANDLE_VALUE)
        return -1;

    char buffer[RESP_SIZE];
    DWORD bytesWritten = 0;
    DWORD bytesRead = 0;
    size_t pos = 0;
    char c;

    snprintf(buffer, sizeof(buffer), "%s\n", comando);

    if (!WriteFile(serial_handle, buffer, (DWORD)strlen(buffer), &bytesWritten, NULL))
        return -1;

    while (pos < maxlen - 1) {
        if (!ReadFile(serial_handle, &c, 1, &bytesRead, NULL))
            return -1;

        if (bytesRead == 0)
            continue;

        respuesta[pos++] = c;

        if (c == '\n')
            break;
    }

    respuesta[pos] = '\0';
    limpiarFinLinea(respuesta);

    return 0;
}

#endif

/* ============================================================
 * SERIAL - POSIX (macOS / Linux)
 * ============================================================ */
#ifndef _WIN32

static speed_t baudratePosix(int baudrate)
{
    switch (baudrate) {
        case 9600:   return B9600;
        case 19200:  return B19200;
        case 38400:  return B38400;
        case 57600:  return B57600;
        case 115200: return B115200;
#ifdef B230400
        case 230400: return B230400;
#endif
        default:     return B9600;
    }
}

static int configurarSerialPosix(int fd, int baudrate)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) != 0)
        return -1;

    cfsetospeed(&tty, baudratePosix(baudrate));
    cfsetispeed(&tty, baudratePosix(baudrate));

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;

    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 10;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
#ifdef CRTSCTS
    tty.c_cflag &= ~CRTSCTS;
#endif

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
        return -1;

    tcflush(fd, TCIOFLUSH);

    return 0;
}

static int enviarComandoSerialPosix(const char* comando, char* respuesta, size_t maxlen)
{
    if (comando == NULL || respuesta == NULL || maxlen == 0)
        return -1;

    if (serial_fd < 0)
        return -1;

    char buffer[RESP_SIZE];
    ssize_t n;
    size_t pos = 0;
    char c;

    snprintf(buffer, sizeof(buffer), "%s\n", comando);

    if (write(serial_fd, buffer, strlen(buffer)) < 0)
        return -1;

    while (pos < maxlen - 1) {
        n = read(serial_fd, &c, 1);

        if (n < 0)
            return -1;

        if (n == 0)
            continue;

        respuesta[pos++] = c;

        if (c == '\n')
            break;
    }

    respuesta[pos] = '\0';
    limpiarFinLinea(respuesta);

    return 0;
}

#endif

/* ============================================================
 * SERIAL - API PUBLICA
 * ============================================================ */

int inicializarConexionSerial(const char* puerto, int baudrate)
{
    if (puerto == NULL || baudrate <= 0)
        return -1;

    cerrarConexion();

#ifdef _WIN32
    serial_handle = CreateFileA(
        puerto,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (serial_handle == INVALID_HANDLE_VALUE)
        return -1;

    if (configurarSerialWindows(serial_handle, baudrate) != 0)
        return -1;

    Sleep(2000);
    PurgeComm(serial_handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
#else
    serial_fd = open(puerto, O_RDWR | O_NOCTTY | O_SYNC);

    if (serial_fd < 0)
        return -1;

    if (configurarSerialPosix(serial_fd, baudrate) != 0)
        return -1;

    usleep(2000000);
    tcflush(serial_fd, TCIOFLUSH);
#endif

    modoConexion = MODO_SERIAL;
    return 0;
}

/* ============================================================
 * DISPATCH GENERICO
 * ============================================================ */

static int enviarComando(const char* comando, char* respuesta, size_t maxlen)
{
    if (modoConexion == MODO_TCP)
        return enviarComandoTCP(comando, respuesta, maxlen);

    if (modoConexion == MODO_SERIAL) {
#ifdef _WIN32
        return enviarComandoSerialWindows(comando, respuesta, maxlen);
#else
        return enviarComandoSerialPosix(comando, respuesta, maxlen);
#endif
    }

    return -1;
}

/* ============================================================
 * TECLADO PORTABLE
 * ============================================================ */

#ifdef _WIN32

int teclaDisponible(void)
{
    return _kbhit();
}

int leerTecla(void)
{
    return _getch();
}

void restaurarTerminal(void)
{
    /* No se necesita en Windows */
}

#else

static void configurarTerminalNoCanonica(void)
{
    if (terminalConfigurada)
        return;

    struct termios newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    terminalConfigurada = 1;
}

int teclaDisponible(void)
{
    configurarTerminalNoCanonica();

    struct timeval tv;
    fd_set fds;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

int leerTecla(void)
{
    configurarTerminalNoCanonica();

    unsigned char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);

    if (n == 1)
        return (int)c;

    return -1;
}

void restaurarTerminal(void)
{
    if (terminalConfigurada) {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        terminalConfigurada = 0;
    }
}

#endif

/* ============================================================
 * CIERRE
 * ============================================================ */

int cerrarConexion(void)
{
#ifdef _WIN32
    if (tcp_sockfd != INVALID_SOCKET) {
        closesocket(tcp_sockfd);
        tcp_sockfd = INVALID_SOCKET;
    }

    if (serial_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(serial_handle);
        serial_handle = INVALID_HANDLE_VALUE;
    }

    WSACleanup();
#else
    if (tcp_sockfd >= 0) {
        close(tcp_sockfd);
        tcp_sockfd = -1;
    }

    if (serial_fd >= 0) {
        close(serial_fd);
        serial_fd = -1;
    }
#endif

    modoConexion = MODO_NINGUNO;
    return 0;
}

/* ============================================================
 * PRIMITIVAS
 * ============================================================ */

int encenderBanda(void)
{
    char resp[RESP_SIZE];

    if (enviarComando("BANDA_ON", resp, sizeof(resp)) != 0)
        return -1;

    return (strcmp(resp, "OK") == 0) ? 0 : -1;
}

int apagarBanda(void)
{
    char resp[RESP_SIZE];

    if (enviarComando("BANDA_OFF", resp, sizeof(resp)) != 0)
        return -1;

    return (strcmp(resp, "OK") == 0) ? 0 : -1;
}

int leerColor(void)
{
    char resp[RESP_SIZE];

    if (enviarComando("LEER_COLOR", resp, sizeof(resp)) != 0)
        return COLOR_ERROR;

    if (strcmp(resp, "RED") == 0)
        return COLOR_RED;
    if (strcmp(resp, "GREEN") == 0)
        return COLOR_GREEN;
    if (strcmp(resp, "YELLOW") == 0)
        return COLOR_YELLOW;
    if (strcmp(resp, "SIN_OBJETO") == 0)
        return COLOR_SIN_OBJETO;

    return COLOR_ERROR;
}

int abrirPuerta(int puerta)
{
    char cmd[RESP_SIZE];
    char resp[RESP_SIZE];

    snprintf(cmd, sizeof(cmd), "ABRIR_PUERTA %d", puerta);

    if (enviarComando(cmd, resp, sizeof(resp)) != 0)
        return -1;

    return (strcmp(resp, "OK") == 0) ? 0 : -1;
}

int cerrarPuerta(int puerta)
{
    char cmd[RESP_SIZE];
    char resp[RESP_SIZE];

    snprintf(cmd, sizeof(cmd), "CERRAR_PUERTA %d", puerta);

    if (enviarComando(cmd, resp, sizeof(resp)) != 0)
        return -1;

    return (strcmp(resp, "OK") == 0) ? 0 : -1;
}

int cerrarPuertas(void)
{
    char resp[RESP_SIZE];

    if (enviarComando("CERRAR_PUERTAS", resp, sizeof(resp)) != 0)
        return -1;

    return (strcmp(resp, "OK") == 0) ? 0 : -1;
}