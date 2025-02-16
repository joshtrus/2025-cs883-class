1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

fgets() reads a whole line and prevents buffer overflows by limiting how many characters it reads. It also properly handles newlines, making it ideal for reading user commands one line at a time.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

Using malloc() allocates the exact amount of memory needed at runtime. This makes the program more flexible and avoids wasting memory or risking fixed-size limits that might not suit all inputs.

3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

Trimming spaces ensures that extra whitespace dont affect the command parsing. Without trimming, it might lead to errors during execution.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

Output redirection (command > file) – sends STDOUT to a file.
Input redirection (command < file) – reads STDIN from a file.
Error redirection (command 2> file) – sends STDERR to a file.
some challenges are correctly parsing these operators, duplicating file descriptors without interfering with each other, and handling cases where multiple redirections occur simultaneously.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.
Redirection lets you send a command's input or output to a file, while piping takes the output of one command and directly feeds it into another command. In short, redirection is all about working with files, whereas piping is used to connect commands together.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

Keeping error messages (STDERR) separate from normal output (STDOUT) means you can handle errors on their own. This helps because error messages won't get mixed up with the regular output, making it easier to spot problems and debug the program.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

The shell should print error messages on STDERR separately from normal output on STDOUT so that users can clearly see when something went wrong. While it's possible to merge them (using something like 2>&1), keeping them separate by default makes debugging and error tracking much easier.
