#include <unistd.h> // Header for POSIX system calls
#include <string.h> // Header for strlen

int main() {
    char *message = "Hello from the Linux Kernel!\n";
    /* The write() system call parameters:
       1. File Descriptor (1 is stdout / the terminal)
       2. Pointer to the message buffer
       3. Number of bytes to write
    */
    write(1, message, strlen(message));

    return 0;
}
