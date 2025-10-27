#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
#define MAX_SIZE 100

void usage(char *pname)
{
    fprintf(stderr, "USAGE:%s -n Name -p OCTAL -s SIZE\n", pname);
    exit(EXIT_FAILURE);
}

struct nameAsize
{
    char *d_name;
    off_t st_size;
};

ssize_t bulk_write(int fd, char *buf, size_t count)
{
    ssize_t c;
    ssize_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(write(fd, buf, count));
        if (c < 0)
            return c;
        buf += c;
        len += c;
        count -= c;
    } while (count > 0);
    return len;
}

void listing(char* path, int fd)
{
    struct nameAsize files[MAX_SIZE]; int files_size = 0;
    struct nameAsize links[MAX_SIZE]; int links_size = 0;
    struct nameAsize dirs[MAX_SIZE]; int dirs_size = 0;
    struct stat filestat;
    struct dirent *entry;

    char cur_path_name[MAX_SIZE];
    if(getcwd(cur_path_name,MAX_SIZE)==NULL)
        ERR("getcwd");
    if(chdir(path))
        ERR("change working directory to path");
    DIR *temp_dir;
    if((temp_dir = opendir("."))==NULL)
        ERR("opendir");
    
    while((entry = readdir(temp_dir))!=NULL)
    {
        errno = 0;
        if(lstat(entry->d_name, &filestat))
            ERR("lstat");
        if(S_ISREG(filestat.st_mode))
        {
            files[files_size].d_name = entry->d_name;
            files[files_size].st_size = filestat.st_size;
            files_size++;
        }
        if(S_ISDIR(filestat.st_mode))
        {
            dirs[dirs_size].d_name = entry->d_name;
            dirs[dirs_size].st_size = filestat.st_size;
            dirs_size++;
        }
        if(S_ISLNK(filestat.st_mode))
        {
            links[links_size].d_name = entry->d_name;
            links[links_size].st_size = filestat.st_size;
            links_size++;
        }
    }
    // fprintf(output,"SCIEZKA:\n%s\n",path);
    // fprintf(output,"LISTA PLIKÓW:\n");
    // for(int i = 0; i < files_size; i++)
    //     fprintf(output,"%s %ld\n",files[i].d_name, files[i].st_size);
    // fprintf(output,"LISTA LINKÓW\n");
    // for(int i = 0; i < links_size; i++)
    //     fprintf(output,"%s %ld\n", links[i].d_name, links[i].st_size);
    // fprintf(output,"LISTA FOLDERÓW\n");
    // for(int i = 0; i < dirs_size; i++)
    //     fprintf(output,"%s %ld\n", dirs[i].d_name, dirs[i].st_size);

    char buffer[256]; // lub większy, jeśli wiesz, że linie mogą być dłuższe
    int len;

    // SCIEZKA
    len = snprintf(buffer, sizeof(buffer), "SCIEZKA:\n%s\n", path);
    write(fd, buffer, len);

    // LISTA PLIKÓW
    len = snprintf(buffer, sizeof(buffer), "LISTA PLIKÓW:\n");
    write(fd, buffer, len);

    for(int i = 0; i < files_size; i++) {
        len = snprintf(buffer, sizeof(buffer), "%s %ld\n", files[i].d_name, files[i].st_size);
        write(fd, buffer, len);
    }

    // LISTA LINKÓW
    len = snprintf(buffer, sizeof(buffer), "LISTA LINKÓW\n");
    write(fd, buffer, len);
    for(int i = 0; i < links_size; i++) {
        len = snprintf(buffer, sizeof(buffer), "%s %ld\n", links[i].d_name, links[i].st_size);
        write(fd, buffer, len);
    }

    // LISTA FOLDERÓW
    len = snprintf(buffer, sizeof(buffer), "LISTA FOLDERÓW\n");
    write(fd, buffer, len);
    for(int i = 0; i < dirs_size; i++) {
        len = snprintf(buffer, sizeof(buffer), "%s %ld\n", dirs[i].d_name, dirs[i].st_size);
        write(fd, buffer, len);
    }
    write(fd,"\n\n",2);
    if(closedir(temp_dir))
        ERR("closedir");
    if(chdir(cur_path_name))
        ERR("change working directory to default one");

}

int main(int argc, char** argv)
{
    char* path_list[MAX_SIZE] = {NULL};
    int path_number = 0;
    char* path_stdout = NULL;
    int c;
    while((c=getopt(argc,argv,"p:o:")) != -1)
    {
        switch(c)
        {
            case 'p':
                path_list[path_number] = optarg;
                path_number++;
                break;
            case 'o':
                path_stdout = optarg;
                break;
            case '?':
            default:
                usage(argv[0]);
        }
    }

    int fd = 1;
    if(path_stdout!=NULL)
    {
        if((fd = open(path_stdout,O_WRONLY | O_CREAT | O_TRUNC, 0777)) == -1)
            ERR("open");
    }
    
    for(int i = 0; i < path_number; i++)
    {
        listing(path_list[i], fd);
    }
    if(path_stdout!=NULL && close(fd))
        ERR("fclose");
    return EXIT_SUCCESS;
}
