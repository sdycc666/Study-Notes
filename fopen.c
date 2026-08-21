#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>






int main(int argc, char const *argv[])
{
    DIR *dir = opendir("/home/sdy/0524/linux-7.0.10");
    if(!dir)
    {
        perror("无法打开\n");
        return EXIT_FAILURE;
    }





}