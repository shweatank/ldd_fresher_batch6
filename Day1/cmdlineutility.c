#include<stdio.h>
#include<dirent.h>
#include<stdlib.h>

int main(int argc,char *argv[])
{
   struct dirent *entry;
   DIR *dir;
   char *path=(argc>1)?argv[1]:".";
   dir=opendir(path);
   if(dir==NULL)
   {
     perror("opendir");
     return 1;
   }

   while((entry=readdir(dir))!=NULL)
   {
	   printf("%s\n",entry->d_name);
   }
   closedir(dir);

   return 0;
}
