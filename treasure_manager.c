#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <direct.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct 
{
    int treasure_id;
    char user_name[50];
    float coord_x, coord_y;
    char clue_text[100];
    int value;
} Treasure;

void add(int hunt_id, Treasure new_treasure) {
    char dir_name[50];
    snprintf(dir_name, sizeof(dir_name), "hunt_%d", hunt_id);

    if (_mkdir(dir_name) != 0) {
        if (errno != EEXIST) {
            perror("Error creating directory");
            return;
        }
    }

    char file_path[100];
    snprintf(file_path, sizeof(file_path), "%s\\treasure.txt", dir_name);

    int fd = _open(file_path, _O_WRONLY | _O_CREAT | _O_APPEND, _S_IREAD | _S_IWRITE);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }

    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "Treasure ID: %d\nUser Name: %s\nCoordinates: (%.2f, %.2f)\nClue: %s\nValue: %d\n\n",
             new_treasure.treasure_id,
             new_treasure.user_name,
             new_treasure.coord_x,
             new_treasure.coord_y,
             new_treasure.clue_text,
             new_treasure.value);

    if (_write(fd, buffer, strlen(buffer)) == -1) {
        perror("Error writing to file");
        _close(fd);
        return;
    }

    _close(fd);
}

int main() {
    Treasure t1 = {12, "Raian", 12.34, 56.78, "Follow the river to the old oak tree.", 100};
    add(111, t1);
    return 0;
}
