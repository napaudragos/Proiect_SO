#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>

pid_t monitor_pid = -1; // process id monitor (copil)
int pipe_fd[2] = {-1, -1}; // [0] citire, [1] scriere

void startMonitor(void)
{
    if (monitor_pid != -1) { //verifică dacă monitorul este deja pornit
        printf("Monitor already running with PID %d\n", monitor_pid);
        return;
    }

    if (pipe(pipe_fd) == -1) { //crează un pipe pt comunicare
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork(); //creaza procesul
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // Proces copil
        close(pipe_fd[0]); // copilul nu citește din pipe , inchide capătul de citire
        char fd_arg[16];
        snprintf(fd_arg, sizeof(fd_arg), "%d", pipe_fd[1]); //converteste pipe_fd[1] in string fd_arg
        execl("./monitor", "./monitor", fd_arg, NULL); //inlocuiește procesul copil cu monitor si transmite argumentul in linie de comanda pt cand se executa monitor
        perror("execl");
        exit(EXIT_FAILURE);
    } else {
        // Proces părinte
        close(pipe_fd[1]); // părintele nu scrie în pipe
        monitor_pid = pid;
        printf("Monitor started with PID %d\n", monitor_pid);
    }
}

void stopMonitor(void)
{
    if (monitor_pid == -1) { //verifică dacă monitorul este pornit
        printf("Monitor not started.\n");
        return;
    }
    kill(monitor_pid, SIGTERM); //transmite semnalul SIGTERM monitorului
    waitpid(monitor_pid, NULL, 0); //asteaptă terminarea procesului monitor
    monitor_pid = -1;
    printf("Monitor stopped.\n");
}

void checkMonitorStatus(void)
{
    if (monitor_pid != -1) { //vefifică dacă monitorul este pornit
        int status;
        pid_t result = waitpid(monitor_pid, &status, WNOHANG); //WNOGANG - permite să verificăm statusul fără a intarzia programul principal    
        if (result == -1) { //eroare 
            perror("waitpid");
            monitor_pid = -1;
        } else if (result > 0) { //monitorul s-a terminat , 0 - inca ruleaza
            printf("Monitor process has exited.\n");
            monitor_pid = -1;
        }
    }
}

void read_from_monitor() {
    char buffer[1024];
    ssize_t n;
    while ((n = read(pipe_fd[0], buffer, sizeof(buffer)-1)) > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
        if (n < (ssize_t)sizeof(buffer)-1) break;
    }
}

void calculate_score_all_hunts() {
    DIR *dir = opendir("."); //deschide directorul curent
    if (!dir) {
        perror("opendir");
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) { //parcurge directorul
        if (entry->d_type != DT_DIR) continue;
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue; //ignora . si ..

        char treasures_path[512];
        snprintf(treasures_path, sizeof(treasures_path), "%s/treasures.dat", entry->d_name); //construiecte calea catre fisierul treasures.dat

        if (access(treasures_path, F_OK) != 0) continue; //verifică dacă fișierul există

        int fd[2]; // [0] citire, [1] scriere
        if (pipe(fd) == -1) {
            perror("pipe");
            continue;
        }
        pid_t pid = fork();
        if (pid == 0) {
            // copil
            close(fd[0]); // închide capătul de citire
            dup2(fd[1], STDOUT_FILENO); // redirecționează stdout către capătul de scriere
            execl("./calculate_score", "./calculate_score", treasures_path, NULL); // execută programul calculate_score
            perror("execl");
            exit(1);
        } else if (pid > 0) {
            // părinte
            close(fd[1]);
            char buf[1024];
            ssize_t n;
            printf("Scores for hunt %s:\n", entry->d_name);
            while ((n = read(fd[0], buf, sizeof(buf)-1)) > 0) {
                buf[n] = 0;
                printf("%s", buf);
            }
            close(fd[0]);
            waitpid(pid, NULL, 0);
        }
    }
    closedir(dir);
}

int main(void)
{
    char command[64];

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
            startMonitor();
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
                kill(monitor_pid, SIGUSR1);
                read_from_monitor();
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
                kill(monitor_pid, SIGUSR1);
                read_from_monitor();
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
                kill(monitor_pid, SIGUSR1);
                read_from_monitor();
            }
        }
        else if (strcmp(command, "stop_monitor") == 0)
        {
            stopMonitor();
        }
        else if (strcmp(command, "calculate_score") == 0)
        {
            calculate_score_all_hunts();
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
            printf("  calculate_score   - Calculate user scores for all hunts\n");
            printf("  exit              - Exit this program (if monitor is stopped)\n");
        }
        else
        {
            printf("Unknown command. Type 'help' for list of commands.\n");
        }

        checkMonitorStatus();
    }

    return 0;
}