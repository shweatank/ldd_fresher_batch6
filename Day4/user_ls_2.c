#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    struct dirent *entry;
    DIR *dp;
    //char *path = (argc > 1) ? argv[1] : ".";
    dp=opendir(argv[1]);
    while((entry=readdir(dp))!=NULL){
      if(entry->d_name[0]!='.'){
        printf("%s ",entry->d_name);
      }
    }
    closedir(dp);
}

