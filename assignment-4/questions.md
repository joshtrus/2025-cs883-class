1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  Fork creates a child process so the shell can continue running while the command executes in the child. Without fork, calling execvp directly would replace the shell’s process with the new command.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  If fork() fails, it returns –1. In my implementation, i check for an error, print an error with perror, and return an error code (ERR_EXEC_CMD) instead of trying to run the command.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  execvp() searches for the command in the directories listed in the PATH environment variable, which tells the system where to look for executable files.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  wait() in the parent makes it pause until the child finishes so i can capture the exit status

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  WEXITSTATUS() extracts the exit code from the status returned by wait(), showing whether the command succeeded or failed. its important for error handling and for built-in commands like "rc" to report the correct return code.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  build_cmd_buff() toggles a flag when it encounters a double quote so that spaces inside quotes aren’t treated as delimiters. its necessary to allow arguments with spaces (like "hello, world") to remain as a single thing.


7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  I removed the old pipe-splitting logic and refactored the parsing to handle one command at a time. I added logic to trim extra spaces and properly manage quoted strings. The main challenge was ensuring that tokens were correctly split without breaking quoted text.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  Signals let processes receive asynchronous notifications rather than exchanging data. Theyre different from other IPC methods by  notifying a process that an event has occurred instead of transferring information.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  SIGINT interrupts a process, SIGTERM requests a graceful shutdown, and SIGKILL forcefully terminates a process immediately. SIGINT and SIGTERM can be caught or handled, while SIGKILL cannot.

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  When a process receives SIGSTOP, it stops until it gets a SIGCONT signal. Unlike SIGINT, SIGSTOP cannot be caught or ignored because it is made to stop a process for system control reasons
