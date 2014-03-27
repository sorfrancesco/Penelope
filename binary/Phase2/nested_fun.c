#include "nested.h"
#include "VectorClocks.h"
#include <unistd.h>

extern int verbose;
extern int pcexpansion;
extern int acqhistory;
extern int reduction;
extern VECTNODE *vcl;
extern VECTNODE *vcl1;
CLASSMETHLIST cmlst;
int ah_checks, countpc, ll_num, pattred;


/* This function records a single read/write event in a transaction. */
void insert_single_R_W(mainlist* A, int ev, char* ah, int pc){
  /* The list is empty. */
  if(A->ah==NULL){
    A->ah=(char *)malloc(sizeof(char) * ll_num);
    if(A->ah==NULL)
      perror("Error ");
    memcpy(A->ah, ah, ll_num);
    A->event=ev;
    A->pc=pc;
    A->next=NULL;
    return;
  }else if((pcexpansion)&&(A->pc==pc)&&(!strncmp(A->ah, ah, ll_num))){
    return;
  }else if((!pcexpansion)&&(!strncmp(A->ah, ah, ll_num))){
    return;
  }else if(A->next!=NULL){
    insert_single_R_W(A->next, ev, ah, pc);
  }else{
    A->next=(mainlist *) malloc(sizeof(mainlist));
    if(A->next==NULL) 
      perror("Error ");
    (A->next)->ah=(char *)malloc (sizeof(char) * ll_num);
    memcpy((A->next)->ah, ah, ll_num);
    (A->next)->event=ev;
    (A->next)->pc=pc;
    (A->next)->next=NULL;
    return;
  }
}

/* This function records multiple read/write events in a transaction. */
void insert_AAWW(mainlist* A, mainlist* B, mainlist* C, int ev, char* ah, int pc){
  if(A->ah==NULL){
    A->ah=(char *)malloc(sizeof(char) * ll_num);
    if(A->ah==NULL)
      perror("Error ");
    memset(A->ah, 'f', ll_num);
    A->event=ev;
    A->pc=pc;
    if(!find_AH(C, ah, pc)){
      mainlist *tmp=(mainlist *) malloc (sizeof(mainlist));
      tmp->ah=(char *)malloc(sizeof(char) * ll_num);
      if(tmp->ah==NULL)
	perror("Error ");
      memcpy(tmp->ah, ah, ll_num);
      tmp->event=ev;
      tmp->pc=pc;
      tmp->next=NULL;
      A->next=tmp;
    }else
      A->next=NULL;
    return;
  }else if(B->ah==NULL){
    B->ah=(char *)malloc(sizeof(char) * ll_num);
    if(B->ah==NULL)
      perror("Error ");
    memset(B->ah, 'l', ll_num);
    B->event=ev;
    B->pc=pc;
    B->next=NULL;
    return;
  }else{
    if(B->next!=NULL){
      mainlist *tmp=(mainlist *) malloc (sizeof(mainlist));
      if(tmp==NULL)
	perror("Error ");
      tmp=B->next;
      merge_list(A, tmp);
    }
    memset(B->ah, 'l', ll_num);
    B->event=ev;
    B->pc=pc;
    B->next=NULL;
    }
    return;
}

/* This function records the end transaction in the WW list. */
void end_transactionWW(mainlist lock_mat[][INF][v_num], int pid, int x){
  //WW1 is empty. 
  if(lock_mat[pid][2][x].ah==NULL) return;
  //WW2 is empty. 
  if((lock_mat[pid][3][x]).ah==NULL){
    if((lock_mat[pid][2][x]).next!=NULL)
      free_list((lock_mat[pid][2][x]).next);
    free((lock_mat[pid][2][x]).ah);
    (lock_mat[pid][2][x]).next=NULL;
    (lock_mat[pid][2][x]).ah=NULL;
    (lock_mat[pid][2][x]).pc=0;
    (lock_mat[pid][2][x]).event=0;
    return;
  }

  //WW2 is not empty. Save just the first element in WW2.
  if((lock_mat[pid][3][x]).next!=NULL){
    free_list((lock_mat[pid][3][x]).next);
    (lock_mat[pid][3][x]).next=NULL;    
  }
  
  //Reach the end of the WW1 list.
  mainlist* tmp=&(lock_mat[pid][2][x]);
  while(tmp->next!=NULL)
    tmp=tmp->next;

  //Add WW2 to WW1
  tmp->next=(mainlist *) malloc (sizeof(mainlist));
  if(tmp->next==NULL)
    perror("Error ");
  (tmp->next)->ah=(char *)malloc(sizeof(char) * ll_num);
  if((tmp->next)->ah==NULL)
    perror("Error ");
  memcpy(((tmp->next)->ah), (lock_mat[pid][3][x]).ah, ll_num);
  (tmp->next)->event=(lock_mat[pid][3][x]).event;
  (tmp->next)->pc=(lock_mat[pid][3][x]).pc;
  (tmp->next)->next=NULL;
  
  //Empty WW2
  free((lock_mat[pid][3][x]).ah);
  (lock_mat[pid][3][x]).ah=NULL;
  (lock_mat[pid][3][x]).event=0;
  (lock_mat[pid][3][x]).pc=0;
  (lock_mat[pid][3][x]).next=NULL;

  char *l=(char *)malloc(sizeof(char) * ll_num);
  memset(l, 'l', ll_num);
  if(strncmp((lock_mat[pid][2][x].next)->ah, l, ll_num)){
    mainlist *tmp2=&(lock_mat[pid][6][x]);
    if(tmp2->ah==NULL){//GlobalWW is empty
      tmp2->ah=(char *)malloc(sizeof(char) * ll_num);
      if(tmp2->ah==NULL)
	perror("Error ");
      memcpy(tmp2->ah, (lock_mat[pid][2][x]).ah, ll_num);
      tmp2->event=(lock_mat[pid][2][x]).event;
      tmp2->pc=(lock_mat[pid][2][x]).pc;
      tmp2->next=(lock_mat[pid][2][x]).next;
    }else{ //GlobalWW is not empty
      while(tmp2->next!=NULL)
	tmp2=tmp2->next; 
      
      //Add the *node to the GlobalWW
      mainlist *tmp3=(mainlist *) malloc (sizeof(mainlist));
      if(tmp3==NULL)
	perror("Error ");
      tmp3->ah=(char *)malloc(sizeof(char) * ll_num);
      if(tmp3->ah==NULL)
	perror("Error ");
      memset(tmp3->ah, '*', ll_num);
      tmp3->event=0;
      tmp3->pc=0;
      tmp2->next=tmp3;
      
      tmp3->next=(mainlist *) malloc (sizeof(mainlist));
      if(tmp3->next==NULL)
	perror("Error ");
      (tmp3->next)->ah=(char *)malloc(sizeof(char) * ll_num);
      if((tmp3->next)->ah==NULL)
	perror("Error ");
      memcpy(((tmp3->next)->ah), (lock_mat[pid][2][x]).ah, ll_num);
      (tmp3->next)->event=(lock_mat[pid][2][x]).event;
      (tmp3->next)->pc=(lock_mat[pid][2][x]).pc;
      (tmp3->next)->next=(lock_mat[pid][2][x]).next;
    }
  }

  //Empty WW1
  free((lock_mat[pid][2][x]).ah);
  (lock_mat[pid][2][x]).ah=NULL;
  (lock_mat[pid][2][x]).event=0;
  (lock_mat[pid][2][x]).pc=0;
  (lock_mat[pid][2][x]).next=NULL;
  free(l);
  return;
}

/* This function records the end transaction in the AA list. */
void end_transactionAA(mainlist lock_mat[][INF][v_num], int pid, int x){
  //AA1 is empty. 
  if(lock_mat[pid][4][x].ah==NULL) return;
  //AA2 is empty. 
  if((lock_mat[pid][5][x]).ah==NULL){
    if((lock_mat[pid][4][x]).next!=NULL)
      free_list((lock_mat[pid][4][x]).next);
    free((lock_mat[pid][4][x]).ah);
    (lock_mat[pid][4][x]).next=NULL;
    (lock_mat[pid][4][x]).ah=NULL;
    (lock_mat[pid][4][x]).event=0;
    (lock_mat[pid][4][x]).pc=0;
    return;
  }
  
  //AA2 is not empty. Save just the first element in AA2.
  if((lock_mat[pid][5][x]).next!=NULL){
    free_list((lock_mat[pid][5][x]).next);
    (lock_mat[pid][5][x]).next=NULL;    
  }
  
  //Reach the end of the AA1 list.
  mainlist* tmp=&(lock_mat[pid][4][x]);
  while(tmp->next!=NULL)
    tmp=tmp->next;
  
  //Add AA2 to AA1
  tmp->next=(mainlist *) malloc (sizeof(mainlist));
  if(tmp->next==NULL)
    perror("Error ");
  (tmp->next)->ah=(char *)malloc(sizeof(char) * ll_num);
  if((tmp->next)->ah==NULL)
    perror("Error ");
  memcpy(((tmp->next)->ah), (lock_mat[pid][5][x]).ah, ll_num);
  (tmp->next)->event=(lock_mat[pid][5][x]).event;
  (tmp->next)->pc=(lock_mat[pid][5][x]).pc;
  (tmp->next)->next=NULL;

  //Empty AA2
  free((lock_mat[pid][5][x]).ah);
  (lock_mat[pid][5][x]).ah=NULL;
  (lock_mat[pid][5][x]).event=0;
  (lock_mat[pid][5][x]).pc=0;
  (lock_mat[pid][5][x]).next=NULL;

  char *l=(char *)malloc(sizeof(char) * ll_num);
  memset(l, 'l', ll_num);
  if(strncmp((lock_mat[pid][4][x].next)->ah, l, ll_num)){
    mainlist *tmp2=&(lock_mat[pid][7][x]);
    if(tmp2->ah==NULL){//GlobalAA is empty
      tmp2->ah=(char *)malloc(sizeof(char) * ll_num);
      if(tmp2->ah==NULL)
      perror("Error ");
      memcpy(tmp2->ah, (lock_mat[pid][4][x]).ah, ll_num);
      tmp2->event=(lock_mat[pid][4][x]).event;
      tmp2->pc=(lock_mat[pid][4][x]).pc;
      tmp2->next=(lock_mat[pid][4][x]).next;
    }else{ //GlobalAA is not empty
      while(tmp2->next!=NULL)
	tmp2=tmp2->next; 

      //Add the *node to the GlobalAA list.
      mainlist *tmp3=(mainlist *) malloc (sizeof(mainlist));
      if(tmp3==NULL)
	perror("Error ");
      tmp3->ah=(char *)malloc(sizeof(char) * ll_num);
      if(tmp3->ah==NULL)
	perror("Error ");
      memset(tmp3->ah, '*', ll_num);
      tmp3->event=0;
      tmp3->pc=0;
      tmp2->next=tmp3;
      
      tmp3->next=(mainlist *) malloc (sizeof(mainlist));
      if(tmp3->next==NULL)
	perror("Error ");
      (tmp3->next)->ah=(char *)malloc(sizeof(char) * ll_num);
      if((tmp3->next)->ah==NULL)
	perror("Error ");
      memcpy(((tmp3->next)->ah), (lock_mat[pid][4][x]).ah, ll_num);
      (tmp3->next)->event=(lock_mat[pid][4][x]).event;
      (tmp3->next)->pc=(lock_mat[pid][4][x]).pc;
      (tmp3->next)->next=(lock_mat[pid][4][x]).next;
    }
  }

  //Empty AA1
  free((lock_mat[pid][4][x]).ah);
  (lock_mat[pid][4][x]).ah=NULL;
  (lock_mat[pid][4][x]).event=0;
  (lock_mat[pid][4][x]).pc=0;
  (lock_mat[pid][4][x]).next=NULL;     
  free(l);
  return;
}

/* This function records the acq/rel in the WW list. */
void AcqRelGWW(mainlist lock_mat[][INF][v_num], int pid, int x, int ev, char* ah, int pc){ 
  /* WW1 is not empty and Curr_AH is not in WW1, in WW2 and in GlobalWW */
  if((lock_mat[pid][2][x].ah!=NULL)&&(!find_AH(&lock_mat[pid][2][x], ah, pc))&&
     (!find_AH(&lock_mat[pid][3][x], ah, pc))&&(!find_AH(&lock_mat[pid][6][x], ah, pc))){
    mainlist *tmp;
    if(lock_mat[pid][3][x].ah==NULL)  
      tmp=&(lock_mat[pid][2][x]);
    else
      tmp=&(lock_mat[pid][3][x]);
    
    while(tmp->next!=NULL)
      tmp=tmp->next; 
    
    tmp->next=(mainlist *) malloc (sizeof(mainlist));
    if(tmp->next==NULL)
      perror("Error ");
    (tmp->next)->ah=(char *)malloc(sizeof(char) * ll_num);
    if((tmp->next)->ah==NULL)
      perror("Error ");
    memcpy((tmp->next)->ah, ah, ll_num);
    (tmp->next)->event=ev;
    (tmp->next)->pc=pc;
    (tmp->next)->next=NULL;
  }
  return;
}

/* This function records the acq/rel in the AA list. */
void AcqRelGAA(mainlist lock_mat[][INF][v_num], int pid, int x, int ev, char* ah, int pc){ 
  /* AA1 is not empty and Curr_AH is not in AA1, in AA2 and in GlobalAA */
  if((lock_mat[pid][4][x].ah!=NULL)&&(!find_AH(&lock_mat[pid][4][x], ah, pc))&&
     (!find_AH(&lock_mat[pid][5][x], ah, pc))&&(!find_AH(&lock_mat[pid][7][x], ah, pc))){
    mainlist *tmp;
    if(lock_mat[pid][5][x].ah==NULL)
      tmp=&(lock_mat[pid][4][x]);
    else
      tmp=&(lock_mat[pid][5][x]);

    while(tmp->next!=NULL)
      tmp=tmp->next; 
    
    tmp->next=(mainlist *) malloc (sizeof(mainlist));
    if(tmp->next==NULL)
      perror("Error ");
    (tmp->next)->ah=(char *)malloc(sizeof(char) * ll_num);
    if((tmp->next)->ah==NULL)
      perror("Error ");
    memcpy((tmp->next)->ah, ah, ll_num);
    (tmp->next)->event=ev;
    (tmp->next)->pc=pc;
    (tmp->next)->next=NULL;  
  }
  return;
}

/* This function, given two lists it checks the compatibility between the AHs and the LSs. */
int Check_list(mainlist* A, mainlist* B, int t1, int t2, int var, TUPLELIST *tplst, FILE* fp, PCLIST PClist){ 
  if(A==NULL || B==NULL || A->ah==NULL || B->ah==NULL)
    return 0;

  int i, j,  flagLS=0, flagAH=0, b;
  int total=0, tmpEV, tmpPC;
  char buff[100], b1[100];
  mainlist *tmpA, *tmpB, *tmp;

  char *f=(char *)malloc(sizeof(char) * ll_num);
  memset(f, 'f', ll_num);
  char *ls=(char *)malloc(sizeof(char) * ll_num);
  memset(ls, 'l', ll_num);
  char *star=(char *)malloc(sizeof(char) * ll_num);
  memset(star, '*', ll_num);

  tmpA=A;
  tmpB=B;
  while(tmpA!=NULL){
    tmpB=B;
    
    while(1){
      if(!strncmp(tmpA->ah, f, ll_num)){
		tmpEV=tmpA->event;
		tmpPC=tmpA->pc;
		tmpA=tmpA->next;
		break;
      }else if((!strncmp(tmpA->ah, ls, ll_num))||(!strncmp(tmpA->ah, star, ll_num))){
		if(tmpA->next!=NULL){
		tmpEV=tmpA->next->next->event;
		tmpPC=tmpA->next->next->pc;
		tmpA=tmpA->next->next;
		}else
			return total;
      }else
		break;
	}
	
    while(tmpB!=NULL){
      flagLS=0;
      flagAH=0;

      //LSs compatibility check.
      for(i=0;i<l_num;i++)
		if(((tmpA->ah)[(l_num-1)*(i+1)]=='1')&&((tmpB->ah)[(l_num-1)*(i+1)]=='1')){	   
			flagLS=1;//Lockset disjointed flag
			break;
		}

      if(!flagLS){
		if(acqhistory)
			for(i=0;(i<l_num && !flagAH);i++)
				for(j=0;(j<l_num && !flagAH);j++) 
					if(i!=(l_num-j-1)) 
						if((((tmpA->ah)[i*l_num+j])=='1')&&(((tmpB->ah)[(l_num-j-1)*l_num+(l_num-i-1)])=='1'))
							flagAH=1;//AHs compatible flag

		//AHs compatibility check.
		if(acqhistory && flagAH){
			ah_checks++;

		}else if((!acqhistory)||(acqhistory && !flagAH)){
			tmp=tmpA;
			sprintf(buff, "%d %d %d %d", t1, t2, tmpEV, tmp->event);
			while((tmpA->next!=NULL)&&(strncmp(tmpA->next->ah, star, ll_num)))
				tmpA=tmpA->next;
			
			if(!(e_before_f(vcl, t_num, t2, tmpB->event, t1, tmpEV)||e_before_f(vcl, t_num, t1, tmpA->event, t2, tmpB->event))
			&&!(e_before_f(vcl1, t_num, t2, tmpB->event, t1, tmpEV)||e_before_f(vcl1, t_num, t1, tmpA->event, t2, tmpB->event))){
					total++;
				if(reduction){
					if(!check_tuple(tplst, var, pcint_to_pcstr(&PClist, tmpPC), 
					    pcint_to_pcstr(&PClist, tmpB->pc), pcint_to_pcstr(&PClist, tmpA->pc))){
						fprintf(fp, "%s %d %d\n", buff, tmpA->event, tmpB->event);							
						pattred++;
						printf("+");
					}else
						printf("-");
				}else{
					fprintf(fp, "%s %d %d\n", buff, tmpA->event, tmpB->event);
				}
				printf("e1: T%3d ev=%7d || f: T:%3d ev=%7d || e2: T%3d ev=%7d\n", t1, tmpEV, t2, tmpB->event, t1, tmpA->event);
				if(verbose){
					sscanf(pcint_to_pcstr(&PClist, tmpPC), "%d:%s", &b, b1);
					printf("%s:%s || ", read_Classmeth(b), b1);
					sscanf(pcint_to_pcstr(&PClist, tmpB->pc), "%d:%s", &b, b1);
					printf("%s:%s || ", read_Classmeth(b), b1);
					sscanf(pcint_to_pcstr(&PClist, tmpA->pc), "%d:%s", &b, b1);
					printf("%s:%s\n", read_Classmeth(b), b1);
					printAHLS(tmp->ah, tmpB->ah);
				}
			}//End of happen-before check
		tmpA=tmp;
	}
      }//End if(!flagLS)
      tmpB=tmpB->next;
    }//End While
    tmpA=tmpA->next;
  }//End While
  
  free(f);
  free(ls);
  free(star);
  return total;
}

/* This function prints the usage of the program. */
void usage(){
  printf("NESTEDCheck\n");
  printf("usage: NESTEDCheck [-a -p -r -v] input_file \n");
  printf("option -a: activate the acquisition history compatibility check\n");
  printf("option -p: deactivate the PC expansion\n");
  printf("option -r: activate the PC reduction\n");
  printf("option -v: increment the verbosity of the output\n");
}

/* This function initialites some global variables. */
void init(){
  countpc=0;
  ll_num=l_num*l_num;
  pattred=0;
}

void init_AHmatrix(char** AH){
  int i;

  for(i=0; i<t_num; i++){
    AH[i]=(char *) malloc (sizeof(char) * (ll_num +1));
    memset(AH[i], '0', ll_num);
    *(AH[i]+ll_num)='\0';
  }
}

/* This function frees the memory for a list. */
void free_list(mainlist* A){
  mainlist* tmp;
  tmp=A->next;
  
  free(A->ah);
  free(A);
  if(tmp!=NULL)
    free_list(tmp);
  return;    
}

/* This function given a list and an AH, returns 1 if the AH belongs to the list and 0 otherwise */
//SEGMENTATION FAULT HERE WHEN STATISTICS ARE PRINTED
int find_AH(mainlist* A, char* ah, int pc){
  if(A==NULL||A->ah==NULL) 
    return 0;
  mainlist* tmp=A;

  while((tmp!=NULL)&&(tmp->ah!=NULL)&&(ah!=NULL)){
    if((pcexpansion)&&(tmp->pc==pc)&&(!strncmp(tmp->ah, ah, ll_num))){     
      return 1;
    }else if((!pcexpansion)&&(!strncmp(tmp->ah, ah, ll_num))){      
      return 1;
    }else{
      if(tmp->next!=NULL){
	if(tmp->next->ah!=NULL)
	  tmp=tmp->next;
      }else
	return 0;    
    }
  }
  return 0;
}

/* This function merges two lists A and B. Moreover, it removes the duplicates and free the memory for B. */
void merge_list(mainlist *A, mainlist *B){
  if(B==NULL)
    return;

  mainlist *tmp=B->next;
  if(!find_AH(A, B->ah, B->pc)){
    mainlist *tmp2=A;
    while(tmp2->next!=NULL)
      tmp2=tmp2->next;

    tmp2->next=B;
    if(tmp2->next->next!=NULL)
      tmp2->next->next=NULL;
  }else{
    B->next=NULL;
    free(B->ah);
    free(B);
  }
  if(tmp)
    merge_list(A, tmp);

  return;
}

/* This function is like merge_list but it doesn't erase the B list. It is used for 
   statistics generation. */
void merge_listbis(mainlist *A, mainlist *B){
  if(B==NULL||B->ah==NULL)
    return;

  if(A==NULL||A->ah==NULL){
    A->ah=(char *)malloc(sizeof(char) * ll_num);
    if(A->ah==NULL)
      perror("Error ");
    memcpy(A->ah, B->ah, ll_num);
    A->event=B->event;
    A->pc=B->pc;
    A->next=NULL; 
  }else{
    if(!find_AH(A, B->ah, B->pc)){
      mainlist *tmp=(mainlist *) malloc (sizeof(mainlist));
      if(tmp==NULL)
	perror("Error ");
      tmp->ah=(char *)malloc(sizeof(char) * ll_num);
      if(tmp->ah==NULL)
	perror("Error ");
      memcpy(tmp->ah, B->ah, ll_num);
      tmp->event=B->event;
      tmp->pc=B->pc;
      tmp->next=NULL;
      
      mainlist *tmp2=A;
      while(tmp2->next!=NULL)
	tmp2=tmp2->next;
      
      tmp2->next=tmp;
    }
  }
  if(B->next!=NULL)
    merge_listbis(A, B->next);
  return;
}

void Check_matrix(mainlist lock_mat[][INF][v_num], FILE* fp, PCLIST PClist){ 
  int i, j, x, n;
  int ww=0, aa=0, total=0;
  int wwred=0, aared=0, totalred=0, tmp;
  TUPLELIST tplst;

  cmlst.head=NULL;
  if(verbose){
    init_Classmeth();
  }

  ah_checks=0;
  tplst.head=NULL;
    
  for(i=0;i<t_num;i++)
    for(j=0;j<t_num;j++)
      if(i!=j){
	for(x=0;x<v_num;x++){
	  tmp=pattred;
	  n=Check_list(&lock_mat[i][6][x], &lock_mat[j][0][x], i, j, x, &tplst, fp, PClist);
	  if(n>0){
	    printf("W R W : %d on variable %d \n\n", n, x);
	    ww+=n;
	    wwred=wwred+pattred-tmp;
	  }
	  total+=n;
	  totalred=totalred+pattred-tmp;
	  tmp=pattred;
	  n=Check_list(&lock_mat[i][7][x], &lock_mat[j][1][x], i, j, x, &tplst, fp, PClist);
	  if(n>0){
	    printf("A W A : %d on variable %d  \n\n", n, x);
	    aa+=n;
	    aared=aared+pattred-tmp;
	  }
	  total+=n;
	  totalred=totalred+pattred-tmp;
	  //n=Check_list(&lock_mat[i][6][x], &lock_mat[j][1][x], i, j, fp);
	  //if(n>0)
	  //printf("W W W : %d \n", n);
	  //total+=n;
	  //n=Check_list(&lock_mat[i][7][x], &lock_mat[j][0][x], i, j, fp);
	  // if(n>0)
	  // printf("A R A : %d \n", n);
	  //total+=n;
	}
      }
  printf("Total patterns found: %d (W R W: %d / A W A: %d)\n", total, ww, aa);
  if(reduction){
    printf("Total patterns with reduction found: %d (W R W: %d / A W A: %d)\n", totalred, wwred, aared);
  }
  // Check AHs
  if(acqhistory && ah_checks>0)
    printf("%d passed the intersection test but not the compatibility check\n", ah_checks);
}
  
void Print_List(mainlist *a, PCLIST* PClist){ 
  if(a==NULL||a->ah==NULL){
    printf("\n");
    return;
  }
  int i,j;
  printf("(Ev=%8d,AH=",a->event);
  for(i=0;i<l_num;i++)
    for(j=l_num-1;j>=0;j--)
      printf("%c", a->ah[i*l_num+j]);
  printf(" PC=%s)\n", pcint_to_pcstr(PClist, a->pc));
  Print_List(a->next, PClist);
}

void Print_All_Lists(mainlist lock_mat[][INF][v_num], int lnum, PCLIST* PClist){ 
  int j, x;
    for(x=0;x<v_num;x++)
      for(j=0;j<t_num;j++){
	printf("Thread %3d VAR=%3d\n", j, x);
	printf("Read List: \n");
	Print_List(&lock_mat[j][0][x], PClist);
	printf("Write List: \n");
	Print_List(&lock_mat[j][1][x], PClist);
	printf("WW List: \n");
	Print_List(&lock_mat[j][6][x], PClist);
	printf("AA List: \n");
	Print_List(&lock_mat[j][7][x], PClist);	    
      }
}

void Print_Stats(mainlist lock_mat[][INF][v_num], int lnum){ 
  int j, x, tmp=0;
  int maxR=0, maxW=0, maxWW=0, maxAA=0;
  int nnulR=0, nnulW=0, nnulWW=0, nnulAA=0;
  int totR=0, totW=0, totWW=0, totAA=0;

  for(j=0;j<t_num;j++)
    for(x=0;x<v_num;x++){
      //Read List
      tmp=Count_List(&lock_mat[j][0][x], 0);
      if(maxR<tmp) maxR=tmp;
      if(tmp>0){ 
	totR+=tmp;
	nnulR++;
      }

      //Write List
      tmp=Count_List(&lock_mat[j][1][x], 0);
      if(maxW<tmp) maxW=tmp;
      if(tmp>0){ 
	totW+=tmp;
	nnulW++;
      }
      
      //WW List
      tmp=Count_List(&lock_mat[j][6][x], 0);
      if(maxWW<tmp) maxWW=tmp;
      if(tmp>0){ 
	totWW+=tmp;
	nnulWW++;
      }
      
      //AA List
      tmp=Count_List(&lock_mat[j][7][x], 0);
      if(maxAA<tmp) maxAA=tmp;
	if(tmp>0){ 
	  totAA+=tmp;
	  nnulAA++;
	}
    }
  
  printf("=================================================================\n");
  printf("Statistics: \n");
  printf("Read List : NNull=%3d MAX=%5d AVG=%.2f\n", nnulR, maxR, (double)totR/(nnulR));
  printf("Write List: NNull=%3d MAX=%5d AVG=%.2f\n", nnulW, maxW, (double)totW/(nnulW));
  printf("WW List   : NNull=%3d MAX=%5d AVG=%.2f\n", nnulWW, maxWW, (double)totWW/(nnulWW));
  printf("AA List   : NNull=%3d MAX=%5d AVG=%.2f\n", nnulAA, maxAA, (double)totAA/(nnulAA));
  printf("=================================================================\n");    
  
  mainlist *total=(mainlist *)malloc(sizeof(mainlist));
  
  for(j=0;j<t_num;j++)
    for(x=0;x<v_num;x++){
	merge_listbis(total, &lock_mat[j][0][x]);
	merge_listbis(total, &lock_mat[j][1][x]);
	merge_listbis(total, &lock_mat[j][6][x]);
	merge_listbis(total, &lock_mat[j][7][x]);
      }
  printf("%d different AHs present.\n",Count_List(total, 0)); 
  return;
}

int Count_List(mainlist *a, int c){ 
  if(a==NULL||a->ah==NULL){
    return c;
  }
  if((!strncmp(a->ah, "*", 1))||(!strncmp(a->ah, "f", 1))||(!strncmp(a->ah, "l", 1)))
    Count_List(a->next, c);
  else
    Count_List(a->next, c+1);
}

void printAHLS(char* AH, char* AH1){
  int i,j,flag;

  flag=0;
  printf("Lockset(e)={");
  for(i=0;i<l_num;i++)
    for(j=l_num-1;j>=0;j--)
      if ((i==(l_num-1-j))&&(AH[i*l_num+j]=='1'))
	if(flag==0)
	  printf("l%d",i);
	else 
	  printf(", l%d",i);
  printf("}");

  flag=0;
  printf(" AH(e)={");
  for(i=0;i<l_num;i++)
    for(j=l_num-1;j>=0;j--)
      if ((i!=(l_num-1-j))&&(AH[i*l_num+j]=='1'))
	if(flag==0)
	  printf("l%d->l%d", i, (l_num-1-j));
	else
	  printf(", l%d->l%d", i, (l_num-1-j));
  printf("}\n");

  flag=0;
  printf("Lockset(f)={");
  for(i=0;i<l_num;i++)
    for(j=l_num-1;j>=0;j--)
      if ((i==(l_num-1-j))&&(AH1[i*l_num+j]=='1'))
	if(flag==0)
	  printf("l%d",i);
	else 
	  printf(", l%d",i);
  printf("}");

  flag=0;
  printf(" AH(f)={");
  for(i=0;i<l_num;i++)
    for(j=l_num-1;j>=0;j--)
      if ((i!=(l_num-1-j))&&(AH1[i*l_num+j]=='1'))
	if(flag==0)
	  printf("l%d->l%d", i, (l_num-1-j));
	else
	  printf(", l%d->l%d", i, (l_num-1-j));
  printf("}");
  printf("\n");
}

/* This function converts a PC from a string rappresentation to an integer rappresentation. */
int pcstr_to_pcint(PCLIST *x, char* v){
  PCNODE *tmp;
  if(x->head==NULL){
    tmp=(PCNODE *) malloc(sizeof(PCNODE));
    tmp->old=(char *) malloc(strlen(v)+1);
    sprintf(tmp->old, "%s", v);
    tmp->new=countpc;
    tmp->next=NULL;
    x->head=tmp;
    return(countpc++);
  }else{  
    tmp=x->head;
    if(!strcmp(tmp->old,v))
      return tmp->new;
    while(tmp->next!=NULL){
      tmp=tmp->next;
      if(!strcmp(tmp->old,v)){
        return tmp->new;
      }
    }
    PCNODE *tmp2=(PCNODE *) malloc(sizeof(PCNODE));
    tmp2->old=(char *) malloc(strlen(v)+1);
    sprintf(tmp2->old, "%s", v);
    tmp2->new=countpc;
    tmp2->next=NULL;
    tmp->next=tmp2;
    return (countpc++); 
  }
}

/* This function converts a PC from an integer rappresentation to a string rappresentation. */
char* pcint_to_pcstr(PCLIST *x, int v){
  int i=0;
  PCNODE *tmp=x->head;
  while(i<v){
    tmp=tmp->next;
    i++;
  }
  return tmp->old;
}

/* This function does the PC reduction. */
int check_tuple(TUPLELIST *lst, int var, char* e1, char* f, char* e2){
  TUPLENODE *tmp;
  if(lst->head==NULL){
    tmp=(TUPLENODE *) malloc(sizeof(TUPLENODE));
    tmp->var=var;
    tmp->e1=(char *) malloc(strlen(e1)+1);
    sprintf(tmp->e1, "%s", e1);
    tmp->f=(char *) malloc(strlen(f)+1);
    sprintf(tmp->f, "%s", f);
    tmp->e2=(char *) malloc(strlen(e2)+1);
    sprintf(tmp->e2, "%s", e2);
    tmp->next=NULL;
    lst->head=tmp;
    return 0;
  }else{  
    tmp=lst->head;
    if((tmp->var==var)&&(!strcmp(tmp->e1,e1))&&
       (!strcmp(tmp->f,f))&&(!strcmp(tmp->e2,e2)))
      return 1;
    while(tmp->next!=NULL){
      tmp=tmp->next;
      if((tmp->var==var)&&(!strcmp(tmp->e1,e1))&&
	 (!strcmp(tmp->f,f))&&(!strcmp(tmp->e2,e2)))
	return 1;
    }
    
    TUPLENODE *tmp2=(TUPLENODE *) malloc(sizeof(TUPLENODE));
    tmp2->var=var;
    tmp2->e1=(char *) malloc(strlen(e1)+1);
    sprintf(tmp2->e1, "%s", e1);
    tmp2->f=(char *) malloc(strlen(f)+1);
    sprintf(tmp2->f, "%s", f);
    tmp2->e2=(char *) malloc(strlen(e2)+1);
    sprintf(tmp2->e2, "%s", e2);
    tmp2->next=NULL;
    tmp->next=tmp2;
    return 0;
  }
}

void init_Classmeth(){
  FILE* fp=fopen("HashClassMethod", "r");
  if(fp==NULL){
    printf("File HashClassMeth missing\n");
    exit(0);
  }
  
  char idstr[200];
  int idnum, flag=0;
  CLASSMETHNODE *tmp=cmlst.head;

  while(fscanf(fp, "%s %d", idstr, &idnum)==2){
    CLASSMETHNODE* tmp2=malloc(sizeof(CLASSMETHNODE));
    tmp2->id=malloc(strlen(idstr));
    strcpy(tmp2->id,idstr);
    tmp2->idnum=idnum;
    if(flag==0){
      cmlst.head=tmp2;
      tmp=cmlst.head;
      flag=1;
    }else{
      tmp->next=tmp2;
      tmp=tmp->next;
    }
  }
  fclose(fp);

}

char* read_Classmeth(int id){
  CLASSMETHNODE *tmp=cmlst.head;

  while(tmp!=NULL){
    if(tmp->idnum==id){
      return(tmp->id);
    }
    tmp=tmp->next;
  }
  return("NOTFOUND");
}
