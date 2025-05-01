#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define FIXED_DIR "/Users/nastya/Documents/test_folder"

typedef struct file_info file_info;
typedef struct snapshot snapshot;

typedef struct file_info {
    ino_t inode;
    time_t creation_time;
    off_t size;
    struct file_info* next;
    char* name;
} file_info;

typedef struct snapshot {
    int snapshot_id;
    time_t timestamp;
    file_info* file_list;
    struct snapshot* next;
} snapshot;


file_info* create_file_info(const char* filename);
file_info* scan_directory(void);
void free_file_list(file_info* head);

snapshot* add_snapshot(snapshot* head, int* snapshot_counter);
void list_snapshots(snapshot* head);
snapshot* find_snapshot(snapshot* head, int id);
void print_snapshot_details(snapshot* snap);
snapshot* delete_snapshot(snapshot* head, int id);
void free_snapshots(snapshot* head);


file_info* create_file_info(const char* filename) {
    file_info* node = malloc(sizeof(file_info));
    if (!node) return NULL;

    char fullpath[512];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", FIXED_DIR, filename);

    struct stat st;
    if (stat(fullpath, &st) != 0) {
        free(node);
        return NULL;
    }

    node->name = strdup(filename);
    node->inode = st.st_ino;
    node->creation_time = st.st_mtime;
    node->size = st.st_size;
    node->next = NULL;

    return node;
}

file_info* scan_directory(void) {
    DIR* dir = opendir(FIXED_DIR);
    if (!dir) {
        perror("opendir failed");
        return NULL;
    }

    file_info* head = NULL;
    struct dirent* entry;

    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        file_info* new_node = create_file_info(entry->d_name);
        if (new_node) {
            new_node->next = head;
            head = new_node;
        }
    }

    closedir(dir);
    return head;
}

void free_file_list(file_info* head) {
    while (head) {
        file_info* temp = head;
        head = head->next;
        free(temp->name);
        free(temp);
    }
}

snapshot* add_snapshot(snapshot* head, int* snapshot_counter) {
    file_info* files = scan_directory();
    if (!files) {
        printf("Snapshot not taken. Directory is empty or does not exist.\n");
        return head;
    }

    snapshot* new_snap = malloc(sizeof(snapshot));
    if (!new_snap) return head;

    new_snap->snapshot_id = ++(*snapshot_counter);
    new_snap->timestamp = time(NULL);
    new_snap->file_list = files;
    new_snap->next = head;

    printf("Snapshot taken.\n");
    return new_snap;
}


void list_snapshots(snapshot* head) {
    if (!head) {
        printf("No snapshots available.\n");
        return;
    }

    while (head) {
        printf("Snapshot #%d - created at %s", head->snapshot_id, ctime(&head->timestamp));
        head = head->next;
    }
}

snapshot* find_snapshot(snapshot* head, int id) {
    while (head) {
        if (head->snapshot_id == id)
            return head;
        head = head->next;
    }
    return NULL;
}

void print_snapshot_details(snapshot* snap) {
    if (!snap) {
        printf("Snapshot not found.\n");
        return;
    }

    printf("Snapshot #%d - created at %s", snap->snapshot_id, ctime(&snap->timestamp));
    file_info* file = snap->file_list;
    while (file) {
        printf("  Name: %s\n", file->name);
        printf("  Inode: %lu\n", (unsigned long)file->inode);
        printf("  Last Modified: %s", ctime(&file->creation_time));
        printf("  Size: %ld bytes\n", (long)file->size);
        printf("  ------------------------\n");
        file = file->next;
    }
}

snapshot* delete_snapshot(snapshot* head, int id) {
    snapshot* current = head;
    snapshot* previous = NULL;

    while (current) {
        if (current->snapshot_id == id) {
            if (previous == NULL) {
                head = current->next;
            } else {
                previous->next = current->next;
            }

            free_file_list(current->file_list);
            free(current);
            printf("Snapshot #%d deleted successfully.\n", id);
            return head;
        }

        previous = current;
        current = current->next;
    }

    printf("Snapshot #%d not found.\n", id);
    return head;
}

void free_snapshots(snapshot* head) {
    while (head) {
        snapshot* temp = head;
        head = head->next;
        free_file_list(temp->file_list);
        free(temp);
    }
}

// ---------- MAIN PROGRAM ----------
int main() {
    snapshot* snapshots = NULL;
    int snapshot_counter = 0;
    char command;

    do {
        printf("\nChoose an operation:\n");
        printf("1 - Take snapshot of directory\n");
        printf("2 - List all snapshots\n");
        printf("3 - Show details of a snapshot\n");
        printf("4 - Delete a snapshot by ID\n");
        printf("q - Quit\n");
        printf(">> ");
        scanf(" %c", &command);
        getchar();

        switch (command) {
            case '1':
                snapshots = add_snapshot(snapshots, &snapshot_counter);
                printf("Snapshot taken.\n");
                break;
            case '2':
                list_snapshots(snapshots);
                break;
            case '3': {
                int id;
                printf("Enter snapshot ID: ");
                scanf("%d", &id);
                getchar();
                snapshot* snap = find_snapshot(snapshots, id);
                print_snapshot_details(snap);
                break;
            }
            case '4': {
                int id;
                printf("Enter snapshot ID to delete: ");
                scanf("%d", &id);
                getchar();
                snapshots = delete_snapshot(snapshots, id);
                break;
            }
            case 'q':
                printf("Exiting...\n");
                break;
            default:
                printf("Unknown command.\n");
        }

    } while (command != 'q');

    free_snapshots(snapshots);
    return 0;
}
