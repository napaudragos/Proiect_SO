#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

void handle_SIGTERM()
{
    printf("Shutting down after delay...\n");
    fflush(stdout);
    // Delay de 3 secunde
    Sleep(3000); // Sleep acceptă timpul în milisecunde pe Windows
    printf("Terminated.\n");
    exit(0); // Termină procesul
}

int main()
{
    // Simulăm primirea unui semnal SIGTERM
    printf("Press Enter to simulate SIGTERM...\n");
    getchar(); // Așteptăm ca utilizatorul să apese Enter

    handle_SIGTERM();

    return 0;
}
