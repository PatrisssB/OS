/*#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>


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
                printf("%s: this is a this is a regular file\n", filename);
                print_reg_file_info(filename);
                break;
            case S_IFLNK:
                printf("%s: this is a symbolic link\n", filename);
                print_sym_link_info(filename);
                break;
            case S_IFDIR:
                printf("%s: directory\n", filename);
                print_dir_info(filename);
                break;
            default:
                printf("%s: thia is an unknown file type\n", filename);
                break;
        }
    }
    
    return 0;
}
*/