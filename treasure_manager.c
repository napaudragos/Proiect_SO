#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>


#define USERNAME_MAX 32
#define CLUE_MAX 256
#define MAX_PATHH 512

typedef struct {
    int treasure_id;
    char username[USERNAME_MAX];
    float latitude;
    float longitude;
    char clue[CLUE_MAX];
    int value;
} Treasure;





void log_operation(const char *hunt_id, const char *message) {
    char log_path[MAX_PATHH];
    snprintf(log_path, sizeof(log_path), "%s/logged_hunt", hunt_id); //construieste calea catre fisierul de log

    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644); //deschide sau creaza fisierul de log
    if (fd == -1) {
        perror("open (logged_hunt)");
        return;
    }

    write(fd, message, strlen(message));
    close(fd);
}

void create_symlink(const char *hunt_id) {
    char target[MAX_PATHH];
    char link_name[MAX_PATHH];

    snprintf(target, sizeof(target), "%s/logged_hunt", hunt_id); // target = hunt_id/logged_hunt
    snprintf(link_name, sizeof(link_name), "logged_hunt-%s", hunt_id); // link_name = logged_hunt-hunt_id

    unlink(link_name); // sterge legatura veche
    if (symlink(target, link_name) == -1) {
        perror("symlink");
    } else {
        printf("Symlink created: %s → %s\n", link_name, target);
    }
}

void add_treasure(const char* hunt_id) {
    char dir_path[MAX_PATHH];  // numele directorului
    char file_path[MAX_PATHH]; // calea comorilor
    Treasure t;

    snprintf(dir_path, sizeof(dir_path), "%s", hunt_id); // dir_path = hunt_id

    if (snprintf(file_path, sizeof(file_path), "%s/treasures.dat", dir_path) >= (int)sizeof(file_path)) //construieste file_path
    {
        fprintf(stderr, "File path too long\n");  //verifica dimensinea bufferului
        exit(EXIT_FAILURE);
    }

    if (mkdir(dir_path, 0755) == -1 && errno != EEXIST) {
        perror("mkdir");
        exit(EXIT_FAILURE);
    }

    printf("Enter Treasure ID: ");
    if (scanf("%d", &t.treasure_id) != 1) {
        fprintf(stderr, "Invalid input for Treasure ID\n");
        exit(EXIT_FAILURE);
    }
    getchar();

    printf("Enter Username: ");
    if (!fgets(t.username, USERNAME_MAX, stdin)) {
        fprintf(stderr, "Error reading username\n");
        exit(EXIT_FAILURE);
    }
    t.username[strcspn(t.username, "\n")] = 0;

    printf("Enter Latitude: ");
    if (scanf("%f", &t.latitude) != 1) {
        fprintf(stderr, "Invalid input for Latitude\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter Longitude: ");
    if (scanf("%f", &t.longitude) != 1) {
        fprintf(stderr, "Invalid input for Longitude\n");
        exit(EXIT_FAILURE);
    }
    getchar();

    printf("Enter Clue: ");
    if (!fgets(t.clue, CLUE_MAX, stdin)) {
        fprintf(stderr, "Error reading clue\n");
        exit(EXIT_FAILURE);
    }
    t.clue[strcspn(t.clue, "\n")] = 0;

    printf("Enter Value: ");
    if (scanf("%d", &t.value) != 1) {
        fprintf(stderr, "Invalid input for Value\n");
        exit(EXIT_FAILURE);
    }
    getchar();

    int fd = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644); //deschide sau creaza fisierul 
    if (fd == -1) {
        perror("open (treasures.dat)");
        exit(EXIT_FAILURE);
    }

    if (write(fd, &t, sizeof(Treasure)) != sizeof(Treasure)) { //scrie in fisier
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);

    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Added treasure ID %d by user %s\n", t.treasure_id, t.username);
    log_operation(hunt_id, log_msg);
    create_symlink(hunt_id);
    printf("Treasure added successfully.\n");
}

void list_treasures(const char *hunt_id) {
    char file_path[MAX_PATHH];
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", hunt_id); //construieste calea fisierului

    struct stat st;
    if (stat(file_path, &st) == -1) { //verifica daca fisierul exista inainte de a-l deschide 
        perror("stat");
        return;
    }

    printf("Hunt: %s\nFile Size: %ld bytes\nLast Modified: %s", hunt_id, st.st_size, ctime(&st.st_mtime));

    int fd = open(file_path, O_RDONLY); //deschide fisierul doar pt citire
    if (fd == -1) {
        perror("open (treasures.dat)");
        return;
    }

    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) { //afiseaza continutul fisierului
        printf("\nTreasure ID: %d\nUsername: %s\nCoordinates: %.2f, %.2f\nClue: %s\nValue: %d\n",
               t.treasure_id, t.username, t.latitude, t.longitude, t.clue, t.value);
    }

    close(fd);
}

void view_treasure(const char *hunt_id, int treasure_id) {
    char file_path[MAX_PATHH]; 
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", hunt_id); //construieste calea fisierului

    int fd = open(file_path, O_RDONLY); //deschide fisierul doar pt citire
    if (fd == -1) {
        perror("open (treasures.dat)");
        return;
    }

    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) { // parcurge fisierul
        if (t.treasure_id == treasure_id) { //cauta comora cu id-ul dat
            printf("Treasure ID: %d\nUsername: %s\nCoordinates: %.2f, %.2f\nClue: %s\nValue: %d\n",
                   t.treasure_id, t.username, t.latitude, t.longitude, t.clue, t.value);
            close(fd);
            return;
        }
    }

    printf("Treasure with ID %d not found.\n", treasure_id);
    close(fd);
}

void remove_treasure(const char *hunt_id, int treasure_id) {
    char file_path[MAX_PATHH];
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", hunt_id); //construieste calea fisierului

    int fd = open(file_path, O_RDWR);  //deschide fisierul pentru citire si scriere
    if (fd == -1) {
        perror("open (treasures.dat)");
        return;
    }

    char temp_path[MAX_PATHH];
    snprintf(temp_path, sizeof(temp_path), "%s/temp_treasures.dat", hunt_id); // construieste calea fisierului temporar

    int temp_fd = open(temp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644); // deschide sau creaza fisierul temporar 
    if (temp_fd == -1) {
        perror("open (temp file)"); //daca e eroare inchide si fisierul principal
        close(fd);
        return;
    }

    Treasure t;
    int treasure_found = 0;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) { //parcurge fisierul
        if (t.treasure_id == treasure_id) {
            treasure_found = 1;
            continue; //sare peste scrierea comorii gasite in fisierul temporar
        }
        if (write(temp_fd, &t, sizeof(Treasure)) != sizeof(Treasure)) { //scrie in fisierul temporar
            perror("write (temp file)");
            close(fd);
            close(temp_fd);
            return;
        }
    }

    close(fd);
    close(temp_fd);

    if (!treasure_found) {
        printf("Treasure with ID %d not found.\n", treasure_id);
        unlink(temp_path); //sterge fisierul temporar
    } else {
        printf("Treasure with ID %d removed successfully.\n", treasure_id);
        unlink(file_path); //sterge fisierul original
        rename(temp_path, file_path); //il inlocuieste cu fisierul temporar
    }
}

void remove_hunt(const char *hunt_id) {
    char file_path[MAX_PATHH];
    const char *files[] = { //un fel de enum pentru fisiere
        "treasures.dat", "logged_hunt", "temp_treasures.dat"
    };

    for (int i = 0; i < sizeof(files)/sizeof(files[0]); i++) { //sizeof(files)/sizeof(files[0]) returneaza numarul de fisiere din director
        snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, files[i]); //creaza calea catre fiecare fisier care urmeaza a fi sters
        unlink(file_path); // incearca sa stearga fisierul
    }

    if (rmdir(hunt_id) == -1) { //sterge directorul
        perror("rmdir");
        return;
    }

    char link_name[MAX_PATHH]; //numere symlinkul
    snprintf(link_name, sizeof(link_name), "logged_hunt-%s", hunt_id); //creaza numele symlinkului
    if (unlink(link_name) == -1 && errno != ENOENT) { //sterge symlinkul , ignora eroarea daca nu exista
        perror("unlink (symlink)");
    }

    printf("Hunt '%s' removed.\n", hunt_id);
}

void list_hunts()
{
    DIR *dir = opendir("."); // deschide directorul curent
    if (!dir)
    {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    struct dirent *entry; // citeste intrarile din director
    while ((entry = readdir(dir)) != NULL) //parcurge directorul
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue; //ignora . si .. 
        }

        struct stat st;
        if (stat(entry->d_name, &st) == -1 || !S_ISDIR(st.st_mode)) //obtine informatii despre intrare si verifica daca e director
        {
            continue;
        }

        char path[MAX_PATHH];
        snprintf(path, sizeof(path), "%s/treasures.dat", entry->d_name); //construiecte calea catre fisierul treasures.dat
        int fd = open(path, O_RDONLY);
        if (fd < 0)
        {
            continue;
        }
        Treasure t;
        int count = 0;
        ssize_t r;
        while ((r = read(fd, &t, sizeof(Treasure))) == sizeof(Treasure)) //numara cate comori sunt in fisier
        {
            count++;
        }

        if (r == -1) //nu s-a putut citi fisierul
        {
            perror("Error reading treasure file");
            close(fd);
            closedir(dir);
            exit(EXIT_FAILURE);
        }
        
        if (close(fd) < 0)
        {
            perror("close");
            closedir(dir);
            exit(EXIT_FAILURE);
        }
        printf("Hunt: %s -> %d treasure%s\n", entry->d_name, count, count == 1 ? "" : "s");
    }
    closedir(dir);
}

int main(int argc, char **argv)
{
    if (argc < 2 || (strcmp(argv[1], "--list_hunts") != 0 && argc < 3))
    {
        fprintf(stderr, "Usage:\n"
                        "--add <hunt_id>\n"
                        "--list <hunt_id>\n"
                        "--view <hunt_id> <treasure_id>\n"
                        "--remove_treasure <hunt_id> <treasure_id>\n"
                        "--remove_hunt <hunt_id>\n"
                        "--list_hunts\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(argv[1], "--add") == 0)
    {
        add_treasure(argv[2]);
    }
    else if (strcmp(argv[1], "--list") == 0)
    {
        list_treasures(argv[2]);
    }
    else if (strcmp(argv[1], "--view") == 0)
    {
        if (argc < 4) {
            fprintf(stderr, "Missing treasure_id for --view\n");
            exit(EXIT_FAILURE);
        }
        view_treasure(argv[2], atoi(argv[3]));
    }
    else if (strcmp(argv[1], "--remove_treasure") == 0)
    {
        if (argc < 4) {
            fprintf(stderr, "Missing treasure_id for --remove_treasure\n");
            exit(EXIT_FAILURE);
        }
        remove_treasure(argv[2], atoi(argv[3]));
    }
    else if (strcmp(argv[1], "--remove_hunt") == 0)
    {
        remove_hunt(argv[2]);
    }
    else if (strcmp(argv[1], "--list_hunts") == 0)
    {
        list_hunts();
    }
    else
    {
        fprintf(stderr, "Invalid command\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}
