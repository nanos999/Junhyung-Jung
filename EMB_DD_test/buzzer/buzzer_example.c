#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define BUZZER_DEVICE "/dev/buzzer"

int main() {
    int buzzer_fd;

    // Open the buzzer device file
    buzzer_fd = open(BUZZER_DEVICE, O_RDWR);
    if (buzzer_fd < 0) {
        perror("Failed to open the buzzer device file");
        return -1;
    }

    // Trigger the buzzer
    printf("Triggering the buzzer...\n");
    write(buzzer_fd, "1", 1);  // You can write different data depending on your driver's implementation

    // Wait for a moment
    sleep(2);

    // Stop the buzzer
    printf("Stopping the buzzer...\n");
    write(buzzer_fd, "0", 1);  // You can write different data depending on your driver's implementation

    // Close the buzzer device file
    close(buzzer_fd);

    return 0;
}
