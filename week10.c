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

int main(int argc, char* argv[]) 
{
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
                // here should be implemented the process part for regular files
                printf("%s: regular file\n", filename);
                int pid_count = 0;
                //we fork the process to create a child process
                pid_t pid_reg = fork();
                if (pid_reg < 0)
                {
                    perror("there is no process created");
                    exit(999);
                }
                else if( pid_reg == 0)
                {
                    pid_count ++;
                    // here we should create the code for the child process 
                    print_reg_file_info(filename);
                    exit(11);
                }
                else
                {
                    // code executed by the parent
                    if(filename[strlen(filename)-1]=='c' && filename[strlen(filename)-2]=='.')
                    {
                        int pipefd[2];
                        if (pipe(pipefd) == -1)
                        {
                            perror("pipe");
                            exit(1);
                        }
                        
                        pid_t pid_script = fork();
                        if (pid_script < 0 )
                        {
                            perror("there is no process created");
                            exit(999);
                        }
                        else if ( pid_script == 0)
                        {
                            pid_count++;
                            // here we should call the script and run it 
                            close(pipefd[0]); // close unused read end of the pipe
                            // redirect stdout to the pipe write end

                            if (dup2(pipefd[1], STDOUT_FILENO) == -1)
                            {
                                perror("dup2");
                                exit(1);
                            }
                            close(pipefd[1]); // close pipe write end

                            execlp("/bin/bash", "/bin/bash", "/mnt/c/Users/triss/Desktop/os_project/script.sh", "arg1", NULL);
                            perror("Failed to execute script");
                            exit(12);
                            
                        }
                    else
                    {
                    /*pid_t pid_lines = fork();
        if (pid_lines < 0) {
            perror("there is no process created");
            exit(999);
        }
        else if (pid_lines == 0) 
        {
            pid_count ++;
            // code for child process
            print_reg_file_info(filename);
            exit(11);
        }*/
                        // parent process       
                        // should use wait
                        close(pipefd[1]); // close unused write end of the pipe
                        char buffer[BUFFER_SIZE];
                        int nbytes = read(pipefd[0], buffer, BUFFER_SIZE);
                        if (nbytes == -1)
                        {
                            perror("read");
                            exit(1);
                        }
                        close(pipefd[0]); // close pipe read end

                        buffer[nbytes] = '\0';
                        int num_errors = 0, num_warnings = 0;
                        char *token = strtok(buffer, "\n");
                        while (token != NULL)
                            {
                                if (strstr(token, "error") != NULL)
                                {
                                    num_errors++;
                                }
                                else if (strstr(token, "warning") != NULL)
                                {
                                    num_warnings++;
                                }
                                token = strtok(NULL, "\n");
                            }
                        printf("Code score for %s: %d errors, %d warnings\n", filename, num_errors, num_warnings);
                        token = NULL;
                        
                        pid_t wpid;
                        int status;
                        for( int i = 0; i < pid_count; i++)
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

            /*case S_IFLNK:
            // create a pipe for communication between parent and child processes
            int pfd[2];
            pipe( pfd);
            if (pipe(pfd) == -1) 
            {
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            pid_t pid_link = fork();
            if (pid_link == -1) 
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            else if (pid_link == 0) 
            {
            // we are in the child process
                close(pfd[0]); // close unused read end of the pipe
            // set the new permissions for the symbolic link
                if (symlink(filename, "temp_link") == -1) 
                {
                    perror("symlink");
                    exit(EXIT_FAILURE);
                }
                if (chmod("temp_link", S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP) == -1) 
                {
                    perror("chmod");
                    exit(EXIT_FAILURE);
                }
                if (write(pfd[1], "success", 8) == -1) 
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
                close(pfd[1]); // close write end of the pipe
                exit(EXIT_SUCCESS);
            }
            else 
            {
        int status;
        waitpid(pid_link, &status, 0);
        if (WIFEXITED(status)) {
            // check if child process succeeded in changing the permissions of the symbolic link
                char buf[8];
                close(pfd[1]); // close unused write end of the pipe
                if (read(pfd[0], buf, sizeof(buf)) == -1) 
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                close(pfd[0]); // close read end of the pipe
                if (strcmp(buf, "success") == 0) 
                {
                    printf("Changed permissions for %s\n", filename);
                }
                else 
                {
                    printf("Failed to change permissions for %s\n", filename);
                }
                printf("\n");
                printf("Child process exited with status %d\n", WEXITSTATUS(status));
            }
            else 
                {
                    printf("\n");
                    printf("Child process did not exit normally\n");
                }
            }
            break; */
            case S_IFLNK:
{
    printf("%s: symbolic link\n", filename);
    pid_t pid_link = fork();
    if (pid_link < 0)
    {
        perror("there is no process created");
        exit(999);
    }
    else if ( pid_link == 0)
    {
        // we are in the child process
        close(STDOUT_FILENO); // close stdout
        int fd[2];
        if (pipe(fd) == -1)
        {
            perror("pipe");
            exit(1);
        }
        pid_t pid_link2 = fork();
        if (pid_link2 < 0)
        {
            perror("there is no process created");
            exit(999);
        }
        else if (pid_link2 == 0)
        {
            // we are in the child process of the second fork
            close(fd[0]); // close unused read end of the pipe
            if (dup2(fd[1], STDOUT_FILENO) == -1)
            {
                perror("dup2");
                exit(1);
            }
            close(fd[1]); // close write end of the pipe
            execlp("chmod", "chmod", "u+rwx,g+rw,o-rwx", filename, NULL);
            perror("Failed to execute chmod");
            exit(22);
        }
        else
        {
            // we are in the first child process
            close(fd[1]); // close unused write end of the pipe
            int nbytes;
            char buffer[BUFFER_SIZE];
            while ((nbytes = read(fd[0], buffer, BUFFER_SIZE)) > 0)
            {
                write(STDOUT_FILENO, buffer, nbytes); // write output of second child to stdout
            }
            if (nbytes == -1)
            {
                perror("read");
                exit(1);
            }
            close(fd[0]); // close read end of the pipe
            exit(22);
        }
    }
    else
    {
        // we are in the parent process
        int status;
        waitpid(pid_link, &status, 0);
        if (WIFEXITED(status)) 
        {
            printf("\n");
            printf("Child process exited with status %d\n", WEXITSTATUS(status));
        } 
        else 
        {
            printf("\n");
            printf("Child process did not exit normally\n");
        }
    }
    break;
}


            case S_IFDIR:
                // here should be implemented the process part for directories
                printf("%s: directory\n", filename);
                pid_t pid_dir = fork();
                if (pid_dir < 0)
                {
                    perror("there is no process created");
                    exit(999);
                }
                else if ( pid_dir == 0)
                {
                    // we are in the child process
                    print_dir_info(filename);
                    exit(33);
                }
                else
                {
                    // we are in the parent process
                    // again i guess we should just wait
                    pid_t pid_create = fork();
                    if (pid_create < 0 )
                    {
                        perror("there is no process created");
                        exit(999);
                    }
                    else if ( pid_create == 0)
                    {
                        //execlp("touch", "touch", "/path/to/directory/newfile.txt", NULL);-
                        //execlp("touch", "touch", "/mnt/c/Users/triss/Desktop/os_project/dir1/new_file.c", NULL);
                        char *cmd = "touch";
                        char filename[strlen(argv[1]) + 11];
                        sprintf(filename, "%s/%s_file.txt", argv[1], argv[1]);
                        execlp(cmd, cmd, filename, (char *) NULL);
                        exit(31);
                        //the process stops when i introduce the option -c
                    }
                    else
                    {
                        pid_t wpid;
                                int status;
                                while ((wpid = wait(&status)) > 0)
                                {
                                    if (wpid == pid_dir || wpid == pid_create)
                                    {
                                        if (WIFEXITED(status))
                                        {
                                            printf("\n");
                                            printf("Child process with PID %d exited with status %d\n", wpid, WEXITSTATUS(status));
                                        }
                                        else
                                            {
                                                printf("\n");
                                                printf("Child process with PID %d did not exit normally\n", wpid);
                                            }
                                    }
                                }
                    }

                }

                break;
            default:
                printf("%s: thia is an unknown file type\n", filename);
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
    printf("User: Read - %s Write - %s Exec - %s \n",
        (mode & S_IRUSR) ? "yes" : "no",
        (mode & S_IWUSR) ? "yes" : "no",
        (mode & S_IXUSR) ? "yes" : "no");
        printf("Group: Read - %s Write - %s Exec - %s \n",
        (mode & S_IRGRP) ? "yes" : "no",
        (mode & S_IWGRP) ? "yes" : "no",
        (mode & S_IXGRP) ? "yes" : "no");
        printf("Others: Read - %s Write - %s Exec - %s\n",
        (mode & S_IROTH) ? "yes" : "no",
        (mode & S_IWOTH) ? "yes" : "no",
        (mode & S_IXOTH) ? "yes" : "no");
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
        if (S_ISREG(st.st_mode)) 
        {
            if (strstr(entry->d_name, ".c") != NULL) 
            {
                total_c_files++;
            }
            total_size += st.st_size;
        }
    }
    printf("Options: name(-n), size(-d), access rights(-a), number of C files(-c): ");
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