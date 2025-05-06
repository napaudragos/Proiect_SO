#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <signal.h>

DWORD monitor_pid = -1;
HANDLE hMonitorProcess = NULL;

// Pornim procesul monitor
void startMonitor(void)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Creăm procesul monitor folosind CreateProcess
    if (!CreateProcess(
            "monitor.exe",   // Numele programului de monitorizat
            NULL,            // Parametrii (nu sunt necesari aici)
            NULL,            // Atribute de securitate proces
            NULL,            // Atribute de securitate fir
            FALSE,           // Nu dorim să moștenim handle-urile
            0,               // Fără flaguri speciale
            NULL,            // Variabile de mediu
            NULL,            // Directorul curent
            &si,             // Informațiile pentru crearea procesului
            &pi))            // Informațiile procesului creat
    {
        printf("CreateProcess failed (%d).\n", GetLastError());
        exit(-1);
    }

    monitor_pid = pi.dwProcessId;  // Salvăm PID-ul procesului monitor
    hMonitorProcess = pi.hProcess;  // Salvăm handle-ul procesului

    printf("Monitor started with PID %d\n", monitor_pid);
}

int main()
{

    
}