#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


#define MAX_OPTIONS 10
#define BUFFER_SIZE 1024

void print_reg_file_info(char* filename);
void print_sym_link_info(char* filename);
void print_access_rights(mode_t mode);
void print_dir_info(char* dirname);
int count_lines(char* filename);

int main(int argc, char* argv[]) 
{
    int fp = open("grades.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR);

    int i;
    for (i = 1; i < argc; i++) 
    {
        char* filename = argv[i];
        struct stat st;
        
        if (lstat(filename, &st) == -1) 
        {
            perror("lstat");
            continue;
        }
        
        switch (st.st_mode & S_IFMT) 
        {
            case S_IFREG:
            {
                printf("%s: the introduced regular file\n", filename);

                int pid_count_reg = 0;
                pid_t pid_reg = fork();

                if (pid_reg < 0)
                {
                    perror("there is no process created");
                    exit(9); 
                }

                else if( pid_reg == 0) 
                {
                    print_reg_file_info(filename);
                    exit(11);
                }
                else
                {
                    pid_count_reg++;

                    int pipefd[2];
                    if (pipe(pipefd) == -1)
                    {
                        perror("pipe create went wrong");
                        exit(5);
                    }

                    pid_t pid_script = fork();

                    if (pid_script < 0 )
                    {
                        perror("there is no process created");
                        exit(9);
                    }
                
                    else if ( pid_script == 0)
                    {
                        if(filename[strlen(filename)-1]=='c' && filename[strlen(filename)-2]=='.')
                        {

                            if( close(pipefd[0] == -1) )
                            {
                                perror("error - pipe/read");
                                exit(50);
                            } 
                           
                            if (dup2(pipefd[1], STDOUT_FILENO) == -1) 
                            {
                                perror("error - dup2/write");
                                exit(51);
                            }

                            char scriptname[10] = "script.sh";
                            execlp("bash", "bash", scriptname, filename, NULL);
                            exit(12);
                        }
                        else
                        {
                            int lines = count_lines(filename);
                            if (lines >= 0) 
                            {
                                printf("File '%s' has %d lines\n", filename, lines);
                            }
                            exit(13);
                        }
                    }
                    else
                    {
                        pid_count_reg++;
                        //printf("count %d",pid_count_reg);
                        pid_t wpid;
                        int status;
                        for( int i = 0; i < pid_count_reg; i++)
                        {
                            wpid = wait(&status);
                            if (WIFEXITED(status))
                            {
                                printf("\n");
                                printf("Child process with PID %d exited with status %d\n", wpid, WEXITSTATUS(status));
                            }
                            else
                                {
                                    printf("\n");
                                    printf("Child process  with PID %d did not exit normally\n", wpid);
                                }     
                        }

                        if( close(pipefd[1]) == -1)
                        {
                            perror("error - pipe/write");
                            exit(51);
                        } 

                        char buffer[BUFFER_SIZE];
                       
                        int i;
                        for(i = 0; i <= 1; i++) 
                        {
                            char c;
                            read(pipefd[0], &c, 1);
                            buffer[i] = c;

                        }
                        buffer[i] = '\0';
                        //printf("%s\n", buffer);

                        int nr = atoi(buffer);
                        int nr_errors = nr / 10;
                        int nr_warnings = nr % 10;
                        int grade;
                        if (nr_errors == 0 && nr_warnings == 0)
                                grade = 10;
                        else if (nr_errors == 0 && nr_warnings > 10)
                                grade = 2;
                        else if (nr_errors == 0 && nr_warnings < 10)
                                grade = 2 + 8 * (10 - nr_warnings) / 10;
                        else if (nr_errors > 0)
                                grade = 1;

                        if( close(pipefd[0]) )
                        {
                            perror("error - pipe/read");
                            exit(50);
                        }
                        //printf("Code score for %s: %d errors, %d warnings\n", filename, nr_errors, nr_warnings);

                        char gradeString[100];
                        sprintf(gradeString, "Grade for file %s is: %d\n", filename, grade);
                        strcat(gradeString, "\0");
                        write(fp, gradeString, strlen(gradeString));
                    }
                }
                break;
            }

            case S_IFLNK:
            {
                printf("%s: the introduced symbolic link\n", filename);

                int pid_count_link = 0;
                pid_t pid_link = fork();

                if (pid_link < 0)
                {
                    perror("there is no process created");
                    exit(9);
                }
                else if ( pid_link == 0)
                {
                    print_sym_link_info(filename);
                    exit(22);
                }
                else
                {
                    pid_count_link++;
                    pid_t pid_change = fork();

                    if (pid_change < 0)
                    {
                        perror("there is no process created");
                        exit(9);
                    }
                    else if (pid_change == 0)
                    {
                        execlp("chmod", "chmod", "u=rwx,g=rw,o=", filename, NULL);
                        printf("error regarding the chmod");
                        exit(21);
                    }
                    else
                    {
                        pid_count_link++;

                        pid_t wpid;
                        int status;
                        for( int i = 0; i < pid_count_link; i++)
                        {
                            wpid = wait(&status);
                            if (WIFEXITED(status))
                            {
                                printf("\n");
                                printf("Child process with PID %d exited with status %d\n", wpid, WEXITSTATUS(status));
                            }
                            else
                                {
                                    printf("\n");
                                    printf("Child process  with PID %d did not exit normally\n", wpid);
                                }     
                        }

                    }

                }
            }
            break;

            case S_IFDIR:
                printf("%s: the introduced the directory: \n", filename);

                int pid_count_dir = 0;
                pid_t pid_dir = fork();

                if (pid_dir < 0)
                {
                    perror("there is no process created");
                    exit(9);
                }
                else if ( pid_dir == 0)
                {
                    print_dir_info(filename);
                    exit(33);
                }
                else
                {   
                    pid_count_dir++;
                    pid_t pid_create = fork();
                    if (pid_create < 0 )
                    {
                        perror("there is no process created");
                        exit(9);
                    }

                    else if ( pid_create == 0)
                    {
                        char *cmd = "touch";
                        char name[strlen(filename) + 11];
                        sprintf(name, "%s/%s_file.txt", filename, filename);
                        execlp(cmd, cmd, name, (char *) NULL);
                        printf("error regarding touch");
                        exit(31);
                    }
                    else
                    {
                        pid_count_dir++;

                        pid_t wpid;
                        int status;
                        for( int i = 0; i < pid_count_dir; i++)
                        {
                            wpid = wait(&status);
                            if (WIFEXITED(status))
                            {
                                printf("\n");
                                printf("Child process with PID %d exited with status %d\n", wpid, WEXITSTATUS(status));
                            }
                            else
                                {
                                    printf("\n");
                                    printf("Child process  with PID %d did not exit normally\n", wpid);
                                }     
                        }
                    }
                break;
                }

            default:
                printf("%s: this is an unknown file type\n", filename);
                break;
        }
    }
    
    return 0;
}


void print_reg_file_info(char* filename) 
{
    struct stat st;
    char options[MAX_OPTIONS];
    int ok= 1;
    if (stat(filename, &st) == -1) 
    {
        perror("stat");
        return;
    }

    printf("Options: name(-n) , size(-d), hard link count(-h), time of last modification(-m), access rights(-a), create symbolic link(-l): ");
    printf("\n");
    scanf("%s", options);

    for(int i=0;i<strlen(options); i++)
    {
        if(strchr("- nldahm",options[i])==NULL)
            ok=0;
    }
    if(ok==0)
    {
        printf("Invalid option\n");
    }
    if (options[0]=='-' && ok==1)
    {
        for(int i=0;i<strlen(options); i++)
        {
            switch(options[i])
            {
                case 'n':
                    printf("Name: %s\n", filename);
                    break;
                case 'd':
                    printf("Size: %ld bytes\n", st.st_size);
                    break;
                case 'h':
                    printf("Hard link count: %ld\n", st.st_nlink);
                    break;
                case 'm':
                    printf("Last modified time: %s", ctime(&st.st_mtime));
                    break;
                case 'a':
                    printf("Access rights: ");
                    print_access_rights(st.st_mode);
                    break;
                case 'l':
                    printf("Create symbolic link (-l): ");
                    char linkname[BUFFER_SIZE];
                    printf("Enter link name: ");
                    scanf("%s", linkname);
                    if (symlink(filename, linkname) == -1) 
                    {
                        perror("symlink");
                    }
                    break;
                case '-':
                    break;
                case ' ':
                    break;
                default:
                    break;
            }
        }
    }
}

void print_sym_link_info(char* filename) 
{
    struct stat st;
    char options[MAX_OPTIONS];
    int ok=1;
    if (lstat(filename, &st) == -1)
    {
        perror("lstat");
        return;
    }
    printf("Options: name(-n), delete symblic link(-l) , size of symblic link(-d), size of target(-t), access rights(-a): ");
    printf("\n");
    scanf("%s", options);

    for(int i=0;i<strlen(options); i++)
    {
        if(strchr("- nldta",options[i])==NULL)
            ok=0;
    }
    if(ok==0)
    {
        printf("Invalid option\n");
    }
    if (options[0]=='-' && ok==1)
    {
        for(int i=0;i<strlen(options); i++)
        {
            switch(options[i])
            {
                case 'n':
                    printf("Name: %s\n", filename);
                    break;
                case 'l':
                    if (unlink(filename) == -1) 
                    {
                        perror("unlink");
                    }
                    i=strlen(options);
                    break;
                case 'd':
                    printf("Size of symbolic link: %ld bytes\n", st.st_size);
                    break;
                case 't':
                    printf("Size of target file: %ld bytes\n", stat(filename, &st) == -1 ? -1 : st.st_size);
                    break;
                case 'a':
                    printf("Access rights: ");
                    print_access_rights(st.st_mode);
                    break;
                case '-':
                    break;
                case ' ':
                    break;
                default:
                    break;
            }
        }
    }
    
}

void print_access_rights(mode_t mode) 
{

    printf("User: \n");
    printf("\nRead - %s \nWrite - %s \nExec - %s \n",
            (mode & S_IRUSR) ? "yes" : "no",
            (mode & S_IWUSR) ? "yes" : "no",
            (mode & S_IXUSR) ? "yes" : "no"         );

    printf("\nGroup: \n");
    printf("\nRead - %s \nWrite - %s \nExec - %s \n",
            (mode & S_IRGRP) ? "yes" : "no",
            (mode & S_IWGRP) ? "yes" : "no",
            (mode & S_IXGRP) ? "yes" : "no"          );

    printf("\nOthers: \n");
    printf("\nRead - %s \nWrite - %s \nExec - %s\n",
            (mode & S_IROTH) ? "yes" : "no",
            (mode & S_IWOTH) ? "yes" : "no",
            (mode & S_IXOTH) ? "yes" : "no"         );
}

void print_dir_info(char* dirname) 
{
    DIR* dir;
    struct dirent* entry;
    struct stat st;
    char filename[BUFFER_SIZE];
    int total_size = 0;
    int total_c_files = 0;
    char options[MAX_OPTIONS];
    int ok=1;
    if ((dir = opendir(dirname)) == NULL) 
    {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) 
    {
        sprintf(filename, "%s/%s", dirname, entry->d_name);
        if (lstat(filename, &st) == -1) 
        {
            perror("lstat");
            continue;
        }
        if (S_ISREG(st.st_mode)) {
            if (strstr(entry->d_name, ".c") != NULL) 
            {
                total_c_files++;
            }
            total_size += st.st_size;
        }
    }
    printf("Options: name(-n), size(-d), access rights(-a), number of C files(-c): ");
    printf("\n");
    scanf("%s", options);
    for(int i=0;i<strlen(options); i++)
    {
        if(strchr("- ndca",options[i])==NULL)
            ok=0;
    }
    if(ok==0)
    {
        printf("Invalid option\n");
    }
    if (options[0]=='-' && ok==1)
    {
        for(int i=0;i<strlen(options); i++)
        {
            switch(options[i])
            {
                case 'n':
                    printf("%s: directory\n", dirname);
                    break;
                case 'd':
                    printf("Total size: %d bytes\n", total_size);
                    break;
                case 'c':
                    printf("Total .c files: %d\n", total_c_files);
                    break;
                case 'a':
                    printf("Access rights: \n");
                    print_access_rights(st.st_mode);
                    break;
                case '-':
                    break;
                case ' ':
                    break;
                default:
                    break;
            }
        }
    }
    closedir(dir);
}

int count_lines(char* filename) 
{
    FILE* file = fopen(filename, "r");
    if (file == NULL) 
    {
        printf("Unable to open file '%s'\n", filename);
        return -1;
    }

    int lines = 0;
    char c;
    while ((c = fgetc(file)) != EOF) 
    {
        if (c == '\n') 
        {
            lines++;
        }
    }

    fclose(file);
    return lines;
}