# Password Cracker
> You have been provided with a program called hackme. To successfully hack the program, you need to provide the correct password. A random password would be impossible to guess.

The program that attempts to find a password by brute-forcing through all possible combinations of characters. It can be run in two modes: fork mode or regular mode.

In regular mode, the program calls the function handleBruteForce(), which in turn calls the function bruteForce() for each partition of 3 characters of the password, until a valid password is found or all possible combinations have been checked.

In fork mode, the program forks two child processes and one grandchild process. Each of the child processes calls the bruteForce() function for different partitions of the password. The grandchild process is responsible for running the checkPassword() function, which is provided by an external library called "checkPassword.h". The parent process waits for any of the child processes to return the correct password.