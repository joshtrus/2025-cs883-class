1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

My shell calls waitpid() for each child process it forks, so the parent waits until every child is done before reading the next command. If waitpid() wasn’t called, some child processes might keep running in the background (becoming “zombie” processes) and could eventually use up system resources.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

After dup2() copies a file descriptor to redirect input or output, the original pipe file descriptors aren’t needed and must be closed. If they aren’t closed, the pipe might never signal that no more data is coming (because some ends stay open), which can cause problems and even run out of file descriptors.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

The cd command needs to change the current working directory of the shell process. If cd were an external command, it would run in a separate child process and wouldn’t affect the shell’s own directory. By making cd a built-in, the shell can change its own working directory correctly.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

To allow an arbitrary number of piped commands, you could use dynamic memory (like a resizable array or a linked list) instead of a fixed-size array. The trade-off is that the code becomes more complex and might be a bit slower due to extra memory management, but you gain the flexibility to handle as many commands as needed.
