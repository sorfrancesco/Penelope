#include "downward.h"
#include <time.h>
//NOTE: All the implementation has to be made faster, instead to save the first item for each
//list in the array. Save the last element and go back using the pred pointer.
//Expensive in 

int TCBA[MAX_TH];

// This function initialites the Vector Clocks list, of the array lastPositions and the variables used
void init(listVC VClock[], listVC VClock1[]){
	int i, j;

	t1id=0; t2id=0; e1=0; e=0; e2=0; f=0;
		
	for(i=0;i<MAX_TH; i++){
		downT1[i]=0;
		downT2[i]=0;
		lastPositions[i]=0;
		TCBA[i]=0;
	}
	
	for(i=0;i<t_num;i++){
		VClock[i].Gevent=0;
		VClock[i].Levent=0;
		VClock[i].VC=(int *) malloc (sizeof(int) * t_num);
		for(j=0;j<t_num;j++){
			VClock[i].VC[j]=0;
		}
		VClock[i].next=NULL;
		VClock[i].pred=NULL;
		
		VClock1[i].Gevent=0;
		VClock1[i].Levent=0;
		VClock1[i].VC=(int *) malloc (sizeof(int) * t_num);
		for(j=0;j<t_num;j++){
			VClock1[i].VC[j]=0;
		}
		VClock1[i].next=NULL;
		VClock1[i].pred=NULL;
	}
	
	lastOp=(int *) malloc (v_num * sizeof(int));
	lastTh=(int *) malloc (v_num * sizeof(int));
	for(i=0;i<v_num;i++){
		lastOp[i]=-1;
		lastTh[i]=-1;
	}
}

// This function prints a list of vector clocks
void print_VC(listVC VClock){
	int i=0;
	listVC *tmp=&VClock;
	while(tmp!=NULL){
		printf("GlobalEv = %d\n", tmp->Gevent);
		printf("LocalEv = %d\n", tmp->Levent);
		for(i=0;i<t_num;i++){
			printf("%d ", tmp->VC[i]);
		}
		printf("\n\n");
		tmp=tmp->next;
	}
}

// This function frees the list of vector clocks
void free_VC(listVC VClock[]){
	int i=0;
	listVC *tmp;
	
	for(i=0;i<t_num;i++){
		tmp=&VClock[i];
		while(tmp!=NULL){
			if(tmp->VC!=NULL)
				free(tmp->VC);
			tmp=tmp->next;
		}
		VClock[i].next=NULL;
	}
}


// This function prints the usage of the program
void usage(){
  printf("downward\n");
  printf("usage: downward file(nrun) file(pattern)\n");
}

// This function finds the downward closure for ev in thread t
void find_down(listVC VClock[], int t, int ev, int *res){
	int i, k, done[t_num], maxgev[t_num];
	int indmax=0, max=0, flag=0, count=0, count1=0;
	/*
	for(i=0;i<t_num;i++){
		printf("T%d\n",i);
		print_VC(VClock[i]);
	}
	printf("\n");
	*/
	listVC *tmp=(listVC *) malloc (sizeof(listVC) * t_num);

	// The array tmp will contain the last vector clock for each list
	for(i=0;i<t_num;i++){
		memcpy(&tmp[i], &VClock[i], sizeof(VClock[i]));
		if(i==t){
			while(tmp[i].Levent<ev)
				memcpy(&tmp[i], tmp[i].next, sizeof(tmp[i]));						
			done[i]=1;
		}else{ 
			while(tmp[i].next!=NULL)
				memcpy(&tmp[i], tmp[i].next, sizeof(tmp[i]));
			for(k=0;k<t_num;k++){
				if(tmp[i].VC[k]!=0){
					done[i]=0;
					break;
				}else
					done[i]=1;
			}
		}
		maxgev[i]=tmp[i].Gevent;
	}

	while(1){
		indmax=flag=max=count=0;
		for(i=0;i<t_num;i++){
			if(done[i]==0){
				flag=1;
				if(tmp[i].Gevent>max){
					max=tmp[i].Gevent;
					indmax=i;
				}
			}else
				count++;
		}			
		if(!flag) break;
		
		count1=0;
		for(i=0;i<t_num;i++){
			if(done[i]==1){
				for(k=0;k<t_num;k++){
					if(tmp[i].VC[k]<tmp[indmax].VC[k]){
						count1++;
						break;
					}
				}
			}
		}
		if(count==count1){
			if(tmp[indmax].pred!=NULL){
				memcpy(&tmp[indmax], tmp[indmax].pred, sizeof(tmp[indmax]));
				if(tmp[indmax].Levent==0)
					done[indmax]=1;
				else
					maxgev[indmax]=tmp[indmax].Gevent;
			}else{
				printf("Error un find_cut function.\n");
				exit(-1);
			}
		}else{
			done[indmax]=1;
		}
	}
			
	for(i=0;i<t_num;i++)
		res[i]=tmp[i].Levent;
}

// This function finds the cuts
void find_cut(listVC VClock[]){
	listVC *tmp=(listVC *) malloc (sizeof(listVC)*t_num);
	int i, j, flag=0;
	/*
		for(i=0;i<t_num;i++){
		printf("T%d\n",i);
		print_VC(VClock[i]);
	}
	printf("\n");
	*/
	// The array tmp will contain the last vector clock for each list
	for(i=0;i<t_num;i++){
		memcpy(&tmp[i], &VClock[i], sizeof(VClock[i]));
		while(tmp[i].next!=NULL)
			memcpy(&tmp[i], tmp[i].next, sizeof(tmp[i]));
	}
	while(1){
		for(i=0;i<t_num;i++){
			for(j=0;j<t_num;j++){
				if(i!=j){
					if(tmp[i].VC[j]>tmp[j].VC[j]){
						flag=1;
						if(tmp[i].pred!=NULL){
							(tmp[i].pred)->next=NULL;
							free(tmp[i].VC);
							memcpy(&tmp[i], tmp[i].pred, sizeof(tmp[i]));
							flag=1;
							i=j=t_num;
						}else
							printf("ERROR!\n");
					}	
				}
			}
		}
		if(!flag)
			break;
		else
			flag=0;
	}
	for(i=0;i<t_num;i++){
		//lastPositions[i]=tmp[i].Levent;
		//printf("%d ", lastPositions[i]);
		lastPositions[i]=0;
	}
}
/* recursive version */
/*
// This function finds the cuts
void find_cut(listVC VClock[]){
	listVC *tmp=(listVC *) malloc (sizeof(listVC)*t_num);
	int i, j;
	
	// The array tmp will contain the last vector clock for each list
	for(i=0;i<t_num;i++){
		memcpy(&tmp[i], &VClock[i], sizeof(VClock[i]));
		while(tmp[i].next!=NULL)
			memcpy(&tmp[i], tmp[i].next, sizeof(tmp[i]));
	}
	for(i=0;i<t_num;i++){
		for(j=0;j<t_num;j++){
			if(i!=j){
				if(tmp[i].VC[j]>tmp[j].VC[j]){
					if(tmp[i].pred!=NULL){
						(tmp[i].pred)->next=NULL;
						free(tmp[i].VC);
						find_cut(VClock);
						return;
					}else
						printf("ERROR!\n");
				}
			}
		}
	}
	for(i=0;i<t_num;i++){
		lastPositions[i]=tmp[i].Levent;
		printf("% d", lastPositions[i]);
	}
	printf("/n");
}
*/

//OPTIMIZATION: KEEP A POINTER TO THE END OF EACH LIST
// This function adds an a vector clock for tid and ev=lev to the list of vector clocks
void insert_VC(int tid, int lev, int gev, listVC VClock[], int* VCthr){
	int i=0;

	listVC* tmp=&VClock[tid];
	while(tmp->next!=NULL)
		tmp=tmp->next;

	listVC* tmp2=(listVC*) malloc (sizeof(listVC));
	tmp2->VC=(int*) malloc (sizeof(int) * t_num);
	for(i=0;i<t_num;i++)
		tmp2->VC[i]=VCthr[tid*t_num+i];
	tmp2->Gevent=gev;
	tmp2->Levent=lev;
	tmp2->pred=tmp;
	tmp2->next=NULL;
	tmp->next=tmp2;
	return;
}

void change_VC(int tid, int lev, int gev, listVC VClock[], int* VCthr){
	int i=0;

	VClock[tid].Gevent=gev;
	VClock[tid].Levent=gev;
	VClock[tid].pred=NULL;
	VClock[tid].next=NULL;
	for(i=0;i<t_num;i++)
		VClock->VC[i]=VCthr[tid*t_num+i];
		
	return;
}

void load_and_generate(char* filename, listVC VClock[], listVC VClock1[]){
  int i,j;
  char buf[100], pc[100], val[100], act[100];
  int threadid, localev, totalev, x, LSEmpty;
  int flage2=0; // Signals that e2 in T1 is reached
  int flagf=0;  // Signals that f in T2 is reached
  int flag1=0; // Signals that e1-1 in T1 is reached
  int flag2=0; // Signals that f-1 in T2 is reached
  int size=v_num+l_num;

   printf("");

  FILE* fp=fopen(filename, "r");  
  if(fp==NULL){
    perror("Error ");
    return;
  }

  // Vector Clocks for the shared variables and locks used for the downward closure
  int *VCvar=(int *) malloc (sizeof(int) * size * t_num);
  for(i=0;i<size;i++)
	for(j=0;j<t_num;j++)
		VCvar[i*t_num+j]=0;
		
  // Vector Clocks for the threads used for the downward closure
  int *VCthr=(int *) malloc (sizeof(int) * t_num * t_num);
  for(i=0;i<t_num;i++)
	for(j=0;j<t_num;j++)
		VCthr[i*t_num+j]=0;
		
  // Vector Clocks for the shared variables and locks used for the cut
  int *VCvar1=(int *) malloc (sizeof(int) * v_num * t_num);
  for(i=0;i<v_num;i++)
	for(j=0;j<t_num;j++)
		VCvar1[i*t_num+j]=0;
		
  // Vector Clocks for the threads used for the cut
  int *VCthr1=(int *) malloc (sizeof(int) * t_num * t_num);
  for(i=0;i<t_num;i++)
	for(j=0;j<t_num;j++)
		VCthr1[i*t_num+j]=0;
		
  while(fscanf(fp, "%d %d %d %s %s %d %s\n",&threadid, &localev, &totalev, act, pc, &LSEmpty, val)==7){
	if(localev>0){
		if(!strncmp(act, "MW", 2)){
			x=(atoi(act+2));
			VCthr[(threadid*t_num)+threadid]++;
			//VCvar[x*t_num+threadid]++;
			for(i=0;i<t_num;i++){
				if(VCthr[threadid*t_num+i]<VCvar[x*t_num+i]){
					if(lastOp[x]==1)
						if(i==lastTh[x] && lastTh[x]!=threadid)
							VCthr[threadid*t_num+i]=VCvar[x*t_num+i]-1;
						else
							VCthr[threadid*t_num+i]=VCvar[x*t_num+i];						
				}else
					VCvar[x*t_num+i]=VCthr[threadid*t_num+i];
			}
			lastOp[x]=1;
			lastTh[x]=threadid;

			VCthr1[(threadid*t_num)+threadid]++;
			for(i=0;i<t_num;i++){
				if(VCthr1[threadid*t_num+i]<VCvar1[x*t_num+i])
					VCthr1[threadid*t_num+i]=VCvar1[x*t_num+i];
				else
					VCvar1[x*t_num+i]=VCthr1[threadid*t_num+i];
			}
		}else if(!strncmp(act, "MR", 2)){
			x=(atoi(act+2));
			VCthr[(threadid*t_num)+threadid]++;
			for(i=0;i<t_num;i++){
				if(VCthr[threadid*t_num+i]<VCvar[x*t_num+i])
					VCthr[threadid*t_num+i]=VCvar[x*t_num+i];
			}
			lastOp[x]=0;
			VCthr1[(threadid*t_num)+threadid]++;
			for(i=0;i<t_num;i++){
				if(VCthr1[threadid*t_num+i]<VCvar1[x*t_num+i])
					VCthr1[threadid*t_num+i]=VCvar1[x*t_num+i];
				else
					VCvar1[x*t_num+i]=VCthr1[threadid*t_num+i];
			}
		}else if(!strncmp(act, "acql", 4)){
			x=(atoi(act+4));
			VCthr[(threadid*t_num)+threadid]++;
			for(i=0;i<t_num;i++){
				if(VCthr[threadid*t_num+i]<VCvar[(x+v_num)*t_num+i])
					VCthr[threadid*t_num+i]=VCvar[(x+v_num)*t_num+i];
				else
					VCvar[(x+v_num)*t_num+i]=VCthr[threadid*t_num+i];
			}
		}else if(!strncmp(act, "rel", 3)){
			x=(atoi(act+3));
			VCthr[(threadid*t_num)+threadid]++;
			for(i=0;i<t_num;i++){
				if(VCthr[threadid*t_num+i]<VCvar[(x+v_num)*t_num+i])
					VCthr[threadid*t_num+i]=VCvar[(x+v_num)*t_num+i];
				else
					VCvar[(x+v_num)*t_num+i]=VCthr[threadid*t_num+i];
			}
		}else if(!strncmp(act, "TC", 2)){
			x=(atoi(act+2));
			VCthr[(threadid*t_num)+threadid]++;
			insert_VC(threadid, localev, totalev, VClock, VCthr);
			for(i=0;i<t_num;i++)
				VCthr[x*t_num+i]=VCthr[threadid*t_num+i];
			VCthr[(x*t_num)+x]=1;
			change_VC(x, 0, 0, VClock, VCthr);
			
			////
			VCthr1[(threadid*t_num)+threadid]++;
			//insert_VC(threadid, localev, totalev, VClock1, VCthr1);
			for(i=0;i<t_num;i++)
				VCthr1[x*t_num+i]=VCthr1[threadid*t_num+i];
			VCthr1[(x*t_num)+x]=1;
			//change_VC(x, 0, 0, VClock1, VCthr1);
			if(((threadid==t1id)&&(localev<=e2))||((threadid==t2id)&&(localev<=f)))
				TCBA[threadid]++;
		}else if(!strncmp(act, "BA", 2)){
			if(((threadid==t1id)&&(localev<=e2))||((threadid==t2id)&&(localev<=f)))
				TCBA[threadid]++;
		}

		if(strncmp(act, "TC", 2))
			insert_VC(threadid, localev, totalev, VClock, VCthr);

		if(threadid==t1id){
			if(localev<=e1-1){
				if(localev==e1-1)
					flag1=1;
				if(LSEmpty==0)
					insert_VC(threadid, localev, totalev, VClock1, VCthr1);
			}else if(localev==e2){
				flage2=1;
				find_down(VClock, t1id, e2, downT1);
			}
		}else if(threadid==t2id){
			if(localev<=f-1){
				if(localev==f-1)
					flag2=1;
				if(LSEmpty==0)
					insert_VC(threadid, localev, totalev, VClock1, VCthr1);
			}else if(localev==f){
				flagf=1;
				find_down(VClock, t2id, f, downT2);
			}
		}else{
			if((LSEmpty==0) && (!flag1 || !flag2))
				insert_VC(threadid, localev, totalev, VClock1, VCthr1);
		}

		if(flagf&&flage2)
			break;
	}
  }
  free(VCvar);
  free(VCthr);
  free(VCvar1);
  free(VCthr1);
  fclose(fp);
} 


//This function finds the starting value for each variable and generates the regionB
void SMT_utils(char* filename, char* filename3, char* filename4, char* filename6){
  int threadid=0, localev=0, totalev=0, LSEmpty=0, i=0, j=0;
  char act[100], pc[100], val[MAX_VAL_LEN], lastVal[v_num][MAX_VAL_LEN];
  int flag1=0, flag2=0;
  int last=-1, counter=0, totalcounter=0;
  
  
  for(i=0;i<v_num;i++)
    sprintf(lastVal[i], "_\0"); 
  
  FILE *fp=fopen(filename, "r");
  if(fp==NULL){    
    perror("Error ");
    exit(0);
  }

  FILE *fp3=fopen(filename3, "w");
  if(fp3==NULL){    
    perror("Error ");
    exit(0);
  }
  FILE *fp4=fopen(filename4, "w");
  if(fp4==NULL){    
    perror("Error ");
    exit(0);
  }
  
  FILE *fp6=fopen(filename6, "w");
  if(fp6==NULL){    
    perror("Error ");
    exit(0);
  }
  

  while((fscanf(fp, "%d %d %d %s %s %d %s\n",&threadid, &localev, &totalev, act, pc, &LSEmpty, val)==7)){  
	// The events of regionA are discarded, only the values for the variables are collected
    if((localev<=lastPositions[threadid])&&(strncmp(act, "TC", 2))&&(strncmp(act, "BA", 2))){///////NEW
		if(localev>0){
			if((last==-1)||(last==threadid)){
				last=threadid;
				counter++;
			}else{
				totalcounter+=counter;
				fprintf(fp6, "%d,%d\n",last, counter);
				last=threadid;
				counter=1;
			}
		}
	
		if(!strncmp(act, "MR", 2)||!strncmp(act, "MW", 2)){
			sprintf(lastVal[atoi(act+2)], "%s", val);
			lastVal[atoi(act+2)][strlen(val)]='\0';
		}
    }else{
	  if(!strncmp(act, "MR", 2)||!strncmp(act, "MW", 2)){
		if(!strncmp(lastVal[atoi(act+2)], "_", 1)){
          sprintf(lastVal[atoi(act+2)], "%s", val);
          lastVal[atoi(act+2)][strlen(val)]='\0';
        }
		
		if(localev<=downT1[threadid]||localev<=downT2[threadid])
			fprintf(fp4, "** %d %c%c %s %s\n", threadid, act[0], act[1], (act+2),  val);
      }else if (!strncmp(act, "rel", 3)){
		if(localev<=downT1[threadid]||localev<=downT2[threadid])
			fprintf(fp4, "** %d SR %s\n", threadid, (act+3));	
      }else if (!strncmp(act, "acql", 4)){
		if(localev<=downT1[threadid]||localev<=downT2[threadid])
			fprintf(fp4, "** %d SA %s\n", threadid, (act+4));
      }else if (!strncmp(act, "TC", 2)){
		if(localev<=downT1[threadid]||localev<=downT2[threadid])
			fprintf(fp4, "** %d TC %s\n", threadid, (act+2));
	  }else if (!strncmp(act, "BA", 2)){
		if(localev<=downT1[threadid]||localev<=downT2[threadid])
			fprintf(fp4, "** %d BA %s\n", threadid, (act+2));
	  }else{
		if(localev<=downT1[threadid]||localev<=downT2[threadid])
			fprintf(fp4, "** %d %s\n", threadid, act);
	  }
    }
	if(!flag1 && threadid==t1id && localev>=e2)
		flag1=1;
	if(!flag2 && threadid==t2id && localev>=f)
		flag2=1;
	if(flag1 && flag2)
		break;
  }

  for(i=0;i<t_num;i++)
	totalcounter-=lastPositions[i];
	
  if(totalcounter<0)
	fprintf(fp6, "%d,%d\n",last, counter);
  else if(totalcounter>0)
	printf("Error in the prefix\n");
	
  for(i=0;i<v_num;i++)
    fprintf(fp3, "%s\n", lastVal[i]);
  
  
  fclose(fp);
  fclose(fp3);
  fclose(fp4);
  fclose(fp6);
}

int main(int argc, char* argv[]){
  listVC VClock[t_num];
  listVC VClock1[t_num];
  int num=1;
  char filename[100]; //filename will contain the file.nrun
  char filename2[100]; //filename2 will contain the file.nrun_val_#
  char filename3[100]; //filename3 will contain the last values of the variables
  char filename4[100]; //filename4 will contain the events for the SMT solver
  char filename5[100]; //filename5 will contain the events e1 e f....for the SMT solver
  char filename6[100]; //filename6 will contain the file.nrun#_pre
  char filename7[100]; //filename7 will contain the file.patternOUT

  // Check the input parameters 
  if(argc<3){
    usage();
    return;
  }

  strcpy(filename, argv[1]);
  FILE* fp2=fopen(argv[2], "r");
  if(fp2==NULL){
    perror("Error ");
    return;
  }

  sprintf(filename5, "%s_SMT", argv[2]);
  FILE *fp5=fopen(filename5, "w");
  if(fp5==NULL){    
    perror("Error ");
    exit(0);
  } 

  sprintf(filename7, "%sOUT", argv[2]);
  FILE *fp7=fopen(filename7, "w");
  if(fp7==NULL){    
    perror("Error ");
    exit(0);
  } 
  
  while(1){
    // Initialization 
    init(VClock, VClock1);
	
    sprintf(filename2, "%s%d", filename, num);
    sprintf(filename3, "%s_val_%d", filename, num);
    sprintf(filename4, "%s_smt_%d", filename, num);
	sprintf(filename6, "%s%d_pre", filename, num);

    if(fscanf(fp2, "%d %d %d %d %d %d\n", &t1id, &t2id, &e1, &e, &e2, &f)!=6)
      break;
	
	// Loads all the vector clocks for the events of interest
	load_and_generate(filename, VClock, VClock1);
	
	// Finds the cut points
	find_cut(VClock1);
	
	// Calculates the values for the variables
	SMT_utils(filename, filename3, filename4, filename6);

	fprintf(fp7, "0\n"); 
	printf("Schedule %d DONE!!!\n", num++);

    fprintf(fp5, "%d %d %d %d %d %d\n", t1id, t2id, e1-lastPositions[t1id]+TCBA[t1id], e-lastPositions[t1id]+TCBA[t1id], e2-lastPositions[t1id]+TCBA[t1id], f-lastPositions[t2id]+TCBA[t2id]);
	free_VC(VClock);
	//exit(0);
  }
  fclose(fp2);
  fclose(fp5);
  fclose(fp7);
}


