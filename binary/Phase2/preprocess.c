#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int countx=0;
int county=0;

typedef struct node{
  char* old;
  int new;
  struct node *next;
}NODE;

typedef struct list {
  NODE *head;
} LIST;

// This function given a variable name returns an integer
int check(LIST *x, char* v, FILE* fp){
  NODE *tmp;
  if(x->head==NULL){
    tmp=(NODE *) malloc(sizeof(NODE));
    tmp->old=(char *) malloc(strlen(v)+1);
    memcpy(tmp->old, v, strlen(v));
    tmp->new=countx;
    tmp->next=NULL;
    x->head=tmp;
    fprintf(fp, "%d %s\n", tmp->new, tmp->old);
    return(countx++);
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
    NODE *tmp2=(NODE *) malloc(sizeof(NODE));
    tmp2->old=(char *) malloc(strlen(v)+1);
    memcpy(tmp2->old, v, strlen(v));
    tmp2->new=countx;
    tmp2->next=NULL;
    tmp->next=tmp2;
    fprintf(fp, "%d %s\n", tmp2->new, tmp2->old);
    return (countx++); 
  }
}

// This function given a lock name returns an integer
int checkl(LIST *x, char* v){
  NODE *tmp;
  if(x->head==NULL){
    tmp=(NODE *) malloc(sizeof(NODE));
    tmp->old=(char *) malloc(strlen(v)+1);
	memcpy(tmp->old, v, strlen(v));
	tmp->new=county;
    tmp->next=NULL;
    x->head=tmp;

    return(county++);
  }else{  
    tmp=x->head;
    if(!strcmp(tmp->old,v))
      return tmp->new;
    while(tmp->next!=NULL){
      tmp=tmp->next;
		if(!strcmp(tmp->old,v))
			return tmp->new;
    }
    NODE *tmp2=(NODE *) malloc(sizeof(NODE));
    tmp2->old=(char *) malloc(strlen(v)+1);
	memcpy(tmp2->old, v, strlen(v));
	tmp2->new=county;
    tmp2->next=NULL;
    tmp->next=tmp2;
    return (county++); 
  }
}

int main(int argc, char* argv[]){
  int tmax=0, lmax=0, pid, lk, maxl=0;
  FILE *fp, *fp1, *fp2;
  char name[100], name2[100];
  char buf[100], buf2[100], val[100];
  int buf3, buf4;
  LIST xx, yy;
  int x, y, newx, newy;

  xx.head=NULL;
  yy.head=NULL;
    
  fp = fopen(argv[1], "r");
  if(fp==NULL)
    perror("Error ");

  strcpy(name, argv[1]);
  strcat(name, "_mod");
  strcpy(name2, argv[1]);
  strcat(name2, "_hvar");
 
  fp1 = fopen(name, "w");
  if(fp1==NULL)
    perror("Error ");

  fp2 = fopen(name2, "w");
  if(fp2==NULL)
    perror("Error ");

  while((fscanf(fp, "%s", buf))>0){
    if(!strncmp(buf, "**", 2)){
      fscanf(fp, "%d %s", &pid, buf);  
      if(pid>tmax)tmax=pid;
	if(!strncmp(buf, "MRS", 3)){
		fscanf(fp, " %s %d %d %s\n", buf2, &buf3, &buf4, val);
		newx=check(&xx, buf2, fp2);
		if(!strncmp(val, "_", 1))
			printf("Warning: Value not present for the variable %s!\n", buf2);
		fprintf(fp1, "** %d MR %d %d:%d %s\n", pid, newx, buf4, buf3, val);
      }else if(!strncmp(buf, "MWS", 3)){
		fscanf(fp, " %s %d %d %s\n", buf2, &buf3, &buf4, val);
		newx=check(&xx, buf2, fp2);
		if(!strncmp(val, "_", 1))
			printf("Warning: Value not present for the variable %s!\n", buf2);
		fprintf(fp1, "** %d MW %d %d:%d %s\n", pid, newx, buf4, buf3, val); 	
      }else if(!strncmp(buf, "MR", 2)){
		fscanf(fp, " %s %d %d %s\n", buf2, &buf3, &buf4, val);
		newx=check(&xx, buf2, fp2);
		if(!strncmp(val, "_", 1))
			printf("Warning: Value not present for the variable %s!\n", buf2);
		fprintf(fp1, "** %d MR %d %d:%d %s\n", pid, newx, buf4, buf3, val); 	
      }else if(!strncmp(buf, "MW", 2)){
		fscanf(fp, " %s %d %d %s\n", buf2, &buf3, &buf4, val);
		newx=check(&xx, buf2, fp2);
		if(!strncmp(val, "_", 1))
			printf("Warning: Value not present for the variable %s!\n", buf2);
		fprintf(fp1, "** %d MW %d %d:%d %s\n", pid, newx, buf4, buf3, val); 	 	
      }else if(!strncmp(buf, "SA", 2)){
		fscanf(fp, " %s %d %d\n", buf2, &buf3, &buf4);
		newy=checkl(&yy, buf2);
		fprintf(fp1, "** %d SA %d %d:%d\n", pid, newy, buf4, buf3); 	
      }else if(!strncmp(buf, "SR", 2)){
		fscanf(fp, " %s %d %d\n",
		buf2, &buf3, &buf4);
		newy=checkl(&yy, buf2);
		fprintf(fp1, "** %d SR %d %d:%d\n", pid, newy, buf4, buf3); 
      }else if(!strncmp(buf, "TC", 2)){
		fscanf(fp, " %d\n", &buf3);
		fprintf(fp1, "** %d TC %d\n", pid, buf3); 		
      }else if(!strncmp(buf, "BA", 2)){
		fscanf(fp, " %d\n", &buf3);
		fprintf(fp1, "** %d BA %d\n", pid, buf3); 		
      }else{
		fgetc(fp);
		fprintf(fp1, "** %d %c%c\n", pid, buf[0],buf[1]);
      } 
    }
  }
  
  fclose(fp);
  fclose(fp1);
  fclose(fp2);
  fp=fopen("nest_conf.h", "w");
  fprintf(fp, "static int v_num=%d;\n", countx);
  fprintf(fp, "static int t_num=%d;\n", tmax+1); 
  fprintf(fp, "static int l_num=%d;\n", county); 
  fclose(fp);
  
  return 0;
}
