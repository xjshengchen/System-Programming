/*b02902027陳昇*/
#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>

typedef struct {
    char c_time_string[100];
    char filename[1024];
} TimeInfo;

void mmap_write();

char name[1024];
int main()
{
	char tmp[1024]={0};
	read(0,name,sizeof(name));
	
	int fd=open(name,O_RDONLY);
	if(fd<0){
		fprintf(stderr, "404 not found\n");
		mmap_write();
		exit(2);															  //  2 == not found
	}													       
	int n;
	while((n=read(fd,tmp,sizeof(tmp))) > 0)
		write(1,tmp,n);
	mmap_write();
	exit(0);																  //  0 == normal
}

void mmap_write() 
{
    int fd,i;
    time_t current_time;
    char c_time_string[100];
    TimeInfo *p_map;
    const char  *file ="time_test";   
    fd = open(file, O_RDWR | O_TRUNC | O_CREAT, 0777); 
    if(fd<0)
    {
        perror("open");
        exit(-1);
    }
    lseek(fd,sizeof(TimeInfo),SEEK_SET);
    write(fd,"",1);
    p_map = (TimeInfo*) mmap(0, sizeof(TimeInfo), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    current_time = time(NULL);
    strcpy(c_time_string, ctime(&current_time));
    memcpy(p_map->c_time_string, &c_time_string , sizeof(c_time_string));
    memcpy(p_map->filename, &name ,sizeof(name));
    munmap(p_map, sizeof(TimeInfo));
    return 0;
}