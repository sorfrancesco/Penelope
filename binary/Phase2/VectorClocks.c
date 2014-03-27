#include "nested.h"
#include "VectorClocks.h"

void print_vc(VECTNODE vcl[], int size){
  int i, j;
  VECTNODE *tmp;
  
  for(i=0;i<size;i++){
    tmp=&vcl[i];
    while(tmp!=NULL){
      if(tmp->vc!=NULL){
	printf("Th=%d Ev=%d\n", i, tmp->localev);
	for(j=0;j<size;j++)
	  printf("%d ", tmp->vc[j]);
	printf("\n");
      }
      tmp=tmp->next;
    }
    printf("\n");
  }
}

/* This function initialites the VC structure for threads creation. */
void init_vcc(VECTNODE vcl[], int size){
  int i;

  for(i=0;i<size;i++){
    vcl[i].localev=1;
    vcl[i].vc=NULL;
    vcl[i].next=NULL;
  }
  vcl[0].vc=(int *) malloc (size*sizeof(int));
  for(i=0;i<size;i++)
    vcl[0].vc[i]=0;
  vcl[0].vc[0]=1;
}

/* This function initialites the VC for thread barriers and the bar_vec. */
void init_vcb(VECTNODE vcl[], int bar_vec[][t_num], int size){
  int i, j;

  for(i=0;i<size;i++){
    vcl[i].localev=1;
    vcl[i].vc=NULL;
    vcl[i].next=NULL;
  }

  for(j=0;j<size;j++){
    vcl[j].vc=(int *) malloc (size*sizeof(int));
    for(i=0;i<size;i++){
      vcl[j].vc[i]=0;
      bar_vec[i][j]=0;
    }
    vcl[j].vc[j]=1;
  }
}

/* This function records the event in which T1 at event# ev creates T2. */
void vc_newop(VECTNODE vcl[], int size, int t1, int ev, int t2){
  VECTNODE* vectmp, *vtmp;
   int i;

  //Get the last vector clock for t1
  vtmp=&vcl[t1];
  while(vtmp->next!=NULL)
    vtmp=vtmp->next;

  vectmp=(VECTNODE *) malloc (sizeof(VECTNODE));
  vectmp->localev=ev;
  vectmp->vc=(int *) malloc (size * sizeof(int));
  for(i=0;i<size;i++)
    vectmp->vc[i]=vtmp->vc[i];
  vectmp->vc[t1]+=1; 
  vectmp->next=NULL;
  vtmp->next=vectmp;
  

  vcl[t2].vc=(int *) malloc (size*sizeof(int));
  memcpy(vcl[t2].vc, vectmp->vc, (size * sizeof(int)));
  vcl[t2].vc[t2]=1;
  vcl[t2].localev=1;
  vcl[t2].next=NULL;
  //vcl[t2].vc[t1]++;
}

/* This function records the event in which T1 at event# ev creates T2. 
   Only join() is handled and the join on a thread can be invoked only by another unique 
   thread. */
void vc_newbr(VECTNODE vcl[], int bar_vec[][t_num], int size, int t1, int ev, int t2){
  VECTNODE  *vtmp=&vcl[t1];
  int i, j;

  bar_vec[t1][t2]=ev;

  for(i=0;i<size;i++)
		vcl[t2].vc[i]=0;
  vcl[t2].vc[t2]=1;
}

//Update the vector clocks for the end events
void vc_newend(VECTNODE vcl[], int bar_vec[][t_num], int size, int t, int ev){
  VECTNODE *vectmp, *vectmp1, *vtmp, *vtmp1;
  int i, j;
  
  //Get the last vector clock for t
  vtmp=&vcl[t];
  while(vtmp->next!=NULL)
    vtmp=vtmp->next;	

  //Insert the new vector clock for t
  vectmp=(VECTNODE *) malloc (sizeof(VECTNODE));
  vectmp->localev=ev;
  vectmp->vc=(int *) malloc (size * sizeof(int));
  vectmp->next=NULL;
  for(i=0;i<size;i++)
    vectmp->vc[i]=vtmp->vc[i];
  vtmp->next=vectmp;

  //Update the vector clock for the thread that invoked the join
  for(i=0;i<size;i++){
	
    //Thread i invoked a join on Thread t
    if(bar_vec[i][t]!=0){
      vtmp1=&vcl[i];
      while(vtmp1->next!=NULL){ 
		vtmp1=vtmp1->next;
	  }

      vectmp1=(VECTNODE *) malloc (sizeof(VECTNODE));
      vectmp1->localev=bar_vec[i][t];
      vectmp1->vc=(int *) malloc (size*sizeof(int));
	  vectmp1->next=NULL;
	  
	  
      for(j=0;j<size;j++){
		if(vectmp->vc[j]>vtmp1->vc[j])
			vectmp1->vc[j]=vectmp->vc[j];
		else
			vectmp1->vc[j]=vtmp1->vc[j];
      }
      vectmp1->vc[i]++;

      vtmp1->next=vectmp1;
    }
  }	
}
/*
void vc_newbr(VECTNODE vcl[], int bar_vec[][t_num], int size, int t1, int ev, int t2){
  VECTNODE  *vtmp=&vcl[t1];
  int i, j;

  bar_vec[t1][t2]=ev;

  //Get the last vector clock for t1
  while(vtmp->next!=NULL)
  	vtmp=vtmp->next;

  //Update the vector clock for t2
  for(i=0;i<size;i++)
		vcl[t2].vc[i]=vtmp->vc[i];
}

//Update the vector clocks for the end events
void vc_newend(VECTNODE vcl[], int bar_vec[][t_num], int size, int t, int ev){
  VECTNODE *vectmp, *vectmp1, *vtmp, *vtmp1;
  int i, j;
  
  //Get the last vector clock for t
  vtmp=&vcl[t];
  while(vtmp->next!=NULL)
    vtmp=vtmp->next;	
  

  //Insert the new vector clock for t
  vectmp=(VECTNODE *) malloc (sizeof(VECTNODE));
  vectmp->localev=ev;
  vectmp->vc=(int *) malloc (size * sizeof(int));
  vectmp->next=NULL;
  for(i=0;i<size;i++)
    vectmp->vc[i]=vtmp->vc[i];
  vtmp->next=vectmp;


  //Update the vector clock for the thread that invoked the join
  for(i=0;i<size;i++){
	
    //Thread i invoked a join on Thread t
    if(bar_vec[i][t]!=0){
      vtmp1=&vcl[i];
      while(vtmp1->next!=NULL){ 
		vtmp1=vtmp1->next;
	  }

      vectmp1=(VECTNODE *) malloc (sizeof(VECTNODE));
      vectmp1->localev=bar_vec[i][t];
      vectmp1->vc=(int *) malloc (size*sizeof(int));
	  vectmp1->next=NULL;
	  
      for(j=0;j<size;j++){
		if(vectmp->vc[j]>vtmp1->vc[j])
			vectmp1->vc[j]=vectmp->vc[j];
		else
			vectmp1->vc[j]=vtmp1->vc[j];
      }
      vectmp1->vc[i]++;

      vtmp1->next=vectmp1;
    }
  }	
}
*/

void get_vc(VECTNODE vcl[], int Tidx, int localev, int* tmp1, int size){
  VECTNODE *tmp=&vcl[Tidx];
  int i;
  //print_vc(vcl, t_num);

  while(1){
	for(i=0;i<size;i++)
	   tmp1[i]=tmp->vc[i];

    if(tmp->localev==localev)
      return;

    if(tmp->next!=NULL){
      if(localev>=tmp->next->localev)
		tmp=tmp->next;
      else{
	    memcpy(tmp1, tmp->next->vc, sizeof(int)*size);
		return ;
		}
    }else{
      tmp1[Tidx]++;
      return;
    }
  }
}


/* This function returns 1 if the event e in T1 has to occur before the event f in T2 */
int e_before_f(VECTNODE vcl[], int size, int T1, int e, int T2, int f){
  int i, count=0;

  int *evec=(int *)malloc(sizeof(int) * t_num);
  get_vc(vcl, T1, e, evec, size);
  int *fvec=(int *)malloc(sizeof(int) * t_num);
  get_vc(vcl, T2, f, fvec, size);
  for(i=0;i<size;i++){
    if(evec[i]>fvec[i]){
	  free(evec);
	  free(fvec);
      return 0;
	}
    else if(evec[i]==fvec[i])
      count++;
  }
  free(evec);
  free(fvec);

  if(count==size)
    return 0;
  return 1;
}
/*

int main(int argc, char* argv[]){
  int bar_vec[t_num][t_num];
  VECTNODE vcl[t_num];

  init_vcb(vcl, bar_vec, t_num);
  
  vc_newbr(vcl, bar_vec, t_num, 0, 10, 1);
  vc_newbr(vcl, bar_vec, t_num, 1, 100, 2);
  vc_newend(vcl, bar_vec, t_num, 2, 200);
  vc_newend(vcl, bar_vec, t_num, 1, 400);
  vc_newbr(vcl, bar_vec, t_num, 0, 500, 3);
  vc_newend(vcl, bar_vec, t_num, 3, 600);

  
  //printf("%d \n", e_before_f(vcl, t_num, 0, 51, 2, 1));
  print_vc(vcl,t_num);

  }*/


