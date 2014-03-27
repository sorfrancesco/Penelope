#include "scheduler.h"

int main(int argc, char* argv[]){
  int num=1, i, flaggen=0;
  char filename[30], filename2[30], filename3[30];

  /* Check the input parameters */
  if(argc<3){
    usage();
    return;
  }

  strcpy(filename, argv[1]);
  sprintf(filename3, "%sOUT", argv[2]);
  
  FILE* fp=fopen(argv[2], "r");
  if(fp==NULL){
    perror("Error ");
    return;
  }

  FILE* fp1=fopen(filename3, "r");
  if(fp1==NULL){
	printf("OUT file missing. All the Schedules will be generated!!!\n");
	flaggen=2;
  }

  list *t1lst, *t2lst;
  
  while(1){
	if(flaggen!=2){
		flaggen=0;	
		if(fscanf(fp1, "%d\n", &flaggen)!=1)
			break;
	}

	//Initialization of the array lastPositions
	for(i=0;i<MAX_TH; i++)
		lastPositions[i]=0;
			
	last=-1; counter=0;
	global_e=0; global_e1=0; global_e2=0; global_f=0;
	maxev=0; minev=0; t1phase2=0; t2phase2=0;
	t1id=0; t2id=0; e1=0; e=0; e2=0; f=0;

	sprintf(filename2, "%s%d", argv[1], num++);
	if(fscanf(fp, "%d %d %d %d %d %d\n", &t1id, &t2id, &e1, &e, &e2, &f)!=6)
		break;
			
	if((flaggen==0)||(flaggen==2)){			
		printf("Generating %s\n", filename2);

		ahl1=(char *) malloc(sizeof(char)*(l_num*l_num+1));
		ahl2=(char *) malloc(sizeof(char)*(l_num*l_num+1));
		t1lst=(list *) malloc(sizeof(list));
		t2lst=(list *) malloc(sizeof(list));

		find_globals(filename);
		load_array(filename);
		if(load_file_and_lists(filename, filename2, t1lst, t2lst)!=-1){ 
			schedule_lists(filename2, t1lst, t2lst);
			free(ahl1);
			free(ahl2);
		}

		t1lst=NULL;
		t2lst=NULL;
	}
  }
  fclose(fp);
  if(flaggen!=2)
	fclose(fp1);  
}

/* This function prints the usage of the program. */
void usage(){
  printf("Scheduler\n");
  printf("usage: Scheduler file(nrun) file(pattern)\n");
}

void init_AHmatrix(char** AH){
  int i;

  for(i=0; i<2; i++){
    AH[i]=(char *) malloc (sizeof(char) * (l_num*l_num +1));
    memset(AH[i], '0', l_num*l_num);
    *(AH[i]+l_num*l_num)='\0';
  }
}

/* This function checks if the Lockset for an event is empty. */
int LS_empty(char* AH){
  int i, n;
  if(strlen(AH)==1)
    if(AH[0]=='1')
      return 0;
    else 
      return 1;

  n=sqrt(strlen(AH));
  for(i=0;i<n;i++){
    if(AH[i*n+(n-i-1)]=='1')
      return 0;
  }
  return 1;
}

/* This function finds the global events. */
void find_globals(char* filename){
  FILE *fp=fopen(filename, "r");

  if(fp==NULL){    
    perror("Error ");
    return;
  }

  int threadid=0, localev=0, totalev=0;
  char pc[100], act[100], val[100];
  int LSEmpty, x1=0, x2=0;

  while(1){
    fscanf(fp, "%d %d %d %s %s %d %s\n",&threadid, &localev, &totalev, act, pc, &LSEmpty, val);
	if(localev>0){
		if(threadid==t1id){
			if(localev==e1)
				global_e1=totalev;
			if(localev<=e){
				if(localev==e)
					global_e=totalev;
				else if((localev<e)&&(LSEmpty==0))
					x1=totalev;
			}else if(localev==e2){
				global_e2=totalev;
			}
		}else if(threadid==t2id){
			if((localev<f)&&(LSEmpty==0))
				x2=totalev;
			else if (localev==f){
				global_f=totalev;
				//printf("%d %d\n", localev, threadid);
				}
		}
		if((global_e2!=0)&&(global_f!=0)){
			if(x1>x2){
				maxev=x1;
				minev=x2;
			}else{
				maxev=x2;
				minev=x1;
			}
			fclose(fp);
			return;
		}
	}
  }
}

/* This function loads the last event that each thread can execute in the array lastPositions. */
void load_array(char* filename){
  int threadid=0, localev=0, totalev=0;
  char act[100], pc[100], val[100]; 
  int LSEmpty=0;

  FILE *fp=fopen(filename, "r");
  if(fp==NULL){    
    perror("Error ");
    return;
  }

  t1phase2=t2phase2=0;

  while(fscanf(fp, "%d %d %d %s %s %d %s\n",&threadid, &localev, &totalev, act, pc, &LSEmpty, val)==7){
    if(localev>0){
		if((totalev<=minev)&&(LSEmpty==0))
			lastPositions[threadid]=localev;
		else if((threadid==t1id)&&(totalev>=minev)&&(totalev<global_e)&&(LSEmpty==0))
			t1phase2=localev;
		else if((threadid==t2id)&&(totalev>=minev)&&(totalev<global_f)&&(LSEmpty==0))
			t2phase2=localev;
		else if ((totalev>global_f)&&(totalev>global_e)){
			fclose(fp);
			return;
		}
	}
  }
}


/* This function pulls out the first part of the schedule (where all the threads have LS empty).
   Then it saves all the events related to the thread T1 and T2 in two lists t1lst, t2lst. 
   This function checks if the schedules already does the operation f between e1 and
   e2, in such case it prints out this information and terminates.*/
int load_file_and_lists(char* filename1, char* filename2, list* t1lst, list* t2lst){
  int threadid=0, localev=0, totalev=0;
  char act[100], pc[100], val[100]; 
  char* AH[2];
  int flag1=0, flag2=0, LSEmpty=0;
  list *tail1=t1lst, *tail2=t2lst;
  int Lockset[2][l_num];

/*
  if((global_f>=global_e1)&&(global_f<global_e2)){
    printf("Interleaving already scheduled!\n");
    printf("T1:%d e1=%d T2:%d f=%d T1:%d e2=%d\n\n", t1id, global_e1, t2id, global_f, t1id, global_e2);
    return -1;
  }*/

  FILE *fp=fopen(filename1, "r");
  if(fp==NULL){    
    perror("Error ");
    exit(0);
  }

  FILE *fp2=fopen(filename2, "w");
  if(fp2==NULL){    
    perror("Error ");
    exit(0);
  }

  init_AHmatrix(AH);
  init_Lockset(Lockset, l_num);
  
  while(fscanf(fp, "%d %d %d %s %s %d %s\n",&threadid, &localev, &totalev, act, pc, &LSEmpty, val)==7){
    if(localev>0){
    if(localev<=lastPositions[threadid]){
      if((last==-1)||(last==threadid)){
	last=threadid;
	counter++;
      }else{
	fprintf(fp2, "%d,%d\n",last, counter);
	last=threadid;
	counter=1;
      }
    }else if((threadid==t1id)&&(localev<=e)){
      if((t1phase2>0)&&(localev<=t1phase2)){
	if((last==-1)||(last==threadid)){
	  last=threadid;
	  counter++;
	}else{
	  fprintf(fp2, "%d,%d\n",last, counter);
	  last=threadid;
	  counter=1;
	}
      }else{//Update LS and AH  
	if(!strncmp(act, "acql", 4)){
	  if(Lockset[0][atoi(act+4)]>0)
	    Lockset[0][atoi(act+4)]++;
	  else{
	    set_Lockset(Lockset, 0, atoi(act+4));
	    add_ah(AH[0], Lockset, 0, atoi(act+4), l_num);
	    (AH[0])[atoi(act+4)*l_num+(l_num-atoi(act+4)-1)]='1';
	  }
	}else if(!strncmp(act, "rel", 3)){
	  if(Lockset[0][atoi(act+3)]>1)
	    Lockset[0][atoi(act+3)]--;
	  else{
	    del_Lockset(Lockset, 0, atoi(act+3));
	    del_ah(AH[0], Lockset, 0, atoi(act+3), l_num);
	    (AH[0])[atoi(act+3)*l_num+(l_num-atoi(act+3)-1)]='0';
	  }
	}

	list *tmplst;
	if(flag1==0)
	  tmplst=tail1;
	else
	  tmplst=(list *)malloc(sizeof(list));

	if(localev==e-1)
	  memcpy(ahl1, AH[0], strlen(AH[0]));
	
	// Check that is a release should be enough//
	//else if((localev==e)&&(strncmp(act, "acql", 4))&&(strncmp(act, "rel", 3))){
	else if((localev==e)&&(strncmp(act, "rel", 3)))
	  memcpy(ahl1, AH[0], strlen(AH[0]));
	
	tmplst->threadid=threadid;
	tmplst->localev=localev;
	tmplst->totalev=totalev;
	tmplst->ah=(char *) malloc(sizeof(char)*strlen(AH[0])+1);
	sprintf(tmplst->ah, "%s", AH[0]);
	//memcpy(tmplst->ah, AH[0], strlen(AH[0])+1);
	tmplst->action=(char *) malloc(sizeof(char)*strlen(act));
	sprintf(tmplst->action, "%s", act);
	//memcpy(tmplst->action, act, strlen(act));

	tmplst->next=NULL;
	tmplst->inc=NULL;
	tmplst->out=NULL;
	
	if(flag1==0){
	  flag1=1;
	}else{
	  tail1->next=tmplst;
	  tail1=tail1->next;
	}
      }
    }else if((threadid==t2id)&&(localev<=f)){
      if((t2phase2>0)&&(localev<=t2phase2)){
	if((last==-1)||(last==threadid)){
	  last=threadid;
	  counter++;
	}else{
	  fprintf(fp2, "%d,%d\n",last, counter);
	  last=threadid;
	  counter=1;
	}
      }else{  
	if(!strncmp(act, "acql", 4)){
	  if(Lockset[1][atoi(act+4)]>0)
	    Lockset[1][atoi(act+4)]++;
	  else{
	    set_Lockset(Lockset, 1, atoi(act+4));
	    add_ah(AH[1], Lockset, 1, atoi(act+4), l_num);
	    (AH[1])[atoi(act+4)*l_num+(l_num-atoi(act+4)-1)]='1';
	  }
	}else if(!strncmp(act, "rel", 3)){
	  if(Lockset[1][atoi(act+3)]>1)
	    Lockset[1][atoi(act+3)]--;
	  else{
	    del_Lockset(Lockset, 1, atoi(act+3));
	    del_ah(AH[1], Lockset, 1, atoi(act+3), l_num);
	    (AH[1])[atoi(act+3)*l_num+(l_num-atoi(act+3)-1)]='0';
	  }
	}

	list *tmplst;
	if(flag2==0)
	  tmplst=tail2;
	else
	  tmplst=(list *) malloc(sizeof(list));

	if(localev==f)
	  memcpy(ahl2,  AH[1], strlen(AH[1]));

	tmplst->threadid=threadid;
	tmplst->localev=localev;
	tmplst->totalev=totalev;
	tmplst->ah=(char *) malloc(sizeof(char)*strlen(AH[1])+1);
	sprintf(tmplst->ah, "%s", AH[1]);
	//memcpy(tmplst->ah, AH[1], strlen(AH[1])+1);
	tmplst->action=(char *) malloc(sizeof(char)*strlen(act));
	sprintf(tmplst->action, "%s", act);
	//memcpy(tmplst->action, act, strlen(act));
	tmplst->next=NULL;
	tmplst->inc=NULL;
	tmplst->out=NULL;
	if(flag2==0){
	  flag2=1;
	  tail2=tmplst;
	}else{
	  tail2->next=tmplst;
	  tail2=tail2->next;
	}
      }
    }
	}
  }


  fclose(fp);
  fclose(fp2);
  return 0;
}
 
void print_list(list* lst){
  list* tmp=lst;  
  
  while(tmp!=NULL){
    printf("ID=%d LOCALEV=%d GLOBALEV=%d AH=%s\n", tmp->threadid, tmp->localev, tmp->totalev, tmp->ah);
    if(tmp->inc!=NULL)
      printf("INC not NULL\n");
    if(tmp->out!=NULL)
      printf("OUT not NULL\n");
    tmp=tmp->next;
  }
}

list* find_first_acq(list* lst, int lockid){
  list* tmp=lst;  
  list* ret=NULL;
  char buff[10];
  sprintf(buff, "acql%d",lockid);

  while(tmp!=NULL){
    if(!strncmp(tmp->action, buff, strlen(buff)))
      return tmp;    
    tmp=tmp->next;
  }
  return NULL;
}

list* find_last_acq(list* lst, int lockid){
  list* tmp=lst;  
  list* ret=NULL;
  char buff[10];
  sprintf(buff, "acql%d",lockid);

  while(tmp!=NULL){
    if(!strncmp(tmp->action, buff, strlen(buff)))
      ret=tmp;    
    tmp=tmp->next;
  }
  return ret;
}

list* find_last_rel(list* lst, int lockid){
  list* tmp=lst;  
  list* ret=NULL;
  char buff[10];
  sprintf(buff, "rel%d",lockid);

  while(tmp!=NULL){
    if(!strncmp(tmp->action, buff, strlen(buff)))
      ret=tmp;    
    tmp=tmp->next;
  }
  return ret;
}

void add_edges(list* t1lst, list* t2lst){
  list *last_r, *first_a;
  int i, j, n;

  n=sqrt(strlen(ahl1));
  if(LS_empty(ahl1)==0){//LS of T1 is not empty
    for(i=0;i<n;i++)
      if((ahl1[i*n+(n-i-1)]=='1')){
	for(j=0;j<n;j++)
	  //if((i!=j)&&(ahl2[j*n+n-i-1]=='1')){
	  if((ahl2[j*n+n-i-1]=='1')){//Case T1: Acq l1 Acq l2 Rel l2 Rel l1 and T2: Acq l1
	    first_a=find_first_acq(t1lst, i);
	    last_r=find_last_rel(t2lst, i);
	    if(last_r!=NULL){
	      last_r->out=first_a;	
	      first_a->inc=last_r;
	    }	
	  }
      }
  }

  last_r=NULL; first_a=NULL;

  if(LS_empty(ahl2)==0){//LS of T2 is not empty
    for(i=0;i<n;i++)
      if((ahl2[i*n+(n-i-1)]=='1')){
	for(j=0;j<n;j++)
	  //if((i!=j)&&(ahl1[j*n+n-i-1]=='1')){
	  if((ahl1[j*n+n-i-1]=='1')){
	    first_a=find_first_acq(t2lst, i);
	    last_r=find_last_rel(t1lst, i);
	    if(last_r!=NULL){
	      last_r->out=first_a;	
	      first_a->inc=last_r;
	    }	
	  }
      }
  }

  
  //Add the edge between e and f
  list *t1tmp=t1lst;
  list *t2tmp=t2lst;
  while(t1tmp!=NULL)
    if(t1tmp->localev==e)
      break;
    else
      t1tmp=t1tmp->next;

  while(t2tmp!=NULL)
    if(t2tmp->localev==f)
      break;
    else
      t2tmp=t2tmp->next;
  

  if((t1tmp->out==NULL && t2tmp->inc==NULL)){
    t1tmp->out=t2tmp;
    t2tmp->inc=t1tmp;
  }
}

/* Given the two lists, t1lst and t2lst, with the egdes between the acq/rel on the same
lock, this function creates a file containg how the schedule should be done.
The function takes into account the original position of the events trying to save
as much as it can.*/
int schedule_lists(char* filename, list* t1lst, list* t2lst){

  FILE *fp=fopen(filename, "a");
  if(fp==NULL){    
    perror("Error ");
    return 0;
  }
  
  add_edges(t1lst, t2lst);
  list *tmp;

  while((t1lst!=NULL)&&(t2lst!=NULL)){
    if((t1lst->totalev<t2lst->totalev)&&(t1lst->inc==NULL)){
      if((t1id!=last)&&(last!=-1)){
	fprintf(fp, "%d,%d\n", last, counter);
	last=t1id;
	counter=1;
      }else{
	last=t1id;
	counter++;
      }
      if(t1lst->out!=NULL)
	(t1lst->out)->inc=NULL;
      tmp=t1lst->next;
      free(t1lst->ah);
      free(t1lst->action);
      t1lst->next=NULL;
      free(t1lst);
      t1lst=tmp;
    }else if((t1lst->totalev>t2lst->totalev)&&(t2lst->inc==NULL)){
      if((t2id!=last)&&(last!=-1)){
	fprintf(fp, "%d,%d\n", last, counter);
	last=t2id;
	counter=1;
      }else{
	last=t2id;
	counter++;
      }
      if(t2lst->out!=NULL)
	(t2lst->out)->inc=NULL;
      tmp=t2lst->next;
      free(t2lst->ah);
      free(t2lst->action);
      t2lst->next=NULL;
      free(t2lst);
      t2lst=tmp;
    }else if((t1lst->totalev<t2lst->totalev)&&(t2lst->inc==NULL)){
      if((t2id!=last)&&(last!=-1)){
	fprintf(fp, "%d,%d\n", last, counter);
	last=t2id;
	counter=1;
      }else{
	last=t2id;
	counter++;
      }
      if(t2lst->out!=NULL)
	(t2lst->out)->inc=NULL;
      tmp=t2lst->next;
      free(t2lst->ah);
      free(t2lst->action);
      t2lst->next=NULL;
      free(t2lst);
      t2lst=tmp;
    }else if((t1lst->totalev>t2lst->totalev)&&(t1lst->inc==NULL)){
      if((t1id!=last)&&(last!=-1)){
	fprintf(fp, "%d,%d\n", last, counter);
	last=t1id;
	counter=1;
      }else{
	last=t1id;
	counter++;
      }
      if(t1lst->out!=NULL)
	(t1lst->out)->inc=NULL;
      tmp=t1lst->next;
      free(t1lst->ah);
      free(t1lst->action);
      t1lst->next=NULL;
      free(t1lst);
      t1lst=tmp;
    }else{
      //CASE: (To double check)
      // T1: acql acql ..... rell rell
      // T2: acql acql ..... rell ....
      // There will be a butterfly: t1lst->inc!=NULL and t2lst->inc!=NULL.
       list *t1tmp=t1lst;
       list *t2tmp=t2lst;
       while(t1tmp!=NULL)
	 if(t1tmp->localev==e)
	   break;
	 else
	   t1tmp=t1tmp->next;

       while(t2tmp!=NULL)
	 if(t2tmp->localev==f)
	   break;
	 else
	   t2tmp=t2tmp->next;
       
       if(LS_empty(t1tmp->ah)!=0){
	 (t1lst->inc)->out=NULL;
	 t1lst->inc=NULL;
       }else if(LS_empty(t2tmp->ah)!=0){
	 (t2lst->inc)->out=NULL;
	 t2lst->inc=NULL;
       }else{
	 printf("Problem with the schedule: %s", filename);
	 fclose(fp);
	 remove(filename);
	 print_list(t1lst);
	 print_list(t2lst);
	 return;
       }
    }
  } 

  while(t1lst!=NULL){
    if((t1id!=last)&&(last!=-1)){
      fprintf(fp, "%d,%d\n", last, counter);
      last=t1id;
      counter=1;
    }else{
      last=t1id;
      counter++;
    }
    tmp=t1lst->next;
    free(t1lst->ah);
    free(t1lst->action);
    t1lst->next=NULL;
    free(t1lst);
    t1lst=tmp;
  }

  while(t2lst!=NULL){
    if((t2id!=last)&&(last!=-1)){
      fprintf(fp, "%d,%d\n", last, counter);
      last=t2id;
      counter=1;
    }else{
      last=t2id;
      counter++;
    }
    tmp=t2lst->next;
    free(t2lst->ah);
    free(t2lst->action);
    t2lst->next=NULL;
    free(t2lst);
    t2lst=tmp;
  }

  fprintf(fp, "%d,%d\n", last, counter);
  fclose(fp);
}


