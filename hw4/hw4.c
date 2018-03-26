#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

#define MAXN 20170118
#define PICKN 32
#define MAXTREE 12000

char train_dir[100], test_dir[100], output[100];
int tree_number, thread_number; 
int pick[MAXTREE][PICKN];

typedef struct
{
	int id;
	double feature[33];
	int label;
}data;
data training_data[25150], testing_data[25008];

typedef struct n
{
	struct n *l, *r;
	double threshold;
	int label, dim;
}Node;
Node *root[MAXTREE], node[MAXTREE*PICKN], *ptr = node;

typedef struct
{
	int id, dim;
}sort_id;

typedef struct
{
	int start, end;	
}D;

void read_file();
void pick_random();
Node* init_node();
int cmp(const void *a, const void *b);
void build_tree(Node* p, int arr[], int n);
double gini(int start, int end, sort_id member[]);
void *decide(void *d);
void submit();

int main(int argc, char **argv)
{
	sprintf(train_dir, "%s%s", argv[2],"/training_data");
	sprintf(test_dir, "%s%s", argv[2], "/testing_data");
	strcpy(output, argv[4]);
	tree_number = atoi(argv[6]);
	thread_number = atoi(argv[8]);

	read_file();
	pick_random();
	
	pthread_t tid[MAXTREE];
	int start = 0, size = 25008/thread_number, end = 0;
	for(int i=0 ; i<thread_number ; i++){
		D tmp;
		start = end;
		end = start + size;
		if(i =  thread_number - 1)
			end = 25008;
		tmp.start = start;
		tmp.end = end;
		pthread_create(&tid[i], NULL, decide, (void *) &(tmp));
	}
	/*
	for(int i=0 ; i<thread_number ; i++)
		pthread_join(tid[i],NULL);
	*/
	submit();
	return 0;
}
void read_file()
{
	FILE *train_fp, *test_fp;
	train_fp = fopen(train_dir, "r");	
	test_fp = fopen(test_dir, "r");
	for(int i=0 ; i<25150 ; i++){
		fscanf(train_fp, "%d", &training_data[i].id);
		for(int j=0 ; j<33 ; j++)
			fscanf(train_fp, "%lf", &training_data[i].feature[j]);
		fscanf(train_fp, "%d", &training_data[i].label);
	}	
	for(int i=0 ; i<25008 ; i++){
		fscanf(test_fp, "%d", &testing_data[i].id);
		for(int j=0 ; j<33 ; j++)
			fscanf(test_fp, "%lf", &testing_data[i].feature[j]);
	}
	fclose(train_fp);
	fclose(test_fp);
	return;
}
void pick_random()
{
	for(int i=0 ; i<tree_number ; i++){
		srand(time(NULL));
		int a, used[25150] = {0};
		for(int j=0 ; j<PICKN ; j++){
			a = rand()%25150;
			if(used[a]) continue;
			pick[i][j] = a;
		}
		root[i] = init_node();
		build_tree(root[i], pick[i], PICKN);
	}
	return;
}
Node* init_node()
{
	Node *p = ptr++;
	p->l = NULL;
	p->r = NULL;
	p->label = -1;
	return p;
}
void build_tree(Node* p, int arr[], int n)
{
	int min_dim, min_cut;
	double min_gini = -1, min_l, min_r;
	double g_l, g_r, g;
	
	sort_id ids[n];
	for(int i=0 ; i<n ; i++)
		ids[i].id = arr[i];
	for(int i=0 ; i<33 ; i++){
		for(int j=0 ; j<n ; j++)
			ids[j].dim = i;
		qsort(ids, n, sizeof(sort_id), cmp);
		for(int j=1 ; j<n ; j++){
			g_l = gini(0, j, ids);
			g_r = gini(j, n, ids);
			g = g_l + g_r;
			if(min_gini == -1 || g < min_gini ){
				min_gini = g;
				min_l = g_l;
				min_r = g_r;
				min_dim = i;
				min_cut = j;
			}
		}
	}
	
	for(int i=0 ; i<n ; i++)
		ids[i].dim = min_dim;
	qsort(ids, n, sizeof(sort_id), cmp);
	p->threshold = (training_data[ ids[min_cut - 1].id ].feature[min_dim] + training_data[ ids[min_cut].id ].feature[min_dim])/2; 
	p->dim = min_dim;
	//printf("dim:%d cut:%d gini:%lf l:%lf r:%lf threshold:%f\n",min_dim,min_cut,min_gini,min_l,min_r,p->threshold);
	
	if(min_l == 0){
		p->l = init_node();
		p->l->label = training_data[ ids[0].id ].label;
	}
	else{
		p->l = init_node();
		int left[min_cut];
		for(int i=0 ; i<min_cut ; i++)
			left[i] = ids[i].id;
		build_tree(p->l, left, min_cut);
	}
	if(min_r == 0){
		p->r = init_node();
		p->r->label = training_data[ ids[n - 1].id ].label;
	}
	else{
		p->r = init_node();
		int right[ n - min_cut ];
		for(int i=min_cut ; i<n ; i++)
			right[ i - min_cut ] = ids[i].id;
		build_tree(p->r, right, n - min_cut);
	}
	
	return;
}
int cmp(const void *a, const void *b)
{
	sort_id aa = *(sort_id *)a;
	sort_id bb = *(sort_id *)b;
	return training_data[ aa.id ].feature[ aa.dim ] > training_data[ bb.id ].feature[ bb.dim ];
}
double gini(int start, int end, sort_id member[])
{
	double total=0, sum_1=0, sum_0=0;
	for(int i=start ; i<end ;i++){
		if(training_data[ member[i].id ].label == 1) sum_1++;
		else sum_0++;
	}
	double f1 = sum_1/(double)(end - start), f0 = sum_0/(double)(end - start);
	return f1*(1-f1) + f0*(1-f0);
}
void *decide(void *d)
{
	D dd = *((D *)d);
	int start = dd.start, end = dd.end;
	for(int i=start ; i<end ; i++){
		int ones = 0, zeros = 0;
		for(int j=0 ; j<tree_number ; j++){
			Node* p = root[j];
			while(p->label == -1){
				if(testing_data[i].feature[ p->dim ] < p->threshold)
					p = p->l;
				else
					p = p->r;
			}
			if(p->label == 1) ones++;
			else if(p->label == 0) zeros++;
		}
		if(ones > zeros) testing_data[i].label = 1;
		else testing_data[i].label = 0;
		//printf("%d %d\n",i,testing_data[i].label);
	}
	pthread_exit(NULL);
}
void submit()
{
	FILE *submit_fp;
	submit_fp = fopen(output, "w+");
	fprintf(submit_fp, "id,label\n");
	for(int i=0 ; i<25008 ; i++)
		fprintf(submit_fp,"%d,%d\n", i, testing_data[i].label);
	fclose(submit_fp);
	return;
}