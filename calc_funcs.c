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
                files[count] = strdup(dir-> d_name); // might have to use strdup(dir->dirname)
                // pipe error check
                if (pipe(file_descripter + 2 * count) < 0)
                {
                    perror("pipe");
                    free(files[count]);
                    exit(1);
                }
                count++;
            }
        }
        // close current directory
        closedir(directory);
    }
}

// Funciton for reading lines of .usp files
// Used most in calculator function
void read_line(int file_descripter, char* buffer, int line_length)
{
    int i = 0;
    char c;

// while loop will read read the opened file and store the first line into a buffer
    while (i < line_length - 1 && read(file_descripter, &c, 1) > 0)
    {
        // if a read character is on a new line, break loop
        if (c == '\n')
        {
            break;
        }
        buffer[i++] = c;
    }
    // buffer null terminator
    buffer[i] = '\0';
}

// Calculator function
void calculator(int file, int i)
{
    char math_operation[2], line[LINE_LENGTH], result[LINE_LENGTH];
    int _result;
    int num1, num2; // for 2 numbers that ned to be read 

    // read ID which is the first line of .usp files
    read_line(file, line, LINE_LENGTH);
    // write it to pipes
    write(file_descripter[2 * i + 1], line, strlen(line) + 1);
    write(file_descripter[2 * i + 1], "\n", 1);

    //read 2nd line which is the first num
    read_line(file, line, LINE_LENGTH);
    num1 = atoi(line);

    //read 3rd line which is the mathematical operator
    read_line(file, line, LINE_LENGTH);
    strcpy(math_operation, line);

    // read 4th line which is 2nd num
    read_line(file, line, LINE_LENGTH);
    num2 = atoi(line);

    // Do the calculation
    switch(math_operation[0])
    {
        case '+':
            _result = num1 + num2;
            break;
        case '-':
            _result = num1 - num2;
            break;
        case '/':
            _result = num1 / num2;
            break;
        case '*':
            _result = num1 * num2;
            break;
        default:
        write(2, "Math Operation Not Available\n", 29);
        free(files[i]);
        exit(1);
    }
    sprintf(result, "%d", _result);

    // writing results to the pipe
    write(file_descripter[2 * i + 1], result, strlen(result) + 1);
    write(file_descripter[2 * i + 1], "\n", 1);
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
            calculator(file, i);
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
    
    // integer that is the result that will be written to result.txt
    int write_results = open("result.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    for (int i = 0; i < count; i++)
    {
        char buffer[LINE_LENGTH];

        // read the firt line of the .usp file from the pipe then write the result to txt file
        read_line(file_descripter[2 * i], buffer, LINE_LENGTH);
        // write to result.txt
        write(write_results, buffer, strlen(buffer));
        // space between ID and result of that ID's USP file's calculation
        write(write_results, " ", 1);

        // Read the calculated result
        read_line(file_descripter[2 * i], buffer, LINE_LENGTH);
        // writing to txt file
        write(write_results, buffer, strlen(buffer));
        write(write_results, "\n", 1);

        // close pipe
        close(file_descripter[2 * i]);
    }
    // close result.txt file
    close(write_results);

    for (int i = 0; i < count; i++)
    {
        // wait for the child processes to finis
        wait(NULL);
    }
}

// == Executing function for calculator functions == //
void calculater_executer()
{
    getFiles();
    create_child_processes();
    get_results();
}

