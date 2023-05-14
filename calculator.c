#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "macros.h"

// == Define global variables == //

char *files[FILE_NUMBER]; //File name array
int count = 0; // File count
int file_descripter[2 * FILE_NUMBER]; // File descripter array

// == Calculator functions == //

// File Getter
void getFiles()
{
    // open current directory
    DIR *directory;
    struct dirent *dir;
    directory = opendir("./");

    if (directory)
    {
        // read .usp files in the current directory
        while ((dir = readdir(directory)) != NULL)
        {
            if (strstr(dir-> d_name, ".usp"))
            {
                files[count] = dir-> d_name; // might have to use strdup(dir->dirname)
                // pipe error check
                if (pipe(file_descripter + 2 * count) < 0)
                {
                    perror("pipe");
                    free(files[count]); // Maybe dont need to free
                    exit(1);
                }
                count++;
            }
        }
        // close current directory
        closedir(directory);
    }
}

// Function that creates parent and child processes and passes the files from 
// parent to child. Child processes will run calculation function here.
void create_child_processes()
{
    // for each parent process, create a child for it
    for (int i = 0; i < count; i++)
    {
        pid_t pid = fork();
        // fork error checkin
        if (pid < 0)
        {
            perror("forking has failed");
            free(files[i]);
            exit(1);
        }
        
        if (pid == 0)
        {
            // for each of the child processes, 
            for (int c = 0; c < 2 * count; c++)
            {
                if (c != 2 * i + 1) // maybe c != 2 * i && c != 2 * i + 1
                {
                    close(file_descripter[c]);
                }
            }
            // open file, using open() and O_RDONLY because cannot use fopen() and "r"
            int file = open(files[i], O_RDONLY); // O_RDONLY is the read only for open()
            // do calculation function here
            close(file);
            close(file_descripter[2 * i + 1]);
            free(files[i]);
            exit(0);
        }
        else
        {
            close(file_descripter[2 * i + 1]);
        }
    }
}

// Function that waits for child proccess to finish then gets the calculated results
void get_results()
{
    // wait 2 seconds for children to finish
    sleep(2);
    
    int write_results = open("result.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    for (int i = 0; i < count; i++)
    {
        char buffer[LINE_LENGTH];

        // read the firt line of the .usp file from the pipe then write the result to txt file
        read_line(file, line, LINE_LENGTH);
    }
}