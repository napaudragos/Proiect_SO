#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

DWORD monitor_pid = -1;
HANDLE hMonitorProcess = NULL;

// Functie pentru a lansa procesul monitor
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

// Funcție pentru a verifica terminarea procesului monitor
void checkMonitorStatus(void)
{
    DWORD dwExitCode;
    if (monitor_pid != -1 && hMonitorProcess != NULL)
    {
        if (GetExitCodeProcess(hMonitorProcess, &dwExitCode))
        {
            if (dwExitCode != STILL_ACTIVE)
            {
                // Procesul monitor s-a terminat
                printf("Monitor process has exited.\n");
                monitor_pid = -1;
                CloseHandle(hMonitorProcess);  // Închidem handle-ul procesului
                hMonitorProcess = NULL;
            }
        }
    }
}

int main(void)
{
    char command[21];

    while (1)
    {
        printf(">>> ");
        fflush(stdout);
        if (!fgets(command, sizeof(command), stdin))
        {
            break;
        }
        command[strcspn(command, "\n")] = '\0';  // Eliminăm newline-ul de la finalul comenzii

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
                char hunt_id[100];
                printf("Enter hunt ID: ");
                fflush(stdout);
                fgets(hunt_id, sizeof(hunt_id), stdin);
                hunt_id[strcspn(hunt_id, "\n")] = '\0';

                FILE *f = fopen("prm.txt", "w");
                if (!f)
                {
                    printf("Failed to open prm.txt\n");
                    exit(-1);
                }

                fprintf(f, "--list\n%s\n", hunt_id);
                fclose(f);
                // Monitorul ar trebui să citească din fișierul prm.txt pentru a procesa comanda
            }
        }
        else if (strcmp(command, "view_treasure") == 0)
        {
            if (monitor_pid == -1)
            {
                printf("Monitor not started.\n");
            }
            else
            {
                char hunt_id[100], treasure_id[100];
                printf("Enter hunt ID: ");
                fflush(stdout);
                fgets(hunt_id, sizeof(hunt_id), stdin);
                hunt_id[strcspn(hunt_id, "\n")] = '\0';

                printf("Enter treasure ID: ");
                fflush(stdout);
                fgets(treasure_id, sizeof(treasure_id), stdin);
                treasure_id[strcspn(treasure_id, "\n")] = '\0';

                FILE *f = fopen("prm.txt", "w");
                if (!f)
                {
                    printf("Failed to open prm.txt");
                    exit(-1);
                }

                fprintf(f, "--view\n%s\n%s\n", hunt_id, treasure_id);
                fclose(f);
                // Monitorul ar trebui să citească din fișierul prm.txt pentru a procesa comanda
            }
        }
        else if (strcmp(command, "list_hunts") == 0)
        {
            if (monitor_pid == -1)
            {
                printf("Monitor not started.\n");
            }
            else
            {
                FILE *f = fopen("prm.txt", "w");
                if (!f)
                {
                    printf("Failed to open prm.txt");
                    exit(-1);
                }

                fprintf(f, "--list_hunts\n");
                fclose(f);
                // Monitorul ar trebui să citească din fișierul prm.txt pentru a procesa comanda
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
                // Încercăm să oprim procesul monitor
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
                break;  // Ieșim din program
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

        // Verificăm starea procesului monitor
        checkMonitorStatus();
    }

    return 0;
}
