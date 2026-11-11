
would it be good??
Yes. One C file with fork() is a good choice for Assignment 7. The assignment asks for two processes that communicate through two pipes. Your program creates the parent and child in a single run, so one source file is enough.

tell me how to run it and what to do after that
Open your Ubuntu terminal and go to the folder containing 7.c. If it’s in your practice folder, run:

cd ~/practice
gcc -Wall -Wextra 7.c -o assignment7

Then create a small text file and pass its name to the program:

printf 'Hello from Assignment 7\n' > sample.txt
./assignment7 sample.txt

You should see:

Hello from Assignment 7
Run ./assignment7 sample.txt once. The program creates the child with fork(). The parent sends sample.txt through the first pipe, the child reads the file and sends its contents through the second pipe, and the parent prints them.

What to do after it works
Try a missing file. Run ./assignment7 missing.txt. If that file doesn’t exist, you should see a No such file or directory error.
For your lab demo, show the contents of sample.txt with cat sample.txt, then run ./assignment7 sample.txt and show that it prints the same text. Explain what each pipe carries.
explain me the code line by line now
Let’s go through your shortened 7.c from top to bottom. Keep this example in mind as you read:

./assignment7 sample.txt

The parent sends the name sample.txt through the request pipe. The child opens that file and sends its contents through the response pipe. The parent prints what it receives.

1. Header files
Code	What it does
#define _POSIX_C_SOURCE 200809L	Makes POSIX functions such as getdelim(), dprintf(), and fdopen() available when compiling in strict C mode on Ubuntu. It goes before the headers.
#include <stdio.h>	Provides FILE, fopen(), fread(), fwrite(), fprintf(), and other input/output functions.
#include <stdlib.h>	Provides exit(), free(), EXIT_SUCCESS, and EXIT_FAILURE.
#include <sys/wait.h>	Provides waitpid() and the macros that inspect how the child finished.
#include <unistd.h>	Provides pipe(), fork(), and close().
2. fail() handles errors
Code	What it does
static void fail(const char *message)	Defines a helper that takes an error label. static makes it private to this source file.
perror(message);	Prints the label and the system’s error description to standard error. For example, missing.txt: No such file or directory.
exit(EXIT_FAILURE);	Ends whichever process called fail() with a failure status.
3. open_pipe() turns a pipe descriptor into a stream
Code	What it does
static FILE *open_pipe(int fd, const char *mode)	Takes a pipe file descriptor and a mode such as "r" for reading or "w" for writing.
FILE *stream = fdopen(fd, mode);	Wraps the already-open descriptor in a FILE *, so you can use functions like fread() and fwrite() on it.
if (stream == NULL)	Checks whether fdopen() failed.
fail("fdopen");	Prints the error and exits if the check above is true.
return stream;	Gives the caller the usable stream. Later, fclose() closes both the stream and its underlying pipe descriptor.
4. copy_data() copies bytes from one stream to another
This same function is used twice. The child copies file → response pipe, and the parent copies response pipe → screen.

Code	What it does
static void copy_data(FILE *input, FILE *output)	Takes the stream to read from and the stream to write to.
char buffer[4096];	Creates a temporary buffer that holds up to 4096 bytes at a time.
size_t count;	Stores how many bytes were read in the current iteration.
while ((count = fread(buffer, 1, sizeof buffer, input)) > 0)	Reads up to 4096 one-byte items into buffer. The loop continues whenever it gets at least one byte.
if (fwrite(buffer, 1, count, output) != count)	Writes exactly the number of bytes received and checks whether all of them were written.
fail("write");	Reports a write failure and exits.
if (ferror(input))	After the loop, checks whether reading stopped because of an error. fread() can return zero at normal end-of-file too.
fail("read");	Reports a read failure and exits.
if (fflush(output) == EOF)	Sends any bytes still held in the output stream’s buffer and checks whether that operation failed.
fail("flush");	Reports a failure while flushing the output.
For example, if the file has 5000 bytes, copy_data() can copy 4096 bytes in one iteration and the remaining 904 in the next.

5. main() checks the argument and creates both pipes
Code	What it does
int main(int argc, char *argv[])	Receives the command-line arguments. In ./assignment7 sample.txt, argc is 2, argv[0] is ./assignment7, and argv[1] is sample.txt.
int request[2], response[2], status;	Reserves two descriptors for each pipe. status will hold information about how the child finished.
if (argc != 2)	Checks that the user supplied exactly one pathname argument.
fprintf(stderr, "Usage: %s <filename>\n", argv[0]);	Prints how to run the program if the argument count is wrong.
return EXIT_FAILURE;	Stops the program in that case.
if (pipe(request) == -1 || pipe(response) == -1)	Creates the request and response pipes. A return value of -1 means the corresponding call failed.
fail("pipe");	Reports a pipe creation failure and exits.
pid_t child = fork();	Creates the child process. Both processes continue from this point, and both inherit the pipe descriptors.
if (child == -1)	Checks whether fork() failed.
fail("fork");	Reports the failure and exits.
For either pipe, [0] is the reading end and [1] is the writing end.

6. Child process receives the pathname and sends the contents
Code	What it does
if (child == 0)	Enters the child branch because fork() returns 0 in the child.
close(request[1]);	Closes the child’s copy of the request writing end. The child only reads from this pipe.
close(response[0]);	Closes the child’s copy of the response reading end. The child only writes to this pipe.
FILE *input = open_pipe(request[0], "r");	Opens the request reading end as a stream.
char *pathname = NULL;	Declares a pointer for the received pathname. getdelim() will allocate memory for it.
size_t capacity = 0;	Tracks how much memory is allocated for the pathname buffer. It starts at zero.
ssize_t length = getdelim(&pathname, &capacity, '\0', input);	Reads from the request pipe through the '\0' byte that the parent sends after the pathname. ssize_t can hold -1, the failure return value.
if (length <= 0 || pathname[length - 1] != '\0')	Checks whether reading failed or the received bytes were missing the terminating '\0'. The || short-circuits, so the code only checks pathname[length - 1] when length is positive.
fprintf(stderr, "Could not read the pathname\n");	Prints an error if the pathname was incomplete or unreadable.
free(pathname);	Releases any memory allocated for the pathname on that error path.
return EXIT_FAILURE;	Ends the child with a failure status.
fclose(input);	Closes the request stream after successfully reading the pathname.
FILE *file = fopen(pathname, "rb");	Opens the requested file for reading in binary mode, so the copy works with raw bytes.
if (file == NULL)	Checks whether opening the file failed.
fail(pathname);	Reports why the file could not be opened, using the pathname as the error label.
free(pathname);	Releases the pathname buffer after a successful open. The child now reads using file.
FILE *output = open_pipe(response[1], "w");	Opens the response writing end as a stream.
copy_data(file, output);	Reads the file in chunks and sends each chunk to the parent through the response pipe.
fclose(file);	Closes the requested file.
fclose(output);	Closes the child’s response writing end. Once the parent reads all remaining bytes, it can detect end-of-file.
return EXIT_SUCCESS;	Ends the child successfully.
7. Parent process sends the pathname and prints the reply
The parent skips the if (child == 0) block because fork() gives it the child’s positive PID.

Code	What it does
close(request[0]);	Closes the parent’s copy of the request reading end.
close(response[1]);	Closes the parent’s copy of the response writing end. This lets the parent detect end-of-file once the child closes its writing end and the remaining bytes have been read.
if (dprintf(request[1], "%s%c", argv[1], '\0') < 0)	Writes the pathname and then an actual '\0' byte to the request pipe. That byte tells the child where the pathname ends.
fail("Sending pathname");	Reports a failure if the write above failed.
close(request[1]);	Closes the request writing end after sending the pathname.
FILE *input = open_pipe(response[0], "r");	Opens the response reading end as a stream.
copy_data(input, stdout);	Reads the child’s reply and writes it to standard output, which is normally your terminal.
fclose(input);	Closes the response reading end after the transfer finishes.
if (waitpid(child, &status, 0) == -1)	Waits for this specific child and collects its termination status.
fail("waitpid");	Reports an error if waiting failed.
return WIFEXITED(status) ? WEXITSTATUS(status) : EXIT_FAILURE;	If the child exited normally, the parent returns the child’s exit code. If the child was terminated by a signal, the parent returns failure.
Why is waitpid() after copy_data()? If the parent waited first and the file was large enough to fill the pipe, the child could get stuck waiting for space while the parent waited for the child to finish. Reading the pipe first lets the child keep sending data.

what is the child process in this case??
The child process is the new process created when your program calls fork(). It runs the same compiled program as the parent, but fork() returns 0 in the child, so it enters this block:

if (child == 0) {
    // Child process runs this block
}

When you run ./assignment7 sample.txt, the child receives sample.txt from the first pipe, opens the file, and sends its contents back through the second pipe. The parent prints those contents.

You run ./assignment7 once. fork() creates the child automatically.

i meant that what is it doing??
The child’s job is to read the requested file and send its contents back to the parent.

If you run ./assignment7 sample.txt and the file contains Hello, here’s what happens:

The child receives the filename sample.txt from the first pipe.
It opens sample.txt and reads the contents Hello.
It writes Hello to the second pipe, closes its writing end, and exits.
The parent reads Hello from the second pipe and prints it on your terminal.





#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static void fail(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

static FILE *open_pipe(int fd, const char *mode)
{
    FILE *stream = fdopen(fd, mode);
    if (stream == NULL)
        fail("fdopen");
    return stream;
}

static void copy_data(FILE *input, FILE *output)
{
    char buffer[4096];
    size_t count;

    while ((count = fread(buffer, 1, sizeof buffer, input)) > 0) {
        if (fwrite(buffer, 1, count, output) != count)
            fail("write");
    }

    if (ferror(input))
        fail("read");
    if (fflush(output) == EOF)
        fail("flush");
}

int main(int argc, char *argv[])
{
    int request[2], response[2], status;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (pipe(request) == -1 || pipe(response) == -1)
        fail("pipe");

    pid_t child = fork();
    if (child == -1)
        fail("fork");

    if (child == 0) {
        close(request[1]);
        close(response[0]);

        FILE *input = open_pipe(request[0], "r");
        char *pathname = NULL;
        size_t capacity = 0;
        ssize_t length = getdelim(&pathname, &capacity, '\0', input);
        if (length <= 0 || pathname[length - 1] != '\0') {
            fprintf(stderr, "Could not read the pathname\n");
            free(pathname);
            return EXIT_FAILURE;
        }
        fclose(input);

        FILE *file = fopen(pathname, "rb");
        if (file == NULL)
            fail(pathname);
        free(pathname);

        FILE *output = open_pipe(response[1], "w");
        copy_data(file, output);
        fclose(file);
        fclose(output);
        return EXIT_SUCCESS;
    }

    close(request[0]);
    close(response[1]);

    if (dprintf(request[1], "%s%c", argv[1], '\0') < 0)
        fail("Sending pathname");
    close(request[1]);

    FILE *input = open_pipe(response[0], "r");
    copy_data(input, stdout);
    fclose(input);

    if (waitpid(child, &status, 0) == -1)
        fail("waitpid");
    return WIFEXITED(status) ? WEXITSTATUS(status) : EXIT_FAILURE;
}

