/*==========================================================
| You have been provided with a program called hackme. To successfully hack the program, you need to provide the correct password. A random password would be impossible to guess.
| Usage: ./assignment-1 [-f] [-p] (arguments are optional)
| Description of variables:
|   f: fork mode
|   p: run hackme after all done
|   password: the password to be cracked
|   partition: the password to be cracked in a partition
|   p: the index of first character of the partition
|   id: the id of the process
|   gid: the id of the grandchild process
|   fd: the file descriptor of the pipe for child1
|   fd1: the file descriptor of the pipe for child2
|   fd2: the file descriptor of the pipe for gChild1
|   child1: the id of the first child process
|   child2: the id of the second child process
|   gChild1: the id of the grandchild process
|   buffer: the buffer to store the password
|   DEBUG: the debug mode
|----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "checkPassword.h"
#define DEBUG 0

// Function prototypes
char *bruteForce(int p);
int handleBruteForce(char *password);
int forkMode(char *password);

// main() handles the arguments and calls forkMode() or handleBruteForce() to crack the password.
int main(int argc, char *argv[])
{
  int f = 0, p = 0;
  char *password = malloc(13);
  if (argc > 3)
  {
    printf("Invalid arguments.\nUsage: %s [-f] [-p] (arguments are optional)\n", __FILE__);
    return 1;
  }
  else if (argc > 1)
  {
    for (int i = 1; i < argc; i++)
    {
      if (!f && strcmp(argv[i], "-f") == 0)
      {
#if DEBUG
        printf("Fork is enabled.\n");
#endif
        f++;
      }
      if (!p && strcmp(argv[i], "-p") == 0)
      {
#if DEBUG
        printf("Hackme will be run automatically after all done.\n");
#endif
        p++;
      }
    }
  }
  if (f)
  {
    forkMode(password);
  }
  else
  {
    handleBruteForce(password);
  }
  if (password)
  {
    printf("The password is %s\n", password);
    if (p && execlp("./hackme", "", NULL) == -1)
    {
      printf("Error: Cannot run hackme\n");
      return -1;
    }
    return 0;
  }
  return -1;
}

// bruteForce() runs checkPassword() by given partition and returns the password if found.
char *bruteForce(int p)
{
  static char buffer[4]; // Initial with three asscii 33
  buffer[0] = buffer[1] = buffer[2] = 33;
  while (buffer[0] != 127)
  {
    if (!checkPassword(buffer, p))
    {
      return buffer;
    }
    // When the last character reaches 126
    if (buffer[2] == 126)
    {
      buffer[2] = 32;
      // When the last characters reach 126
      if (buffer[1] == 126)
      {
        buffer[1] = 33;
        buffer[0]++;
      }
      else
      {
        buffer[1]++;
      }
    }
    // Increment the last character
    buffer[2]++;
  }
  return NULL;
}

// handleBruteForce() calls bruteForce() to crack every 3 characters and return -1 if no password found.
int handleBruteForce(char *password)
{
  int p = 0;
  char *partition;
  //
  while (p < 12)
  {
    partition = bruteForce(p);
    if (partition)
    {
      strcat(password, partition);
      printf("%s\n", partition);
    }
    else
    {
      printf("No password found\n");
      return -1;
    }
    p += 3;
  }
  return 0;
}

// forkMode() forks a child process to run bruteForce() and wait for the child process to finish.
int forkMode(char *password)
{
  pid_t child1, child2, gChild1;
  int fd[2], fd1[2], fd2[2];
  // The usage of pipe() is referenced from StackOverflow
  // https://stackoverflow.com/questions/49581349/how-to-get-return-value-from-child-process-to-parent
  double id = 1.0;
  char *partition, gid[10];
  // Create three pipes
  if (pipe(fd) == -1)
  {
    perror("Pipe failed");
    return -1;
  }
  if (pipe(fd1) == -1)
  {
    perror("Pipe failed");
    return -1;
  }
  if (pipe(fd2) == -1)
  {
    perror("Pipe failed");
    return -1;
  }
  child1 = fork();
  switch (child1)
  {
  case -1:
    perror("Fork failed");
    return -1;
  case 0:
    close(fd[0]);
    printf("PID %.1f IS %d, PPID %.1f IS %d\n", id + 0.1, getpid(), id, getppid()); // Child 1
    id += 0.1;
    gChild1 = fork();
    switch (gChild1)
    {
    case -1:
      perror("Fork failed");
      return -1;
    case 0:
      close(fd2[0]);
      sprintf(gid, "%.1f.1", id);
      printf("PID %s IS %d, PPID %.1f IS %d\n", gid, getpid(), id, getppid()); // Grandchild 1
      partition = bruteForce(9);
      // Write the partition to the pipe
      if (partition)
      {
        printf("%s\n", partition);
        write(fd2[1], partition, sizeof(partition));
#if DEBUG
        printf("Grandchild 1 wrote: %s\n", partition);
#endif
      }
      close(fd2[1]);
      exit(0);
    default:
      sprintf(gid, "%.1f.1", id);
      printf("PID %.1f IS %d, CHILD %s IS %d\n", id, getpid(), gid, gChild1); // Child 1
      partition = bruteForce(3);
      // Write the partition to the pipe
      if (partition)
      {
        printf("%s\n", partition);
        write(fd[1], partition, sizeof(partition));
#if DEBUG
        printf("Child 1 wrote: %s\n", partition);
#endif
      }
      close(fd[0]);
      // Wait for the grandchild to finish
      waitpid(gChild1, NULL, 0);
      exit(0);
    }
  default:
    close(fd[1]);
    printf("PID %.1f IS %d, CHILD %.1f IS %d\n", id, getpid(), id + 0.1, child1);
    child2 = fork();
    switch (child2)
    {
    case -1:
      perror("Fork failed");
      return -1;
    case 0:
      close(fd1[0]);
      printf("PID %.1f IS %d, PPID %.1f IS %d\n", id + 0.2, getpid(), id, getppid()); // Child 2
      partition = bruteForce(6);
      // Write the partition to the pipe
      if (partition)
      {
        printf("%s\n", partition);
        write(fd1[1], partition, sizeof(partition));
#if DEBUG
        printf("Child 2 wrote: %s\n", partition);
#endif
      }
      close(fd1[1]);
      exit(0);
    default:
      // Close the write end of pipes
      close(fd[1]);
      close(fd1[1]);
      close(fd2[1]);
      partition = bruteForce(0);
      if (partition)
      {
        strcpy(password, partition);
        printf("%s\n", partition);
#if DEBUG
        printf("Parent got: %s\n", partition);
#endif
      }
      // Reading the password from the pipe and concatenating it to the password.
      read(fd[0], partition, sizeof(partition));
      strcat(password, partition);
      read(fd1[0], partition, sizeof(partition));
      strcat(password, partition);
      read(fd2[0], partition, sizeof(partition));
      strcat(password, partition);
      // Close the read end of pipes
      close(fd[0]);
      close(fd1[0]);
      close(fd2[0]);
      // Wait for the child processes to finish
      waitpid(child1, NULL, 0);
      waitpid(child2, NULL, 0);
      return 0;
    }
  }
}
