#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#define TEST_DIR "test_filesystem"
#define TEST_FILE "test_filesystem/data.txt"
#define TEMP_DIR "test_filesystem_tmp"
#define MAX_ATTEMPTS 10
#define SLEEP_INTERVAL 2

void create_test_environment() {
    // Create test directory
    if (mkdir(TEST_DIR, 0755) == -1 && errno != EEXIST) {
        fprintf(stderr, "Error creating directory %s: %s\n", TEST_DIR, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Create test file with some content
    int fd = open(TEST_FILE, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) {
        fprintf(stderr, "Error creating file %s: %s\n", TEST_FILE, strerror(errno));
        exit(EXIT_FAILURE);
    }
    const char *content = "Test data for filesystem simulation\n";
    if (write(fd, content, strlen(content)) == -1) {
        fprintf(stderr, "Error writing to file %s: %s\n", TEST_FILE, strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
    printf("Test environment created: %s\n", TEST_FILE);
}

void simulate_filesystem_loss() {
    // Simulate loss by renaming the directory
    if (rename(TEST_DIR, TEMP_DIR) == -1) {
        fprintf(stderr, "Error simulating filesystem loss (rename %s to %s): %s\n",
                TEST_DIR, TEMP_DIR, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Filesystem loss simulated: %s is unavailable\n", TEST_DIR);
}

void simulate_filesystem_recovery() {
    // Simulate recovery by renaming the directory back
    if (rename(TEMP_DIR, TEST_DIR) == -1) {
        fprintf(stderr, "Error simulating filesystem recovery (rename %s to %s): %s\n",
                TEMP_DIR, TEST_DIR, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Filesystem recovered: %s is available\n", TEST_DIR);
}

int attempt_file_access() {
    // Try to open and read the test file
    int fd = open(TEST_FILE, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Failed to access file %s: %s\n", TEST_FILE, strerror(errno));
        return -1;
    }

    char buffer[256];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        fprintf(stderr, "Error reading file %s: %s\n", TEST_FILE, strerror(errno));
        close(fd);
        return -1;
    }

    buffer[bytes_read] = '\0';
    printf("Successfully read from file: %s", buffer);
    close(fd);
    return 0;
}

void cleanup() {
    // Remove test file and directory
    if (unlink(TEST_FILE) == -1 && errno != ENOENT) {
        fprintf(stderr, "Error removing file %s: %s\n", TEST_FILE, strerror(errno));
    }
    if (rmdir(TEST_DIR) == -1 && errno != ENOENT) {
        fprintf(stderr, "Error removing directory %s: %s\n", TEST_DIR, strerror(errno));
    }
    if (rmdir(TEMP_DIR) == -1 && errno != ENOENT) {
        fprintf(stderr, "Error removing temporary directory %s: %s\n", TEMP_DIR, strerror(errno));
    }
    printf("Cleanup completed\n");
}

int main() {
    printf("Starting filesystem loss and recovery simulation\n");

    // Step 1: Create test environment
    create_test_environment();

    // Step 2: Attempt file access before loss
    printf("Attempting file access before simulating loss...\n");
    if (attempt_file_access() == 0) {
        printf("File access successful before loss\n");
    }

    // Step 3: Simulate filesystem loss
    simulate_filesystem_loss();

    // Step 4: Attempt file access during loss
    printf("Attempting file access during simulated loss...\n");
    int attempt_count = 0;
    while (attempt_count < MAX_ATTEMPTS) {
        if (attempt_file_access() == 0) {
            printf("Unexpected file access success during loss\n");
            break;
        }
        printf("Retrying in %d seconds... (Attempt %d/%d)\n",
               SLEEP_INTERVAL, attempt_count + 1, MAX_ATTEMPTS);
        sleep(SLEEP_INTERVAL);
        attempt_count++;
    }

    // Step 5: Simulate filesystem recovery
    simulate_filesystem_recovery();

    // Step 6: Attempt file access after recovery
    printf("Attempting file access after recovery...\n");
    if (attempt_file_access() == 0) {
        printf("File access successful after recovery\n");
    } else {
        printf("File access failed after recovery\n");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Step 7: Cleanup
    cleanup();
    printf("Simulation completed successfully\n");
    return 0;
}
