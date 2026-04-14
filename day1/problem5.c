#include<stdio.h>
#include<dirent.h>
#include<string.h>
#include<errno.h>
int main(int argc,char *argv[])
{
	if(argc<2)
	{
		puts("Insufficient input");
		return 1;
	}
	const char*path=(argc==1)?".":argv[1];
	DIR *dir=opendir(path);
	if(dir==NULL)
	{
		perror("opendir");
		return 0;
	}
	struct dirent *entry;
	while((entry=readdir(dir))!=NULL)
	{
		printf("%s\n",entry->d_name);
	}
	closedir(dir);
}
