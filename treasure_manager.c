#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir _mkdir
#else
    #include <unistd.h>
#endif
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

// Function declarations
void add_treasure(const char *hunt_id);
void list_treasures(const char *hunt_id);
void view_treasure(const char *hunt_id, int treasure_id);
void remove_treasure(const char *hunt_id, int treasure_id);
void remove_hunt(const char *hunt_id);
void log_operation(const char *hunt_id, const char *message);
void create_symlink(const char *hunt_id);
void list_hunts();

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
        exit(-1);
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
        view_treasure(argv[2], atoi(argv[3]));
    }
    else if (strcmp(argv[1], "--remove_treasure") == 0)
    {
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
        exit(-1);
    }
    return 0;
}

void log_operation(const char *hunt_id, const char *message) {
    char log_path[MAX_PATHH];
    snprintf(log_path, sizeof(log_path), "%s/logged_hunt", hunt_id);

    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
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

    snprintf(target, sizeof(target), "%s/logged_hunt", hunt_id);
    snprintf(link_name, sizeof(link_name), "logged_hunt-%s", hunt_id);


    DWORD attrs = GetFileAttributesA(target);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        fprintf(stderr, "Target '%s' does not exist\n", target);
        return;
    }

    DeleteFileA(link_name);

    DWORD flags = (attrs & FILE_ATTRIBUTE_DIRECTORY)
                  ? SYMBOLIC_LINK_FLAG_DIRECTORY
                  : 0;

    if (!CreateSymbolicLinkA(link_name, target, flags)) {
        DWORD err = GetLastError();
        if (err == 1314) {
            fprintf(stderr, "Error 1314: Run terminal as Administrator or enable Developer Mode in Windows Settings.\n");
        } else {
            fprintf(stderr, "Failed to create symlink (Windows): Error %lu\n", err);
        }
    } else {
        printf("Symlink created: %s → %s\n", link_name, target);
    }


}

void add_treasure(const char* hunt_id) {
    char dir_path[MAX_PATHH];
    char file_path[MAX_PATHH];
    char log_path[MAX_PATHH];
    Treasure t;

    snprintf(dir_path, sizeof(dir_path), "%s", hunt_id);
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", dir_path);
    snprintf(log_path, sizeof(log_path), "%s/logged_hunt", dir_path);

    if (mkdir(dir_path) == -1 && errno != EEXIST) {
        perror("mkdir");
        exit(EXIT_FAILURE);
    }

    printf("Enter Treasure ID: ");
    scanf("%d", &t.treasure_id);
    getchar();

    printf("Enter Username: ");
    fgets(t.username, USERNAME_MAX, stdin);
    t.username[strcspn(t.username, "\n")] = 0;

    printf("Enter Latitude: ");
    scanf("%f", &t.latitude);

    printf("Enter Longitude: ");
    scanf("%f", &t.longitude);
    getchar();

    printf("Enter Clue: ");
    fgets(t.clue, CLUE_MAX, stdin);
    t.clue[strcspn(t.clue, "\n")] = 0;

    printf("Enter Value: ");
    scanf("%d", &t.value);

    int fd = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("open (treasures.dat)");
        exit(EXIT_FAILURE);
    }

    if (write(fd, &t, sizeof(Treasure)) != sizeof(Treasure)) {
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
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", hunt_id);

    struct stat st;
    if (stat(file_path, &st) == -1) {
        perror("stat");
        return;
    }

    printf("Hunt: %s\nFile Size: %ld bytes\nLast Modified: %s", hunt_id, st.st_size, ctime(&st.st_mtime));

    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("open (treasures.dat)");
        return;
    }

    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        printf("\nTreasure ID: %d\nUsername: %s\nCoordinates: %.2f, %.2f\nClue: %s\nValue: %d\n",
               t.treasure_id, t.username, t.latitude, t.longitude, t.clue, t.value);
    }

    close(fd);
}

void view_treasure(const char *hunt_id, int treasure_id) {
    char file_path[MAX_PATHH];
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", hunt_id);

    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("open (treasures.dat)");
        return;
    }

    Treasure t;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.treasure_id == treasure_id) {
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
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", hunt_id);

    int fd = open(file_path, O_RDWR);
    if (fd == -1) {
        perror("open (treasures.dat)");
        return;
    }

    char temp_path[MAX_PATHH];
    snprintf(temp_path, sizeof(temp_path), "%s/temp_treasures.dat", hunt_id);
    int temp_fd = open(temp_path, O_WRONLY | O_CREAT, 0644);
    if (temp_fd == -1) {
        perror("open (temp file)");
        close(fd);
        return;
    }

    Treasure t;
    int treasure_found = 0;
    while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.treasure_id == treasure_id) {
            treasure_found = 1;
            continue;
        }
        write(temp_fd, &t, sizeof(Treasure));
    }

    if (!treasure_found) {
        printf("Treasure with ID %d not found.\n", treasure_id);
    } else {
        printf("Treasure with ID %d removed successfully.\n", treasure_id);
        remove(file_path);
        rename(temp_path, file_path);
    }

    close(fd);
    close(temp_fd);
}

void remove_hunt(const char *hunt_id) {
    char file_path[MAX_PATHH];
    const char *files[] = {
        "treasures.dat", "logged_hunt", "temp_treasures.dat"
    };

    for (int i = 0; i < sizeof(files)/sizeof(files[0]); i++) {
        snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, files[i]);
        unlink(file_path);
    }

    if (rmdir(hunt_id) == -1) {
        perror("rmdir");
        return;
    }

    char link_name[MAX_PATHH];
    snprintf(link_name, sizeof(link_name), "logged_hunt-%s", hunt_id);
    if (unlink(link_name) == -1 && errno != ENOENT) {
        perror("unlink (symlink)");
    }

    printf("Hunt '%s' removed.\n", hunt_id);
}

void list_hunts()
{
    // Open the current directory
    // and check if it was opened successfully
    // Check if the directory exists
    DIR *dir = opendir(".");
    if (!dir)
    {
        printf("Error opening the directory\n");
        exit(-1);
    }
    struct dirent *entry;
    // Read the directory entries
    // and check if it was read successfully
    // Loop through the directory entries
    // and check if the entry is a directory
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        struct stat st;
        if (stat(entry->d_name, &st) == -1 || !S_ISDIR(st.st_mode))
        {
            continue;
        }
        // Check if the directory contains a treasure file
        char path[50];
        snprintf(path, sizeof(path), "%s/treasures.dat", entry->d_name);
        // Check if the file exists
        int fd = open(path, O_RDONLY);
        if (fd < 0)
        {
            continue;
        }
        Treasure t;
        int count = 0;
        ssize_t r;
        // Read the treasures from the file
        // and count the number of treasures
        while ((r = read(fd, &t, sizeof(Treasure))) == sizeof(Treasure))
        {
            count++;
        }
        // If r is -1, it means there was an error reading the file
        if (r == -1)
        {
            perror("Error reading treasure file");
            close(fd);
            exit(-1);
        }
        if (close(fd) < 0)
        {
            printf("Error closing the file\n");
            exit(-1);
        }
        printf("Hunt: %s -> %d treasure%s\n", entry->d_name, count, count == 1 ? "" : "s");
    }
    closedir(dir);
}