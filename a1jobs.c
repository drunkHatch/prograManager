#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/times.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/resource.h>
//the time limit is 600 second
#define TIMELIM 1

//for split function  split(str,return_s," []{}()=")
#define MAXLINE     132
#define MAX_NTOKEN  MAXLINE
#define MAXWORD     32
int split(char inStr[],  char token[][MAXWORD], char fs[]);

#define MAXJOBS  32

// pid list
int pid_list[32];

//user input process number
int input_pnum;

//number of process running (maximun 32)
int process=0;

//store parent process id
int parent;


void quit (int code){

	return;
}

void stop (int code){
	printf("aaaaaaaaastop");
	return;
}

void cont (int code){
	printf("aaaaaaaaacont");
	return;
}

static void terminate(int signo){
	char cmd[MAXLINE];
	char pid_s[MAXWORD];

    printf("%d                 \n", signo);
    /*
	strcpy(cmd,"kill -s 9 \0");
    snprintf( pid_s, 20, "%d", pid_list[input_pnum] );
	strcat(cmd,pid_s);
	system(cmd);
	printf("!!%s!!%d  -- %d\n", cmd,input_pnum,pid_list[input_pnum]);
    */
	printf("exit\n");
	_exit(0);
}



int run(char *cmd,int* plist){
	int child, pid;

	//check if there are too many jobs
	if (process>=MAXJOBS){fprintf(stderr,"error: too many jobs are running (maximun 32)\n");}

	//fork a child
	child = fork();
    printf("this is the pid of child:%d\n",child);

	plist[process] = child;
	if (child == 0){

        signal(SIGUSR1,terminate);

		pid = getpid();
		plist[process] = pid;
		//execlp(cmd,"\0");
		system(cmd);
        

		printf("!   child      :%d\n",pid);

	}
    /*
    if ((pid = fork()) == 0) {
        signal(SIGINT, sigint_handler);
        printf("This is children %d\n", getpid());
        sleep(1);
        exit(0);
    }
    */

	process += 1;
	return pid;
}

int main(int argc, char* argv[]){

	// get the starting time of the program
	clock_t st_time;
	clock_t en_time;
	struct tms st_cpu;
	struct tms en_cpu;
	st_time = times(&st_cpu);

	//get parent pid
	parent = getpid();

	//set time limit
	struct rlimit rl;
	getrlimit (RLIMIT_CPU, &rl);
	rl.rlim_cur = TIMELIM;
	rl.rlim_max = TIMELIM;
	if (setrlimit(RLIMIT_CPU, &rl) == -1) fprintf(stderr,"set limit error\n");


	//set signal
	signal(SIGSTOP,stop);
	signal(SIGCONT,cont);
	signal(SIGQUIT,quit);
    signal(SIGUSR1,terminate);




	//main loop-------------------
	//count number of process

	while(getpid()==parent){

		char oricmd[MAXLINE];
		char sp_cmd[MAXLINE][MAXWORD];
		// char strexit[8];

		// strcpy(strexit,"exit");

		printf("a1jobs[%d]: ",parent);
		scanf(" %132[^\n]",oricmd);
		split(oricmd,sp_cmd," []{}()=");
		char* cmd = sp_cmd[0];

		if(strcmp(cmd, "list")==0 ){
			printf("list input");
			//system("xclock -geometry 200x200 -update 2");
		}else if (strcmp(cmd, "run")==0 ){
			// system("xclock -geometry 200x200 -update 2");
			//printf("run input");
			run(&oricmd[3],pid_list);
		}else if (strcmp(cmd, "terminate")==0 ){
			sscanf(sp_cmd[1],"%d",&input_pnum);
			printf("terminate input%d",input_pnum);

		}else if (strcmp(cmd, "quit")==0 ){

			break;
		}

	}
	//main loop ends--------------

	// clock_t t1 = clock();

	en_time = times(&en_cpu);

	//get clock per second
	static long clktck;
	if ((clktck = sysconf(_SC_CLK_TCK)) < 0) fprintf(stderr,"sysconf error");


	//print cpu runtime

	return 0;



}


int split(char inStr[],  char token[][MAXWORD], char fs[])
{
    int    i, count;
    char   *tokenp, inStrCopy[MAXLINE];

    count= 0;
    memset (inStrCopy, 0, sizeof(inStrCopy));
    for (i=0; i < MAX_NTOKEN; i++) memset (token, 0, sizeof(token[i]));

    strcpy (inStrCopy, inStr);
    if ( (tokenp= strtok(inStr, fs)) == NULL) return(0);

    strcpy(token[count],tokenp); count++;

    while ( (tokenp= strtok(NULL, fs)) != NULL) {
        strcpy(token[count],tokenp); count++;
    }
    strcpy (inStr, inStrCopy);
    return(count);
}
