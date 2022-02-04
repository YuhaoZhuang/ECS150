### Simple Shell Program Report

**Introduction**
To start off with the shell, there is a while loop that wraps every single
piece of code. After one process finish executing, the shell will loop again
continue accepting user inputs through the fgets function in the while loop.
This while loop is what makes the shell keeps running and simulating a shell in
real life. When the users press enter key on their keyboard or are using the
"\n" command, the shell just simply continues the while loop to accept a new
command. The command is processed in a similar way to the cd, pwd, and sls
command as there is no need to parse them into data strcuture and it is better
to do it directly in the while loop, which is the parent process.

**Parsing command line input and storing it**
To store the commands taken by the user to understandable code, the program
declares and creates a struct, which contains 4 arrays of string arrays of a
maximum size of 17, because there could be a maximum of 3 pipes or 4 chained
commands, and the total number of arguments is 16 plus one extra null
characters for the exec function. The basic idea is to divide the piping
process into different groups, and each array of strings store the commands
into different groups of the command line. To parse the command, the program
first searches if there are any redirection commands. If there are, the program
then split the command into 2 parts, first part for the commands without the
file, the second part for the file. The file will then be parsed by whitespaces
and be returned. After separating out the filename from the command, the
program then parses and separates the commands by '|'', the piping command.
Each of those separated commands will be parsed by white spaces and be stored
into the struct data structure created earlier. The first program before the
pipeline command '|' will be stored into the first array of strings (array[0]),
and the second will be stored into array[1] of strings, and so on. The shell's
parser also simultaneously checks for command-line errors such as incorrect
filename, incomplete commands, or too many arguments by using functions to
detect those errors and print corresponding error messages to prevent incorrect
commands being executed.

**EXIT**
To exit the shell, we trivially break the loop using ext() upon input
validation of 'exit'. This prevents the continued execution of the shell prompt.

**Forking and executing commands**
After parsing and turning in all the commands into the correct format, the
shell then creates a child using fork(). The shell checks whether the process
is a child or a parent by comparing the value of the return of fork() to 0. 0 =
child, everything else is the parent. In the child process, the shell calls the
commands using execvp, since it will automatically search in the $PATH
environment variable for the path of the process file. And the parent calls
waitpid on the child to wait for it fo finish executing and collects its return
status to print out the completion message. Since all the processing of the
commands is called in the child process, this ensures that the shell won't be
dumped or left by the calling of execvp. So after the child process finishes
running, the parent process will always be able to keep running and wait for
the next command from the terminal. 

**PWD**
For the present working directory command, we create a container to store
the absolute path name which we then verify, before we print the path with
completion status.

**Chaing Directories (CD)**
The command to change directory was handled after parsing and storing the
whole command into the command struct, so that we could deal with additional
arguments. With this command, we were able to use the stat structure to first
verify the argument (which is the directory we are attempting to cd into)
actually exists. We then simply change the working directory into the
validated chosen directory with chdir().

**Process redirection**
Everytime when the process sees a '>' character, the program then search if
there is another > that goes right behind it ('>>'), and if there are, the
program then set a counter to remind itself that the process rediection needs
to append to the EOF instead of truncating it. As mentioned earlier, the
program will separate the commands into 2 parts and parse the second part,
which is the filename, by white spaces. A file descriptor will then be openend
with either truncate or point the file offset pointing to the end of the file
based on the counter set earlier. In order for the shell to be able to still
write to the terminal after STDOUT gets connected to the file descriptor, a
copy of the STDOUT will be saved for the shell to write back to the terminal.
To let the process redirect its STDOUT or its output to the file,
dup2(fd,STDOUT_FILENO) is called to create a copy of the file descriptor and
connect STDOUT to it. Then the file descriptor will be closed since the program
won't need it anymore. At this stage, the command's output will be redirected
to the file. Finally, after the child process has finished executing the
command and writing it to the file, the parent process will then set STDOUT
back to the copy the shell saved earlier so its future processes can write to
the terminal again.

**Piping**
Since the shell divides the commands into different groups if there are pipline
the command '|' within the commands, if the shell sees that there are more than one
group of processes before executing it, instead of just creating one child and
calling one execvp command, it starts a for loop of forks() and links the STDIN
and STDOUT of its children's processes together using pipes. The basic idea
here is that after the parent process calls fork, the child process doesn't
immediately starts calling execvp to execute the commands, but instead starts a
for loop of forking and creating pipes up until 1 less than the total groups of
commands separted by the "|" piping sign. And before each execution of the
fork() in for loop, the child processes will create a pipe using file
descriptor and then fork(). After the fork() process in the for loop, those
child processes that were created by the parent's child process will then call
dup2(fd[1],STDOUT_FILENO) to let STDOUT write to fd[1] of the pipe instead of
the terminal, and the first child process created by the parent will call
dup2(fd[0],STDIN) to connect STDIN to the fd[0] of the pipe and read the output
performed by its child processes created by the for loop. That parent's first
child process also wait for its child process created by the for loop to finish
using waitpid and stores their exit status in an array. This for loop runs up
until one less than the total groups of commands, and then the first child
process that was created by the parent runs execvp on the last command in the
groups of commands since it waited and got all the inputs it needs to run the
last process. After that, the child process then stops executing, and the
parent process prints all the return status and continues executing the shell. 

**SLS**
The sls command is designed to open a directory, verify that it opened,
then process each entry, while simply ignoring all hidden files (which begins
with a '.') with an if statement. All files that aren't filtered out will
print the directory name followed by the size of the file. The sls command
gave us some trouble when using stat() to determine the file size. Our best
guess it that, as all directories in the pathname which lead to the file
require search permission (from the stat() Linux man page), there was some
illegal access which prevented safe use of stat(). Thus, we wrote a simple
function to determine the size of a given file without stat(), though it ended
up more complex than the original.


