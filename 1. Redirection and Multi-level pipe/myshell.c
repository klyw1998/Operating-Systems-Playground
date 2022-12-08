/*
    COMP3511 Fall 2022 
    PA1: Simplified Linux Shell (MyShell)

    Your name: Liangyawei Kuang 
    Your ITSC email: lkuang@connect.ust.hk 

    Declaration:

    I declare that I am not involved in plagiarism
    I understand that both parties (i.e., students providing the codes and students copying the codes) will receive 0 marks. 

*/

// Note: Necessary header files are included
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h> // For constants that are required in open/read/write/close syscalls
#include <sys/wait.h> // For wait() - suppress warning messages
#include <fcntl.h> // For open/read/write/close syscalls

// Assume that each command line has at most 256 characters (including NULL)
#define MAX_CMDLINE_LEN 256

// Assume that we have at most 8 arguments
#define MAX_ARGUMENTS 8

// Assume that we only need to support 2 types of space characters: 
// " " (space) and "\t" (tab)
#define SPACE_CHARS " \t"

// The pipe character
#define PIPE_CHAR "|"

// Assume that we only have at most 8 pipe segements, 
// and each segment has at most 256 characters
#define MAX_PIPE_SEGMENTS 8

// Assume that we have at most 8 arguments for each segment
//
// We also need to add an extra NULL item to be used in execvp
//
// Thus: 8 + 1 = 9
//
// Example: 
//   echo a1 a2 a3 a4 a5 a6 a7 
//
// execvp system call needs to store an extra NULL to represent the end of the parameter list
//
//   char *arguments[MAX_ARGUMENTS_PER_SEGMENT]; 
//
//   strings stored in the array: echo a1 a2 a3 a4 a5 a6 a7 NULL
//
#define MAX_ARGUMENTS_PER_SEGMENT 9

// Define the  Standard file descriptors here
#define STDIN_FILENO    0       // Standard input
#define STDOUT_FILENO   1       // Standard output 


 
// This function will be invoked by main()
// TODO: Implement the multi-level pipes below
void process_cmd(char *cmdline);

// read_tokens function is given
// This function helps you parse the command line
// Note: Before calling execvp, please remember to add NULL as the last item 
void read_tokens(char **argv, char *line, int *numTokens, char *token);

// Here is an example code that illustrates how to use the read_tokens function
// int main() {
//     char *pipe_segments[MAX_PIPE_SEGMENTS]; // character array buffer to store the pipe segements
//     int num_pipe_segments; // an output integer to store the number of pipe segment parsed by this function
//     char cmdline[MAX_CMDLINE_LEN]; // the input argument of the process_cmd function
//     int i, j;
//     char *arguments[MAX_ARGUMENTS_PER_SEGMENT] = {NULL}; 
//     int num_arguments;
//     strcpy(cmdline, "ls | sort -r | sort | sort -r | sort | sort -r | sort | sort -r");
//     read_tokens(pipe_segments, cmdline, &num_pipe_segments, PIPE_CHAR);
//     for (i=0; i< num_pipe_segments; i++) {
//         printf("%d : %s\n", i, pipe_segments[i] );    
//         read_tokens(arguments, pipe_segments[i], &num_arguments, SPACE_CHARS);
//         for (j=0; j<num_arguments; j++) {
//             printf("\t%d : %s\n", j, arguments[j]);
//         }
//     }
//     return 0;
// }


/* The main function implementation */
int main()
{
    char cmdline[MAX_CMDLINE_LEN];
    fgets(cmdline, MAX_CMDLINE_LEN, stdin);
    process_cmd(cmdline);
    return 0;
}

// TODO: implementation of process_cmd
void process_cmd(char *cmdline)
{
    /*if (strcmp(cmdline, "exit") == 0) {
        printf("The shell program (pid=%d) terminates\n", getpid());
        exit(0);
    }*/
    char *pipe_segments[MAX_PIPE_SEGMENTS]; // character array buffer to store the pipe segements
    int num_pipe_segments; // an output integer to store the number of pipe segment parsed by this function
    int i, j, k=0;
    char *arguments[MAX_ARGUMENTS_PER_SEGMENT] = {NULL}; 
    int num_arguments;

    
    pid_t pid;
    read_tokens(pipe_segments, cmdline, &num_pipe_segments, PIPE_CHAR);
    for (i=0; i < num_pipe_segments; i++) {
        //printf("%d : %s\n", num_pipe_segments, pipe_segments[i] );
        
        int in, out = dup(1);
        int pfds[2];
        
        
        
        
        if (i < num_pipe_segments-1) pipe(pfds);
        
        read_tokens(arguments, pipe_segments[i], &num_arguments, SPACE_CHARS);
        arguments[num_arguments] = NULL;
        //printf("nihao, %d\n", num_arguments); 
        if (i != num_pipe_segments-1 && num_pipe_segments > 1)       
          pid = fork();
        
        //printf("%s\n", arguments[0]);
        
        if (num_pipe_segments > 1) {
        if ( pid == 0 ) { /* The child process*/
        close(1); /* close stdout */
        dup(pfds[1]); /* make stdout as pipe input (1 is the smallest unused file descriptor) */
        close(pfds[0]);/* don't need this */
        } else { /* The parent process*/
        close(0); /* close stdin */
        dup(pfds[0]); /* make stdin as pipe output (0 is the smallest unused file descriptor) */
        close(pfds[1]); /* don't need this * wait(0); /* wait for the child process */
        if (i < num_pipe_segments-1) 
          continue;
        }
        }
        

        
        
        for (j=0; j < num_arguments; j++) {
            
            if (pid < 0) {
                perror("fork");
            } 
            else if (pid == 0) {
                //printf("\t%d : %s\n", j, arguments[j]);
                
                /*if (j) {dup2(in, 0), close(in);}
                if (j < num_arguments-1) {dup2(ps[1], 1), close(ps[1]);}
                else {dup2(out, 1), close(out);}*/
                
                char input[64],output[64];
                int fd0, fd1;
                // replacing '<' or '>' by NULLs
                //printf("%s\n", arguments[j]);
                //printf("%c\n", arguments[j][0]);
                if(arguments[j][0] == '<'){        
                arguments[j]=NULL;
                //printf("in!");
                //printf("%s\n", arguments[j]);
                strcpy(input,arguments[j+1]);
                //printf("%s\n", input);
                //in=2;
                int fd0 = open(input, O_RDONLY);
                dup2(fd0, STDIN_FILENO);
                close(fd0);
                }
                else if(arguments[j][0] == '>'){        
                arguments[j]=NULL;
                //printf("out!");
                //printf("%s\n", arguments[j]);
                strcpy(output,arguments[j+1]);
                //printf("%s\n", output);
                //out=2;
                int fd1 = creat(output, 0644);
                dup2(fd1, STDOUT_FILENO);
                close(fd1);
                }
                
                               
                /*if(strcmp(arguments[j],">")==0){      
                arguments[j]=NULL;
                printf("out!");
                printf("%s\n", arguments[j]);
                strcpy(output,arguments[j+1]);
                out=2; 
                }        
                if (in) { //if '<' char was found in string inputted by user
                int fd0 = open(input, O_RDONLY);
                dup2(fd0, STDIN_FILENO);
                close(fd0);
                }
                if (out) { //if '>' was found in string inputted by user
                int fd1 = creat(output, 0644);
                dup2(fd1, STDOUT_FILENO);
                close(fd1);
                }*/
                
                //perror("execvp");
                //_exit(1);
            } 
            
        }
        if (pid == 0){
        //printf("%s\n", arguments[0]);
        execvp(arguments[0], arguments);
        perror("execvp");
        _exit(1);
        } // execution line
        else {
        //printf("%s\n", arguments[0]);
          waitpid(pid, 0, 0);
          if (num_pipe_segments > 1) {
            execvp(arguments[0], arguments);
            perror("execvp");
            _exit(1); // execution line
            }
        }
    }
    
    
}

// Implementation of read_tokens function
void read_tokens(char **argv, char *line, int *numTokens, char *delimiter)
{
    int argc = 0;
    char *token = strtok(line, delimiter);
    while (token != NULL)
    {
        argv[argc++] = token;
        token = strtok(NULL, delimiter);
    }
    *numTokens = argc;
}