#include "nested.h"
#include "VectorClocks.h"
#include <unistd.h>

//NOTE the file nested.h can not contain l_num=0;

int verbose=0;
int pcexpansion=1;
int acqhistory=0;
int reduction=0;
VECTNODE *vcl;
VECTNODE *vcl1;

int main(int argc, char* argv[]){
  FILE *fp, *fp1, *fp2;

  int c;
  extern char *optarg;
  extern int optind, optopt;
  
  while ((c = getopt(argc, argv, ":vpar")) != -1) {
    switch(c) {
    case 'v':
      verbose=1;
      break;
    case 'p':
      pcexpansion=0;
      break;
    case 'r':
      reduction=1;
      break;
    case 'a':
      acqhistory=1;
      break;
    }
  }
  
  /* Check the input parameters */
  if(argc-optind!=1){
    usage();
    return;
  }

  /* bar_vec is a matrix of size t x t such that bar_vec[i][j]=e if Ti at the event# e 
     invokes join(Tj) */
  int bar_vec[t_num][t_num];
  vcl=(VECTNODE *) malloc (t_num*(sizeof(VECTNODE)));
  init_vcc(vcl, t_num);
  vcl1=(VECTNODE *) malloc (t_num*(sizeof(VECTNODE)));
  init_vcb(vcl1, bar_vec, t_num);
  /* Open the file containing the run. */
  fp = fopen(argv[optind], "r");
  if(fp==NULL){
    perror("Error ");
    return;
  }

  char tmp[30];
  sprintf(tmp, "%s.nrun", argv[optind]);
  fp1 = fopen(tmp, "w");
  if(fp1==NULL){
    perror("Error ");
    return;
  }

  sprintf(tmp, "%s.pattern", argv[optind]);
  fp2 = fopen(tmp, "w");
  if(fp2==NULL){
    perror("Error ");
    return;
  }

  int Lockset[t_num][l_num]; /* This array indicates which locks own a given thread. */
  int events[t_num]; /* This is used to save the events number. */
  int LSEmpty[t_num]; /* This is used to indicate if the LS is empty at this moment. */ 
  int blocksOpen[t_num]; /* This is used to check if a R/W event is inside a block. */
  char* AH[t_num]; /* This array contains the AHs for each thread. */
  mainlist lock_mat[t_num][INF][v_num]; 
  char buf[100], pcstr[100], val[100];
  int pid, x=0, a, b, lock, multiplelocksflag=0, cont=0;
  PCLIST PClist;

  PClist.head=NULL;
  
  int thflag=0;

  /* Initialization of the data structures and variables. */
  init();  
  init_Lockset(Lockset, t_num, l_num);
  init_array(events, t_num);
  init_array(LSEmpty, t_num);
  init_array(blocksOpen, t_num);
  init_AHmatrix(AH);
  init_matrix(lock_mat);
	
  while(1){
    if(fscanf(fp, "** %d %s", &pid, buf)<=1)
      break;     /* EOF reached */

    if((!strncmp(buf, "MR", 2)) || (!strncmp(buf, "MW", 2))){ //Read/Write events
		fscanf(fp, " %d %s %s\n", &x, pcstr, val);
		if(!strncmp(buf, "MR", 2)){
			if(thflag){
				//Update the value of the pid's event.
				events[pid]++;
				cont++;
				fprintf(fp1, "%d %d %d MR%d %s %d %s\n", pid, events[pid], cont, x, pcstr, LSEmpty[pid], val); 
				insert_single_R_W(&(lock_mat[pid][0][x]), events[pid], AH[pid], pcstr_to_pcint(&PClist, pcstr));
				if(blocksOpen[pid]>0)
					insert_AAWW(&(lock_mat[pid][4][x]), &(lock_mat[pid][5][x]), &(lock_mat[pid][7][x]), events[pid], AH[pid], pcstr_to_pcint(&PClist, pcstr));
			}else
				fprintf(fp1, "%d -1 %d MR%d %s -1 %s\n", pid, cont, x, pcstr, val); 
		}else if(!strncmp(buf, "MW", 2)){
			if(thflag){
				//Update the value of the pid's event.
				events[pid]++;
				cont++;
				fprintf(fp1, "%d %d %d MW%d %s %d %s\n", pid, events[pid], cont, x, pcstr, LSEmpty[pid], val);    
				insert_single_R_W(&(lock_mat[pid][1][x]), events[pid], AH[pid], pcstr_to_pcint(&PClist, pcstr));
				if(blocksOpen[pid]>0){	  
					//For the Write events we have to update the lists AA and WW.
					insert_AAWW(&(lock_mat[pid][2][x]), &(lock_mat[pid][3][x]), &(lock_mat[pid][6][x]), events[pid], AH[pid], pcstr_to_pcint(&PClist, pcstr));
					insert_AAWW(&(lock_mat[pid][4][x]), &(lock_mat[pid][5][x]), &(lock_mat[pid][7][x]), events[pid], AH[pid], pcstr_to_pcint(&PClist, pcstr));
				}
			}else
				fprintf(fp1, "%d -1 %d MW%d %s -1 %s\n", pid, cont, x, pcstr, val);
		}
    }else if((!strncmp(buf, "SA", 2))||(!strncmp(buf, "SR", 2))){ //Acquire/Release events 
		//Update the value of the pid's event.
		events[pid]++;
		cont++;
        fscanf(fp, " %d %s\n", &lock, pcstr);
      
        if(!strncmp(buf, "SA", 2)){
			//Here we check if exists a thread that own more than one lock at time.
			if(multiplelocksflag==0){
				for(a=0;a<t_num;a++)
					if(Lockset[pid][a]>0) multiplelocksflag++;
				if(multiplelocksflag<=1) multiplelocksflag=0;
				else printf("Multiple locks owned by a single Thread. \n");
			}

			if(Lockset[pid][lock]>0){//Multiple acquisition of the same lock case.
				Lockset[pid][lock]++;
				fprintf(fp1, "%d %d %d acql%d %s %d _\n", pid, events[pid], cont, lock, pcstr, LSEmpty[pid]);       
			}else{
				set_Lockset(Lockset, pid, lock);
				add_ah(AH[pid], Lockset, pid, lock, l_num);
				(AH[pid])[lock*l_num+(l_num-lock-1)]='1';
				LSEmpty[pid]=1;
	  
				fprintf(fp1, "%d %d %d acql%d %s %d _\n", pid, events[pid], cont, lock, pcstr, LSEmpty[pid]);        
				for(a=0;a<v_num;a++){
					AcqRelGWW(lock_mat, pid, a, events[pid], AH[pid], pcstr_to_pcint(&PClist, pcstr));
					AcqRelGAA(lock_mat, pid, a, events[pid], AH[pid], pcstr_to_pcint(&PClist, pcstr));
				}
			}
		}else{//SR case
			if(Lockset[pid][lock]>1){//Multiple acquisition of the same lock case.
				Lockset[pid][lock]--;
				fprintf(fp1, "%d %d %d rel%d %s %d _\n", pid, events[pid], cont, lock, pcstr, LSEmpty[pid]);    
			}else if (Lockset[pid][lock]==1){
				del_Lockset(Lockset, pid, lock);
				del_ah(AH[pid], Lockset, pid, lock, l_num);
				(AH[pid])[lock*l_num+(l_num-lock-1)]='0';
				LSEmpty[pid]=0;
				for(a=0;a<l_num;a++)
					if(Lockset[pid][a]>=1){
						LSEmpty[pid]=1;
						break;
					}
				fprintf(fp1, "%d %d %d rel%d %s %d _\n", pid, events[pid], cont, lock, pcstr, LSEmpty[pid]);

				for(a=0;a<v_num;a++){
					AcqRelGWW(lock_mat, pid, a, events[pid], AH[pid], pcstr_to_pcint(&PClist, pcstr));
					AcqRelGAA(lock_mat, pid, a, events[pid], AH[pid], pcstr_to_pcint(&PClist, pcstr));
				}
			}else{
				printf("Error in the input: Multiple release of locks - Ev:%6d T:%3d L:%3d\n", events[pid], pid, lock);
				exit(0);
			}
		}
//		if(pid==3 && events[pid]<50){
//			printf("%d %d %d\n", pid, events[pid], LSEmpty[pid]);
//		}
			
		
    }else if((!strncmp(buf, "BE", 2))){//Transaction end events 
		//Update the value of the pid's event.
		events[pid]++;
		cont++;
		fprintf(fp1, "%d %d %d other 0 %d _\n", pid, events[pid], cont, LSEmpty[pid]); 
		fgetc(fp); //To get the '\n'   
		if(blocksOpen[pid]>0){ 
			blocksOpen[pid]--;
			for(a=0;a<v_num;a++){
				end_transactionWW(lock_mat, pid, a);
				end_transactionAA(lock_mat, pid, a);
			}
		}else{
			printf("Problem with BE: Thread %d close a block not open!\n", pid);
			exit(0);
		}
		//reset_Lockset(Lockset, pid, l_num);
    }else if((!strncmp(buf, "BB", 2))){//Transaction begin events
		//Update the value of the pid's event.
		events[pid]++;
		cont++;
		fprintf(fp1, "%d %d %d other 0 %d _\n", pid, events[pid], cont, LSEmpty[pid]);
		blocksOpen[pid]++;
		fgetc(fp); //To get the '\n'
    }else if((!strncmp(buf, "TB", 2))){//Transaction begin events
		//Update the value of the pid's event.
		events[pid]++;
		cont++;
		fgetc(fp); //To get the '\n'
		fprintf(fp1, "%d %d %d other 0 %d _\n", pid, events[pid], cont, LSEmpty[pid]);        
    }else if((!strncmp(buf, "TC", 2))){//Thread creation events
		//The TC events are discarded
		//events[pid++];
		//cont++;
		thflag=1;
		fscanf(fp, " %d\n", &x);
		vc_newop(vcl, t_num, pid, events[pid], x);
    }else if((!strncmp(buf, "BA", 2))){//Barrier events
		//The BA events are discarded
		//events[pid]++;
		//cont++;
		fscanf(fp, " %d\n", &x);
		vc_newbr(vcl1, bar_vec, t_num, pid, events[pid], x);//C'e' un problema con le barriere
    }else if((!strncmp(buf, "TE", 2))){//Thread End events
		//Update the value of the pid's event.
		events[pid]++;
		cont++;
		vc_newend(vcl1, bar_vec, t_num, pid, events[pid]);
		fprintf(fp1, "%d %d %d other 0 %d _\n", pid, events[pid], cont, LSEmpty[pid]);        
		fgetc(fp);
    }else{
		//Update the value of the pid's event.
		events[pid]++;
		cont++;
		fgetc(fp); //To get the '\n'
		fprintf(fp1, "%d %d %d other 0 %d _\n", pid, events[pid], cont, LSEmpty[pid]);        
    }
  }
   
  //Check the perfect matching (all the locks are released)
  for(a=0;a<t_num;a++)
    for(b=0;b<l_num;b++)
      if(Lockset[a][b]!=0){
		printf("Error in the input: lock not released - T:%3d L:%3d\n", a, b);
		exit(0);
      }

  printf("Option Acquisition Histories Check: ");
  (acqhistory)?printf("enabled\n"):printf("disabled\n");
  printf("Option PC Expansion:                ");
  (pcexpansion)?printf("enabled\n"):printf("disabled\n");
  printf("Option PC Reduction:                ");
  (reduction)?printf("enabled\n"):printf("disabled\n");
  printf("Option Output Verbosity:            ");
  (verbose)?printf("enabled\n"):printf("disabled\n");

  printf("\n");
  if(verbose)
    Print_Stats(lock_mat, l_num);
	
  //print_vc(vcl1, t_num);
  
  //Print_List(&lock_mat[3][0][257], &PClist);
    //Print_List(&lock_mat[3][1][257], &PClist);
	  //Print_List(&lock_mat[2][6][257], &PClist);
  //Print_List(&lock_mat[2][7][257], &PClist);
  //Print_All_Lists(lock_mat, l_num, &PClist);

  Check_matrix(lock_mat, fp2, PClist);

  fclose(fp);
  fclose(fp1);
  fclose(fp2);

  return 0;
}

