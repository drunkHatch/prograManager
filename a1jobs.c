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

typedef struct{
	int running_status; // 0 -> terminated, 1 -> running, -1 ->empty
	int jobNo;
	int pid;
	char command[256];
} Program;

/**********************************************************************/
/*****************         Global Variable         ********************/
/**********************************************************************/

int child_finished = 0;
int pid_list[32]; // pid list
int input_pnum; //user input process number
int process=0; //number of process running (maximun 32)
int parent; //store parent process id
Program executed_programs[MAXJOBS]; // array to record executed program


/**********************************************************************/
/*****************      Function Declaration       ********************/
/**********************************************************************/

void print_non_teminated_jobs();
void init_jobs_array();
int argument_number_finder(char *raw_string);
void exec_other_program(int option_number, char splited_raw_command[][32]);
void run(char splited_raw_command[][32], int option_number, char *raw_command);
void enroll_new_job(int pid, char command[256]);

void quit (int code){
}

void stop (int code){
}

void cont (int code){
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

// struct to store program info


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

	// init array variables
	init_jobs_array();

	//main loop-------------------
	//count number of process

	while(getpid()==parent){

		char raw_command[MAXLINE], copy_raw_command[MAXLINE];
		char splited_command[MAXLINE][MAXWORD];

		// 1, enter command
		printf("a1jobs[%d]: ",parent);
		scanf(" %132[^\n]",raw_command);

		// 2, analyze command -> fixed size & nonfixed
		strcpy(copy_raw_command, raw_command);
		split(raw_command,splited_command," []{}()=");
		char* cmd = splited_command[0];

		if(strcmp(cmd, "list")==0 ){
			printf("Enter list scope\n");
			print_non_teminated_jobs();
		}
		// TODO add MAXJOBS chneck later
		else if (strcmp(cmd, "run")==0 ){
			int argument_count = 0;
			printf("Enter run scope\n");
			// a function to check argument length
			argument_count = argument_number_finder(copy_raw_command) - 1;
			// deliver length to run function
			//run(&raw_command[3],pid_list);
			run(splited_command, argument_count, copy_raw_command);
		}
		else if (strcmp(cmd, "terminate")==0 ){
			printf("Enter terminate scope\n");\
			sscanf(splited_command[1],"%d",&input_pnum);
			printf("terminate input%d",input_pnum);
		}
		else if (strcmp(cmd, "quit")==0 ){
			printf("Enter quit scope\n");

		}
		else if (strcmp(cmd, "suspend")==0 ){
			printf("Enter suspend scope\n");

		}
		else if (strcmp(cmd, "resume")==0 ){
			printf("Enter resume scope\n");
		}
		else if (strcmp(cmd, "exit")==0 ){
			printf("Enter exit scope\n");
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

void run(char splited_raw_command[][32], int option_number, char *raw_command){
	int new_child_pid;

	new_child_pid = fork();
	// depend on option number, run another function to call execlp
	if (!new_child_pid) {
		printf("This is child: other program runs\n");
		exec_other_program(option_number, splited_raw_command);
	}
	sleep(1);
	child_finished = 0;
	// after calling execlp, enroll new jobs in job array (a function)
	enroll_new_job(new_child_pid, raw_command);

}


/**********************************************************************/
/*****************         Utility Function        ********************/
/**********************************************************************/
//int running_status; // 0 -> terminated, 1 -> running, -1 ->empty
//int jobNo;
//int pid;
//char command[256];

void enroll_new_job(int pid, char command[256]){
	for (int i = 0; i < MAXJOBS; i++) {
		// find empty slot for new job
		if (executed_programs[i].running_status == -1) {
		 	executed_programs[i].pid = pid;
			executed_programs[i].running_status = 1;
			strcpy(executed_programs[i].command, command);
			break;
		}
	}
}

void exec_other_program(int option_number, char splited_raw_command[][32]){
	switch (option_number) {
		case 0:
			execlp(splited_raw_command[1], splited_raw_command[1], (char *) NULL);
		case 1:
			execlp(splited_raw_command[1], splited_raw_command[1], splited_raw_command[2], (char *) NULL);
		case 2:
			execlp(splited_raw_command[1], splited_raw_command[1], splited_raw_command[2], splited_raw_command[3],(char *) NULL);
		case 3:
			execlp(splited_raw_command[1], splited_raw_command[1], splited_raw_command[2], splited_raw_command[3],splited_raw_command[4],(char *) NULL);
		case 4:
			execlp(splited_raw_command[1], splited_raw_command[1], splited_raw_command[2], splited_raw_command[3],splited_raw_command[4],splited_raw_command[5],(char *) NULL);
	}

}

int argument_number_finder(char *raw_string){
	int count = 0;

	for (int i = 0; raw_string[i]; i++) {
		if (raw_string[i] == ' ') {
			count++;
		}
	}
	return count;
}

// init variables of global job array
void init_jobs_array(){
	for (int i = 0; i < MAXJOBS; i++) {
		executed_programs[i].jobNo = i;
	 	executed_programs[i].pid = -1;
		executed_programs[i].running_status = -1;
	}
}

void print_non_teminated_jobs(){
	for (int i = 0; i < MAXJOBS; i++) {
		// if program is not terminated then print info
		if (executed_programs[i].running_status == 1) {
			printf("JobNo: %d\n\tpid: %d\n\tcmd: '%s'\n", executed_programs[i].jobNo, executed_programs[i].pid, executed_programs[i].command);
		}
	}
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
