#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "nest_conf.h"
#define MAX_TH 100
#define MAX_VAL_LEN 30

typedef struct NodeVC {
  int Gevent;
  int Levent;  
  int *VC;
  struct NodeVC *next;
  struct NodeVC *pred;
}listVC;

int downT1[MAX_TH];
int downT2[MAX_TH];
int lastPositions[MAX_TH];
int t1id, t2id, e1, e, e2, f;
int global_e=0, global_e1=0, global_e2=0, global_f=0;

int *lastOp;
int *lastTh;
