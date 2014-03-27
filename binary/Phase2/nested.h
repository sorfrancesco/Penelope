#ifndef _NESTED_H_
#define _NESTED_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "nest_conf.h"
/* In the matrix that we build there will be these fields:
   Read | Write | WW1 | WW2 | AA1 | AA2 | GWW | GAA  */ 
#define INF 8

typedef struct mainnode{
  int event;
  char* ah;
  int pc;
  struct mainnode *next;
}mainlist;

typedef struct pcnode{
  char* old;
  int new;
  struct pcnode *next;
}PCNODE;

typedef struct pclist{
  PCNODE *head;
}PCLIST;

typedef struct tuplenode{
  int var;
  char* e1;
  char* f;
  char* e2;
  struct tuplenode *next;
}TUPLENODE;

typedef struct tuplelist {
  TUPLENODE *head;
}TUPLELIST;

typedef struct relationnode{
  int father;
  int child;
  int event;
  struct relationnode *next;
}RELATIONNODE;

typedef struct relationlist {
  RELATIONNODE *head;
}RELATIONLIST;

typedef struct classmethnode{
  char* id;
  int idnum;
  struct classmethnode *next;
}CLASSMETHNODE;

typedef struct classmethlist {
  CLASSMETHNODE *head;
}CLASSMETHLIST;

#define init_Lockset(_m, _tnum, _lnum) do{	    \
    int _i, _j;			                    \
    for((_i)=0;(_i)<(_tnum);(_i)++)		    \
      for((_j)=0;(_j)<(_lnum);(_j)++)		    \
	(_m)[(_i)][(_j)]=0;			    \
  }while(0)

#define check_Lockset(_m, _tnum, _lnum, _n) do{				\
    int _i, _k, _j;							\
    for((_i)=0;(_i)<(_tnum);(_i)++)					\
      for((_k)=0;(_k)<(_tnum);(_k)++)					\
	for((_j)=0;(_j)<(_lnum);(_j)++)					\
	  if(((_i)!=(_k))&&(((_m)[(_i)][(_j)])==1)&&(((_m)[(_k)][(_j)])==1))  \
	    (_n)[(_i)][(_k)]=1;						\
	  else								\
	    (_n)[(_i)][(_k)]=0;						\
  }while(0)

#define add_ah(_ah, _L, _tind, _lind, _lnum) do{    \
    int _i;					    \
    for((_i)=0;(_i)<(_lnum);(_i)++)		    \
      if((_L)[(_tind)][(_i)]==1)		    \
	(_ah)[((_i)*(_lnum))+(_lnum-_lind-1)]='1';  \
    }while(0)

#define del_ah(_ah, _L, _tind, _lind, _lnum) do{    \
    int _i;					    \
    for((_i)=0;(_i)<(_lnum);(_i)++)		    \
      if((_L)[(_tind)][(_i)]==1)		    \
	(_ah)[((_lind)*(_lnum))+(_lnum-_i-1)]='0';  \
    }while(0)

#define set_Lockset(_L, _tind, _lind) do{           \
    (_L)[(_tind)][(_lind)]=1;			    \
  }while(0)

#define del_Lockset(_L, _tind, _lind) do{           \
    (_L)[(_tind)][(_lind)]=0;			    \
  }while(0)

#define init_array(_x, _dim) do{                    \
    int _i;					    \
    for((_i)=0;(_i)<(_dim);(_i)++)		    \
      (_x)[(_i)]=0;				    \
  }while(0)


#define init_matrix(_l) do{			    \
    int _t, _i, _r, _c;				    \
    mainlist _tmp;				            \
    (_tmp).event=0;				    \
    (_tmp).ah=NULL;				    \
    (_tmp).pc=0;				    \
    (_tmp).next=NULL;                               \
    for((_t)=0;(_t)<(t_num);(_t)++)		    \
      for((_i)=0;(_i)<INF;(_i)++)		    \
        for((_r)=0;(_r)<v_num;(_r)++)		    \
	  (_l)[(_t)][(_i)][(_r)]=(_tmp);	    \
}while(0)


void usage();
void init();
void init_AHmatrix(char* AH[]);
void insert_single_R_W(mainlist* A, int ev, char* ah, int pc);
void insert_AAWW(mainlist* A, mainlist* B, mainlist* C, int ev, char* ah, int pc);
void end_transactionWW(mainlist lock_mat[][INF][v_num], int pid, int x);
void end_transactionAA(mainlist lock_mat[][INF][v_num], int pid, int x);
void merge_list(mainlist *A, mainlist *B);
void AcqRelGWW(mainlist lock_mat[][INF][v_num], int pid, int x, int ev, char* ah, int pc);
void AcqRelGAA(mainlist lock_mat[][INF][v_num], int pid, int x, int ev, char* ah, int pc);
void free_list(mainlist* A);
int Check_list(mainlist* A, mainlist* B, int t1, int t2, int var, TUPLELIST *tplst, FILE* fp, PCLIST PClist);
void Check_matrix(mainlist lock_mat[][INF][v_num], FILE* fp, PCLIST PClist);
int find_AH(mainlist* A, char* ah, int pc);
void Print_List(mainlist* A, PCLIST* PClist);
void Print_All_Lists(mainlist lock_mat[][INF][v_num], int lnum, PCLIST* PClist);
void Print_Stats(mainlist lock_mat[][INF][v_num], int lnum);
int Count_List(mainlist* A, int c);
void printAHLS(char* AH, char* AH1);
int pcstr_to_pcint(PCLIST *x, char* v);
char* pcint_to_pcstr(PCLIST *x, int v);
int check_tuple(TUPLELIST *lst, int var, char* e1, char* f, char* e2);
void init_Classmeth();
char* read_Classmeth(int id);

#endif
