/*b04902027 陳昇*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>

int j_free[15]={0},judge_num;
int schedule[4845][4],s_n=0,solution[5];
void rec(int now,int a,int N)
{
	if(now==4){
		for(int i=0;i<4;i++)
			schedule[s_n][i]=solution[i];
		s_n++;
		return;
	}
	for(int i=a;i<=N;i++){
		solution[now]=i;
		rec(now+1,i+1,N);
	}
	return;
}
typedef struct p
{
	int id,score;	
}P;
int cmp(const P *a,const P *b)
{
	return a->score < b->score;
}
int main(int argc,char** argv)
{
	pid_t childpid[30];

	judge_num=atoi(argv[1]);
	int player_num=atoi(argv[2]);
	int big_[20][2],small_[20][2];
	rec(0,1,player_num);
	if(judge_num > s_n) judge_num=s_n;

	P player[25];
	for(int i=1;i<=player_num;i++){
		player[i].id=i;
		player[i].score=0;
	}

	int thisrun=0;
	for(int i=1;i<=judge_num;i++){
		pipe(big_[i]);			// distribute judge
		pipe(small_[i]);
		pid_t pid=fork();
		if(pid==0){
			close(big_[i][1]);
			close(small_[i][0]);
			dup2(big_[i][0],0);
			close(big_[i][0]);
			dup2(small_[i][1],1);
			close(small_[i][1]);
			char execname[100]="judge\0";
			char arg[2][100]={0};
			strcpy(arg[0],execname);
			sprintf(arg[1],"%d",i);
			execl(execname,(char *)arg[0],(char *)arg[1],NULL);
		}
		else childpid[i]=pid;
		close(big_[i][0]);
		close(small_[i][1]);
		char player_ids[100]={0};
		sprintf(player_ids,"%d %d %d %d\n",schedule[i-1][0],schedule[i-1][1],schedule[i-1][2],schedule[i-1][3]);
		write(big_[i][1],player_ids,sizeof(player_ids));
		thisrun++;
	}

	int finish=0;
	
	while(finish != s_n){
		fd_set rset;
		FD_ZERO(&rset);
		struct timeval tv;
    	tv.tv_sec=0;
    	tv.tv_usec=1000;
    	for(int i=1;i<=judge_num;i++)
    		if(j_free[i]==0)						// set those not yet finished
    			FD_SET(small_[i][0],&rset);
    	if(select(FD_SETSIZE,&rset,NULL,NULL,&tv)==0) continue;  // no finish
		 // judge response?		
		for(int i=1;i<=judge_num;i++){
			if(FD_ISSET(small_[i][0],&rset)==1){		// judge finished
				char result[100]={0};
				read(small_[i][0],result,sizeof(result));
				//write(1,result,sizeof(result));
				int id[4]={0},rank[4]={0};
				sscanf(result,"%d %d\n%d %d\n%d %d\n%d %d\n",&id[0],&rank[0],&id[1],&rank[1],&id[2],&rank[2],&id[3],&rank[3]);
				for(int j=0;j<4;j++)
					player[id[j]].score += (4-rank[j]);
				
				finish++;

				char term[100]={0};
				if(thisrun == s_n){               // all distribute completed
					strcpy(term,"-1 -1 -1 -1\n");
					write(big_[i][1],term,sizeof(term));
					close(big_[i][1]);
					close(small_[i][0]);
					int status;
					waitpid(childpid[i],&status,NULL);    // prevent zombie
					j_free[i]=1;
				}
				else{
					sprintf(term,"%d %d %d %d\n",schedule[thisrun][0],schedule[thisrun][1],schedule[thisrun][2],schedule[thisrun][3]);
					write(big_[i][1],term,sizeof(term));
					thisrun++;
				}
				//printf("finish: %d\n",finish);
			}
		}
	}

	qsort(&player[1],player_num,sizeof(P),cmp);
	for(int i=1;i<=player_num;i++)
		printf("%d %d\n",player[i].id,player[i].score);
	return 0;
}