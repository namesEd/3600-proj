//Edward Kyles
//Fall 2022
//demonstrate pipes

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


int main(void) 
{
    struct t_data {
        int a;
        char c; 
    } data;
    printf("size of my stucture: %lu\n", sizeof(data));
    data.a = 1234; 
    data.c = 'A';
    //char buf[100];
    //buf[0] = 'a';
    //*buf = 'a';
    //printf("char: %c\n", *buf);
    //setup pipe
    int fd[2];
    pipe(fd);
    //char c = 'a';
    //int num = 12345;
    write(fd[1], &data, sizeof(data));
    //int dest; 
    struct t_data dstruct; 
    read(fd[0], &dstruct, sizeof(data));
    printf("values from pipe: %i %c\n", dstruct.a, dstruct.c);

    return 0;
}
