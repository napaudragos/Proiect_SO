#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// Handler pentru "semnalul" de oprire
void handle_SIGTERM()
{
    printf("Shutting down after delay...\n");
    fflush(stdout);
    Sleep(3000); // Pauză de 3 secunde
    printf("Terminated.\n");
    exit(0);
}

// Handler pentru comenzi din prm.txt
void handle_monitor_command()
{
    FILE *f = fopen("prm.txt", "r");
    if (!f)
    {
        printf("Could not open prm.txt\n");
        return;
    }

    char args[3][100];
    int count = 0;

    while (fgets(args[count], sizeof(args[count]), f) && count < 3)
    {
        args[count][strcspn(args[count], "\n")] = '\0'; // Scoatem newline
        count++;
    }
    fclose(f);

    remove("prm.txt");

    if (count == 0)
    {
        printf("prm.txt is empty\n");
        return;
    }

    // Construim linia de comandă
    char commandLine[256] = "treasure_manager";
    for (int i = 0; i < count; ++i)
    {
        strcat(commandLine, " ");
        strcat(commandLine, args[i]);
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(
            NULL,          // Numele aplicației (NULL pentru a folosi commandLine)
            commandLine,   // Comandă cu parametri
            NULL, NULL, FALSE,
            0, NULL, NULL,
            &si, &pi))
    {
        printf("CreateProcess failed (%lu).\n", GetLastError());
        return;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

// Verifică dacă fișierul există
int file_exists(const char *filename)
{
    DWORD attrib = GetFileAttributesA(filename);
    return (attrib != INVALID_FILE_ATTRIBUTES &&
            !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}

// Funcția principală
int main(void)
{
    printf("Monitor started. Waiting for commands...\n");

    while (1)
    {
        if (file_exists("prm.txt"))
        {
            handle_monitor_command();
        }

        if (file_exists("stop.txt"))
        {
            // După procesare, poți și șterge stop.txt dacă vrei
            remove("stop.txt");
            handle_SIGTERM();
        }

        Sleep(1000); // Așteaptă 1 secundă între verificări
    }

    return 0;
}
