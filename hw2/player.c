/*b04902027 陳昇*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

int main(int argc,char** argv)
{
	srand(time(NULL));

	int judge_id=atoi(argv[1]);
	char play_index=*argv[2];
	int random_key=atoi(argv[3]);
	char j_p[256]={0},p_j[256]={0};
	sprintf(j_p,"judge%d_%c.FIFO",judge_id,play_index);
	int J_P=open(j_p,O_RDONLY);
	sprintf(p_j,"judge%d.FIFO",judge_id);
	int P_J=open(p_j,O_WRONLY);
	for(int i=1;i<=20;i++){
		//while(1);
		int choose=(rand()%2==1)? 3:5;
		char bufin[100]={0},bufout[100]={0};
		sprintf(bufout,"%c %d %d\n",play_index,random_key,choose);
		write(P_J,bufout,sizeof(bufout));
		if(i!=20)
			read(J_P,bufin,sizeof(bufin));		  //every round result
	}
	exit(0);
}