#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "nest_conf.h"
#define MAX_TH 100

int lastPositions[MAX_TH];

#define init_Lockset(_m, _lnum) do{		    \
    int _i, _j;			                    \
    for((_i)=0;(_i)<(2);(_i)++)			    \
      for((_j)=0;(_j)<(_lnum);(_j)++)		    \
	(_m)[(_i)][(_j)]=0;			    \
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

typedef struct Node {
  int threadid;
  int localev;
  int totalev;
  char* action;
  char* ah;
  struct Node *next;
  struct Node *inc;
  struct Node *out;
}list;

typedef struct relationnode{
  int father;
  int child;
  int event;
  struct relationnode *next;
}RELATIONNODE;

typedef struct relationlist {
  RELATIONNODE *head;
}RELATIONLIST;


void print_list(list* lst);
void usage();
int LS_empty(char* AH);
void find_globals(char* filename);
void load_array(char* filename);
int load_file_and_lists(char* filename1, char* filename2, list* t1lst, list* t2lst);
int schedule_lists(char* filename, list* t1lst, list* t2lst);
void add_edges(list* t1lst, list* t2lst);
void SMT_utils(char* filename1, char* filename3, char* filename4);
void count_phases(char* filename);
int t1id, t2id, e1, e, e2, f;
int global_e=0, global_e1=0, global_e2=0, global_f=0;
int maxev, minev, t1phase2, t2phase2;
char *ahl1, *ahl2;
int last, counter;
