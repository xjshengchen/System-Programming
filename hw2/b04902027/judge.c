/*b04902027 陳昇*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>

int cmp(const void *a,const void *b){return *(int *)a < *(int *)b;}
int main(int argc,char** argv)
{
	srand(time(NULL));
	pid_t childpid[4];

    while(1){
		fd_set r_big;
		struct timeval tv0;
    	tv0.tv_sec=0;
    	tv0.tv_usec=0;
		FD_ZERO(&r_big);
		FD_SET(0,&r_big);
		select(1,&r_big,NULL,NULL,&tv0);
		if(!FD_ISSET(0,&r_big))
			continue;
	    
		char player_ids[100]={0};
		int player_id[4];
		read(0,player_ids,sizeof(player_ids));
		sscanf(player_ids,"%d %d %d %d\n",&player_id[0],&player_id[1],&player_id[2],&player_id[3]);
	    //0=A,1=B,2=C,3=D 
		if(player_id[0]==-1 && player_id[1]==-1 && player_id[2]==-1 && player_id[3]==-1)
			break;

	    int judge_id=atoi(argv[1]);
		char p_j[256]={0},j_p[4][256]={0};
		sprintf(p_j,"judge%d.FIFO",judge_id);
		mkfifo(p_j,0777);
		for(int i=0;i<4;i++){
			sprintf(j_p[i],"judge%d_%c.FIFO",judge_id,'A'+i);
			mkfifo(j_p[i],0777);
		}

		int r[4];
		r[0]=rand()%65536;
		r[1]=rand()%65536;
		r[2]=rand()%65536;
		r[3]=rand()%65536;
		for(int i=0;i<4;i++){
			pid_t pid=fork();
			if(pid==0){
				char execname[100]="player\0";
				char arg[4][100]={0};
				strcpy(arg[0],execname);
				sprintf(arg[1],"%d",judge_id);
				sprintf(arg[2],"%c",'A'+i);
				sprintf(arg[3],"%d",r[i]);
				execl(execname,(char *)arg[0],(char *)arg[1],(char *)arg[2],(char *)arg[3],NULL);
			}
			else childpid[i]=pid;
		}

		int J_P[4];
		for(int i=0;i<4;i++)
			J_P[i]=open(j_p[i],O_WRONLY);
		int P_J=open(p_j,O_RDONLY);

		int score[4]={0};
		int penalty[4]={0};
		int wait_num=4;

		for(int ii=1;ii<=20;ii++){
			int p_choose[4]={0},result[4]={0},pass[4]={0},pass_num=0;
			clock_t t1=clock();    // set alarm

			while(pass_num != wait_num && (clock()-t1)/(double)(CLOCKS_PER_SEC) <= 3){
				char player_index;
				int random,choose;

				fd_set rset;
				FD_ZERO(&rset);
				FD_SET(P_J,&rset);
				struct timeval tv;
	    		tv.tv_sec=0;
	    		tv.tv_usec=0;
				select(P_J+1,&rset,NULL,NULL,&tv);
				if(!FD_ISSET(P_J,&rset)) continue;    // no choose

				char buf[100]={0};
				read(P_J,buf,sizeof(buf));
				sscanf(buf,"%c %d %d\n",&player_index,&random,&choose);
				if(r[player_index-'A']==random && penalty[player_index-'A']==0){
					p_choose[player_index-'A']=choose;
					result[player_index-'A']=choose;
					pass[player_index-'A']=1;
					pass_num++;
				}   
			}
			for(int i=0;i<4;i++)
				if(pass[i]==0 && penalty[i]==0){
					penalty[i]=1;
					wait_num--;
				}
			
			for(int i=0;i<4;i++)
				for(int j=0;j<4;j++)
					if(i!=j && p_choose[i]==p_choose[j])
						result[i]=result[j]=0;
			for(int i=0;i<4;i++)
				score[i]+=result[i];
			
			if(ii!=20){
				char sresult[100]={0};
				sprintf(sresult,"%d %d %d %d\n",p_choose[0],p_choose[1],p_choose[2],p_choose[3]);
				for(int i=0;i<4;i++)
					write(J_P[i],sresult,sizeof(sresult));
			}
		}
		for(int i=0;i<4;i++)
			close(J_P[i]);
		close(P_J);
		int status;
		for(int i=0;i<4;i++){
			if(penalty[i]==0)
				waitpid(childpid[i],&status,NULL);	
			else{
				kill(childpid[i],SIGKILL);
				waitpid(childpid[i],&status,NULL);
			}
		}

		int sortscore[4]={0},rank[4]={0};
		for(int i=0;i<4;i++)
			sortscore[i]=score[i];
		qsort(sortscore,4,sizeof(int),cmp);
		for(int i=3;i>0;i--){
			if(sortscore[i]==-1) continue;
			int j=i-1;
			while(sortscore[j]==sortscore[i]){
				sortscore[j]=-1;
				j--;
			}
		}
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				if(score[i]==sortscore[j])
					rank[i]=j+1;
		char ranking[100]={0};
		sprintf(ranking,"%d %d\n%d %d\n%d %d\n%d %d\n",player_id[0],rank[0],player_id[1],rank[1],player_id[2],rank[2],player_id[3],rank[3]);
		write(1,ranking,sizeof(ranking));
	}

	exit(0);
}