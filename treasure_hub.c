#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <signal.h>

DWORD monitor_pid = -1;
HANDLE hMonitorProcess = NULL;

void startMonitor(void)
{
    // Crează un nou proces care va rula programul monitor
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Folosim CreateProcess pentru a lansa monitorul
    if (!CreateProcess(
            "monitor.exe",   // numele programului de lansat
            NULL,            // parametri (dacă sunt)
            NULL,            // atribut de securitate proces
            NULL,            // atribut de securitate fir
            FALSE,           // nu dorim să moștenim handle-urile
            0,               // niciun flag special
            NULL,            // variabile de mediu
            NULL,            // directorul curent
            &si,             // informații de startup
            &pi))            // informații de proces
    {
        printf("CreateProcess failed (%d).\n", GetLastError());
        exit(-1);
    }

    monitor_pid = pi.dwProcessId;
    hMonitorProcess = pi.hProcess;  // salvăm handle-ul procesului

    printf("Monitor started with PID %d\n", monitor_pid);
}

void handle_sigchld(int sig)
{
    // Înlocuim semnalizarea POSIX cu funcționalități Windows
    DWORD dwExitCode;
    if (monitor_pid != -1 && hMonitorProcess != NULL)
    {
        if (GetExitCodeProcess(hMonitorProcess, &dwExitCode))
        {
            if (dwExitCode != STILL_ACTIVE)
            {
                printf("Monitor process has exited.\n");
                monitor_pid = -1;
                CloseHandle(hMonitorProcess);
                hMonitorProcess = NULL;
            }
        }
    }
}

int main(void)
{
    char command[21];
    // Setăm handler-ul de semnale (Windows nu folosește semnalele POSIX)
    // Aici, putem să apelăm direct funcțiile de proces pentru a urmări terminația procesului
    // pentru Windows nu este nevoie de `sigaction`.

    while (1)
    {
        // Afișăm promptul și așteptăm input
        printf(">>> ");
        fflush(stdout);
        // Citim comanda de la utilizator
        if (!fgets(command, sizeof(command), stdin))
        {
            break;
        }
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "start_monitor") == 0)
        {
            if (monitor_pid != -1)
            {
                printf("Monitor already running with PID %d\n", monitor_pid);
            }
            else
            {
                startMonitor();
            }
        }
        else if (strcmp(command, "list_treasures") == 0)
        {
            if (monitor_pid == -1)
            {
                printf("Monitor not started.\n");
            }
            else
            {
                // Creăm fișierul pentru parametrii
                FILE *f = fopen("prm.txt", "w");
                if (!f)
                {
                    printf("Failed to open prm.txt\n");
                    exit(-1);
                }
                fprintf(f, "--list\n");
                fclose(f);
                // Trimitem un semnal (nu există semnal POSIX, dar putem folosi Windows signaling)
                // În acest exemplu, pur și simplu așteptăm că monitorul să fie gata.
            }
        }
        else if (strcmp(command, "stop_monitor") == 0)
        {
            if (monitor_pid == -1)
            {
                printf("Monitor not started.\n");
            }
            else
            {
                // Încercăm să terminăm procesul monitorului
                if (TerminateProcess(hMonitorProcess, 0) == 0)
                {
                    printf("Failed to stop the monitor.\n");
                }
                else
                {
                    printf("Monitor stopped.\n");
                }
                CloseHandle(hMonitorProcess);
                hMonitorProcess = NULL;
                monitor_pid = -1;
            }
        }
        else if (strcmp(command, "exit") == 0)
        {
            if (monitor_pid != -1)
            {
                printf("Monitor still running with PID %d, you need to stop it first.\n", monitor_pid);
            }
            else
            {
                break;
            }
        }
        else if (strcmp(command, "help") == 0)
        {
            printf("Available commands:\n");
            printf("  start_monitor     - Start the monitor process\n");
            printf("  list_hunts        - List all available hunts\n");
            printf("  list_treasures    - List treasures in a hunt\n");
            printf("  view_treasure     - View a specific treasure\n");
            printf("  stop_monitor      - Stop the monitor process\n");
            printf("  exit              - Exit this program (if monitor is stopped)\n");

        }
        else
        {
            printf("Unknown command. Type 'help' for list of commands.\n");
        }
    }

    return 0;
}
