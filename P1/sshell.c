#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#define CMDLINE_MAX 512
#define TOKEN_MAX 32
#define MAX_ARGS 16

typedef struct command{
	int cmd_count;
        char *args[4][MAX_ARGS+1];
}command;

/* parse commands by whitespace and put them into arrays */
void parse_space(char* string, command* input){
	int count = 0;
	int x = input->cmd_count;
	/* parse string by whitespace */
	char *token = " ";
	char* parsed = strtok(string,token);
	/* put arguments into an array */
	while(parsed != NULL){
		input->args[x][count] = parsed;
		parsed = strtok(NULL, token);
		count += 1;
	}
	/* put NULL at end of array and increment count */
	input->args[x][count] = NULL;
	input->cmd_count += 1;
}

/* parse file by whitespace and return filename */
char* parse_file (char* string){
	/* parse string by white string */
	char *token = " ";
	char *parsed = strtok(string,token);
	return parsed;
}

/* parse commands by | and calls parse_space to put them into arrays*/
void parse_pipe(char* string, command* input){
	/* parse string by | */
	char *token_a = "|";
	char* parsed = strtok(string,token_a);
	int count = 0;
	/* stores a temporary copy of parsed string */
	char* array[4];
	while (parsed != NULL && count < 4){
		array[count] = parsed;
		parsed = strtok(NULL, token_a);
		count += 1;
	}
	/* calls parse_space to get rid of whitespaecs */
	for (int a = 0;  a < count; a++){
		parse_space(array[a], input);
	}
}

/* check if number of argments are within 16 */
int check_arg_num(char* string){
	/* create copy of string so it won't get changed */
	char cmd_check[CMDLINE_MAX];
	strcpy(cmd_check, string);
	int count = 0;
	/* separate commands by whitespaecs */
	char *token = " ";
	char* parsed = strtok(cmd_check, token);
	while (parsed != NULL){
		parsed = strtok(NULL, token);
		count += 1;
	} 
	/* check if command is within bound */
	if (count > MAX_ARGS){
		return 1;
	}
	return 0;
}

unsigned long fsize(char* my_file)
{
	/* Open my_file in read only mode
	and create pointer to FILE object */
	FILE *f = fopen(my_file, "r");
	/* Move file pointer to end of file */
	fseek(f, 0, SEEK_END);
	/* Use ftell on pointer to return file size in bytes*/
	unsigned long len = (unsigned long)ftell(f);
	/* Close file and return file size*/
	fclose(f);
return len;
}

void pipe_process(int count, command* input, int* array){
	/* create pipe */
	int fd[2];
	pipe(fd);
	pid_t child = fork();
	if (!child){
		/* child */
		close(fd[0]);
		/* redirect STDOUT */
		dup2(fd[1],STDOUT_FILENO);
		close(fd[1]);
		/* execute command */
		execvp(input->args[count][0], input->args[count]);
		perror("execvp");
		exit(1);
	} else {
		/* parent */
		int status;
		/* wait for child proecss to finish */
		waitpid(child, &status, 0);
		array[count] = WEXITSTATUS(status);
		close(fd[1]);
		/* redirect STDIN */
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
	}
}

int main(void)
{
        char cmd[CMDLINE_MAX];

        while (1) {
                char *nl;
                int retval;
                /* Print prompt */
                printf("sshell@ucd$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';
		
		/* Continue upon return */
                if(!strcmp(cmd, "")){
                        continue;
                }

		/* if command received is pwd */
		if (!strcmp(cmd, "pwd")){
                        /* Create storage for current working directory */
			char cwd[CMDLINE_MAX];
                        /* Determin path name of cwd 
			and store into cwd buffer */
			if (getcwd(cwd, sizeof(cwd)) == NULL) {
                                perror("error with getcwd()");
                        }
                        
			else {
                                /* Output the cwd and completion status */
				fprintf(stdout, "%s\n",cwd);
                                fprintf(stderr, "+ completed '%s",cmd);
                                fprintf(stderr, "' [0]\n");
                        }
                        continue;
                }

		/* If command received is sls */
                if (!strcmp(cmd, "sls")){    
                	
			/* To receive file size */
			unsigned long file_size;                   	
			
			DIR *dp;
			struct dirent *myfile;
			dp = opendir ("./");
			
			/* Ensure directory can be opened */
			if (dp != NULL){		
				/* Process each entry */
				while ((myfile = readdir (dp))){
					/* Filter out hidden files by first character */
					if(myfile->d_name[0] != '.'){
						/* Output directory name */
						fprintf(stdout, "%s",myfile->d_name);
						
						/* Get size of file */
						file_size = fsize(myfile->d_name);
						
						/* Output file size in bytes */
						fprintf(stdout, " (%lu bytes)\n", file_size);	
					}
				}			
				
				/* Close directory and output completion status */
				(void) closedir (dp);
				fprintf(stderr, "+ completed '%s",cmd);
				fprintf(stderr, "' [0]\n");
			}		
			else{
				/* Output in the case the directory fails to open */
				perror ("Couldn't open the directory");
				fprintf(stderr, "Error: cannot cd into directory\n");
				fprintf(stderr, "+ completed '%s",cmd);
				fprintf(stderr, "' [1]\n");

			}
			continue;	
                }

		/* create a copy of cmd before it gets parsed */
                char cmd_copy[CMDLINE_MAX];
		strcpy(cmd_copy,cmd);
		/* check if there are too many commands */
		if (check_arg_num(cmd)){
			fprintf(stderr, "Error: too many process arguments\n");
			continue;
		}
		/* Inilization of command struct, */
		command cmd_post;
		cmd_post.cmd_count = 0;
		/* saves stdout in case of output redirection */
		int saved_stdout;
		saved_stdout = dup(STDOUT_FILENO);

		/* Parsing and storing cmd into command struct */
		if (strchr(cmd, '>') != NULL){
			/* counter for if redierction needs to append to EOF*/
			int set = 0; 
			char *token_redi;
			/* check append vs truncate File */
			if (strstr(cmd, ">>") != NULL) {
				token_redi = ">>";
				set += 1;
			} else {
				token_redi = ">";
			}
			/* Separate commands into 2 parts for redirect */
			char *parsed_redi = strtok(cmd, token_redi);
			char *parsed_cmd = parsed_redi;
			char *parsed_filename = strtok(NULL, token_redi);
			/* check if output file exists */
			if (parsed_filename == NULL){
				fprintf(stderr, "Error: no output file\n");
				continue;
			}
			/* parse command */
			parse_pipe(parsed_cmd, &cmd_post);
			/* check output redirection error */
			if (strchr(parsed_filename, '|') != NULL){
				fprintf(stderr, "Error: mislocated output redirection\n");
				continue;
			}
			/* opening file */
			char *file_name = parse_file(parsed_filename);
			int fd;
			if (set == 0){
				fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0777);
				/* check if file exists */
				if (fd == -1){
					fprintf(stderr, "Error: cannot open output file\n");
					continue;
				}
			} else {
				fd = open(file_name, O_WRONLY | O_CREAT, 0777);
				/* check if file exists */
				if (fd == -1){
					fprintf(stderr, "Error: cannot open output file\n");
					continue;
				}
				/* point to EOF if >> */
				lseek(fd, 0, SEEK_END);
			}
			/* redirect stdout */
			dup2(fd, STDOUT_FILENO);
			close (fd);
		} else{
			/* parse command */
			parse_pipe(cmd, &cmd_post);
		}

		/* Change current working directory */
                if (!strcmp(cmd_post.args[0][0], "cd")){     
                        struct stat stats;
			
			/* Verify directory exists */
			if(stat(cmd_post.args[0][1], &stats) == -1){
                                fprintf(stderr, "Error: cannot cd into directory\n");
                                fprintf(stderr, "+ completed '%s",cmd_post.args[0][0]);
                                fprintf(stderr, " %s", cmd_post.args[0][1]); 
                                fprintf(stderr, "' [1]\n");
                                continue;
                        }        
                        
			/* Buffer for absolute path name */
			char cwd[CMDLINE_MAX];
                        /* Place absolute pathname into cwd,
			and check validity of pathname */
			if (getcwd(cwd, sizeof(cwd)) == NULL) {
                                perror("error with getcwd()");
                        }
                        
			/* Absolute path is valid, so change current working directory
			into into chosen directory */
			else {
                                chdir(cmd_post.args[0][1]);
				fprintf(stderr, "+ completed '%s",cmd_copy);
				fprintf(stderr, "' [0]\n");
                        }
                        continue;
                }
		
		/* Exit while loop to stop displaying shell prompt */
                if (!strcmp(cmd_post.args[0][0], "exit")) {
                        
			int status = 0;
                        fprintf(stderr, "Bye...\n");
                        fprintf(stderr, "+ completed '%s",cmd);
                        fprintf(stderr, "' [%d]\n", WEXITSTATUS(status));
                        break;
                }

		/* create a child process */
		pid_t pid;
                pid = fork();
		/* return status array */
		int return_status[3] = {0,0,0}; 
                if(pid == 0){
			/* child */
			int i; 
			if (cmd_post.cmd_count > 1){
				for (i=0; i < cmd_post.cmd_count-1; i++){
					pipe_process(i, &cmd_post, return_status);
				}
			}
			/* execute last command */
			execvp(cmd_post.args[i][0], cmd_post.args[i]);
                        //perror("execvp");
			fprintf(stderr, "Error: command not found\n");
			exit(1);    
                } else if (pid > 0) {
                        /* parent */
                        int status;	
			//wait(&status);
			waitpid(pid, &status, 0);
			/* points stdout back to terminal */
			dup2(saved_stdout,STDOUT_FILENO);
			close(saved_stdout);
			/* Print completion and exit status */
			fprintf(stderr, "+ completed '%s' ",cmd_copy);
			if (cmd_post.cmd_count > 1){
				for (int a = 0; a < cmd_post.cmd_count-1; a++){
					fprintf(stderr, "[%d]", return_status[a]);
				}
			}
			fprintf(stderr, "[%d]\n", WEXITSTATUS(status));
                }
                else{
                        perror("fork");
                        exit(1);
                }
        }
        return EXIT_SUCCESS;
}
