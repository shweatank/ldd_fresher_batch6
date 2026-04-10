#include<stdio.h>
#include<stdlib.h>
#include <dirent.h>
int main(){
  struct dirent *entry;
  DIR *dp;
  dp=opendir("./");
  if(dp == NULL) {
        perror("Error: Unable to open directory");
        return EXIT_FAILURE;
  }
  while((entry=readdir(dp))!=NULL){
    if(entry->d_name[0]=='.'){
      continue;
    }	  
    printf("%s %ld\n",entry->d_name,entry->d_ino);
  }
  closedir(dp);
}
