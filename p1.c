#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ftw.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

#define MAX_DIR 20

void usage(char *pname)
{
    fprintf(stderr, "USAGE:%s -n Name -p OCTAL -s SIZE\n", pname);
    exit(EXIT_FAILURE);
}

FILE *output; 

const char* ext(char* file_name)
{
    return strrchr(file_name, '.');
}
int print_files(const char* name, FILE *output)
{
    char working_directory[100]; 
    if(getcwd(working_directory,100)==NULL)
        ERR("getcwd");
    if(chdir(name))
        ERR("chdir");
    fprintf(output,"path: %s\n",name );
    DIR *dirp;
    struct dirent *dp;
    struct stat filestat;
    if((dirp = opendir("."))==NULL)
        ERR("opendir");
    do
    {
        errno = 0;
        if((dp=readdir(dirp)) != NULL)
        {
            if(lstat(dp->d_name,&filestat))
                ERR("lstat");
            if(S_ISREG(filestat.st_mode))
                {
                    fprintf(output, "%s %ld\n",dp->d_name,filestat.st_size);
                }
        }
    } while (dp != NULL);
    if (errno != 0)
        ERR("readdir");
    if(chdir(working_directory))
        ERR("chdir");
    if(closedir(dirp))
        ERR("closedir");
    fprintf(output,"\n");
    return 0;
}

int walk(const char *name, const struct stat *s, int type, struct FTW *f)
{
    if (type == FTW_D)
        if(print_files(name,output))
            ERR("printfiles");
    return 0;
}

int main(int argc, char** argv)
{
    int c;
    char* dir_paths[MAX_DIR] = {NULL}; int dir_number = 0;
    while((c = getopt(argc, argv, "p:"))!=-1)
        switch(c)
        {
            case 'p':
                dir_paths[dir_number] = optarg;
                dir_number++;
                break;
        }
    output = stdout;
    dir_paths[0] = ".";
    dir_number = 1;
    for(int i = 0; i < dir_number; i++)
    {
        if(nftw(dir_paths[i], walk, 20, FTW_PHYS))
            ERR("nftw");
    }
    return EXIT_SUCCESS;
}
