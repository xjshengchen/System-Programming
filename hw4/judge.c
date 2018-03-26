#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

int main()
{
	int cnt=0;
	FILE *a,*b;
	a=fopen("./ans.csv","r");
	b=fopen("./submission.csv","r");
	char buf[100],aa[100],bb[100];
	fgets(buf,100,a);
	fgets(buf,100,b);
	for(int i=0;i<25008;i++){
		fgets(aa,100,a);
		fgets(bb,100,b);
		if(!strcmp(aa,bb)) cnt++;
	}
	printf("%lf\n",(double) cnt / 25008);
	return 0;
}