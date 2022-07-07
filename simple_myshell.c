#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#define MAX_CMD_ARG 15
#define BUFSIZ 256

const char *prompt = "myshell> ";
char* cmdvector[MAX_CMD_ARG];
char* cmdpipe[MAX_CMD_ARG];
char  cmdline[BUFSIZ];
char  cmdcpy[BUFSIZ];

void fatal(char *str);
int makelist(char *s, const char *delimiters, char** list, int MAX_LIST);
void cmd_cd(char *dirName);  // for cd
void wait_child(int sig);  //  for zombie process
void redirect(char** cmd, const int len); // redirect routine

int main(int argc, char**argv){
  int i=0;
  int tokenCount = 0;
  int pipeCount = 0;
  static struct sigaction act;
  pid_t pid;

  sigemptyset(&act.sa_mask);
  act.sa_handler = wait_child;
  sigaction(SIGCHLD, &act, NULL);  //  prevent zombie process
  signal(SIGINT,SIG_IGN);  //  ignore ^C
  signal(SIGQUIT, SIG_IGN); // ignore ^\

  while (1) {
  	fputs(prompt, stdout);
again:
	if(fgets(cmdline, BUFSIZ, stdin) == NULL){
		if(errno == EINTR)    // if interrupt error call fget again
			goto again;
		else
			fatal("shell fget()");
	}
	cmdline[strlen(cmdline) -1] = '\0';
	memset(cmdcpy, 0, sizeof(cmdcpy));
	memcpy(cmdcpy, cmdline, strlen(cmdline)); // cpy cmdline to cmdcpy

	if((tokenCount = makelist(cmdline, " \t", cmdvector, MAX_CMD_ARG)) < 1)
		continue; // handler No input
	pipeCount = makelist(cmdcpy, "|", cmdpipe, MAX_CMD_ARG); // checking the presence of pipe 

	if(!strcmp(cmdvector[tokenCount-1], "&")){ // background process
		cmdvector[tokenCount-1] = (char*)0; // remove '&'
		tokenCount--;
		pid = fork();
		if(pid == -1)
			fatal("shell fork()");
		if(pid == 0){
			pid = fork();
			if(pid == -1)
				fatal("second fork()");
			if(pid == 0){
				if(!strcmp(cmdvector[0], "cd")){
					cmd_cd(cmdvector[1]);
					return 0;
				}
				else if(!strcmp(cmdvector[0], "exit"))
					return 0;
				else{
					/*background pipe*/
					if(pipeCount > 1){ // pipe exist
						char* cmd[MAX_CMD_ARG];
						int cmdCount;
						int p[2];

						for(i=0; i<pipeCount-1; i++){
							if((cmdCount = makelist(cmdpipe[i], " \t", cmd, MAX_CMD_ARG)) < 1)
								fatal("pipe input errer"); // if cmdpipe[i] is Null
							if(pipe(p) == -1)
								fatal("pipe() errer");
							switch(fork()){
								case -1:
									fatal("pipe fork error");
								case 0:
									close(p[0]);
									dup2(p[1], 1);
									close(p[1]);
								
									redirect(cmd, cmdCount);
									execvp(cmd[0], cmd);
									fatal("pipe execvp errro");
								default:
									close(p[1]);
									dup2(p[0], 0);
									close(p[0]);
									break;
							}
						}
					
						if((cmdCount = makelist(cmdpipe[pipeCount-1], " \t", cmd, MAX_CMD_ARG)) < 2)
							fatal("Last pipe input error");
						cmd[cmdCount-1] = '\0'; //  remove '&' from cmd
						cmdCount--;
						redirect(cmd, cmdCount);
						execvp(cmd[0], cmd);
						fatal("Last pipe execvp error");
					}
					else{	// no pipe
						redirect(cmdvector, tokenCount); // if redirect cmd exist do redirect
						execvp(cmdvector[0], cmdvector);
						fatal("Background execvp()");
					}
				}
			}
			else
				return 0;
		}
	}
	else{ // foreground
		if(!strcmp(cmdvector[0], "cd"))  // if cmd is changdirectory 
			cmd_cd(cmdvector[1]);
		else if(!strcmp(cmdvector[0], "exit")) // for exit command
			return 0;
		else{	
			switch(pid=fork()){
			case 0:
				signal(SIGINT, SIG_DFL); // default handler
				signal(SIGQUIT, SIG_DFL); // default handler

				/*pipe*/
				if(pipeCount > 1){ // pipe exist
					char* cmd[MAX_CMD_ARG];
					int cmdCount;
					int p[2];

					for(i=0; i<pipeCount-1; i++){
						if((cmdCount = makelist(cmdpipe[i], " \t", cmd, MAX_CMD_ARG)) < 1)
							continue; // input error
						if(pipe(p) == -1)
							fatal("pipe() errer");
						switch(fork()){
							case -1:
								fatal("pipe fork error");
							case 0:
								close(p[0]);
								dup2(p[1], 1);
								close(p[1]);
								
								redirect(cmd, cmdCount);
								execvp(cmd[0], cmd);
								fatal("pipe execvp errro");
							default:
								close(p[1]);
								dup2(p[0], 0);
								close(p[0]);
								break;
						}
					}
					
					if((cmdCount = makelist(cmdpipe[pipeCount-1], " \t", cmd, MAX_CMD_ARG)) < 1)
						fatal("Last pipe input error");
					redirect(cmd, cmdCount);
					execvp(cmd[0], cmd);
					fatal("Last pipe execvp error");
				}
				else{ // no pipe
					redirect(cmdvector, tokenCount); // if redirect cmd exist do redirect
					execvp(cmdvector[0], cmdvector);
					fatal("shell execvp()");
				}
			case -1:
  				fatal("foreground main()");
			default:
				waitpid(pid, NULL, 0);
			}
		}
	}
  }
  return 0;
}

void fatal(char *str){
	perror(str);
	exit(1);
}
int makelist(char *s, const char *delimiters, char** list, int MAX_LIST){	
  int i = 0;
  int numtokens = 0;
  char *snew = NULL;

  if( (s==NULL) || (delimiters==NULL) ) return -1;

  snew = s + strspn(s, delimiters);	/* Skip delimiters */
  if( (list[numtokens]=strtok(snew, delimiters)) == NULL )
    return numtokens;
	
  numtokens = 1;
  
  while(1){
     if( (list[numtokens]=strtok(NULL, delimiters)) == NULL)
	break;
     if(numtokens == (MAX_LIST-1)) return -1;
     numtokens++;
  }
  return numtokens;
}
void cmd_cd(char* dirName){
	if(chdir(dirName) != 0)
		printf("Usage : cd <dirpath>\n");
}
void wait_child(int sig){
	waitpid(-1, NULL, WNOHANG);
}
void redirect(char** cmd, const int len){
	int i, fd;
	char* filename;

	for(i=len-1; i > 0; i--){
		if(*cmd[i] == '<'){
			cmd[i] = '\0';
			filename = cmd[i+1];
			fd = open(filename, O_RDONLY| O_CREAT, 0644);
			if(fd == -1)
				fatal("redirect '<'");
			dup2(fd, 0);
			close(fd);
		}
		else if(*cmd[i] == '>'){
			cmd[i] = '\0';
			filename = cmd[i+1];
			fd = open(filename, O_WRONLY| O_CREAT| O_TRUNC, 0644);
			if(fd == -1)
				fatal("redirect '>'");
			dup2(fd, 1);
			close(fd);
		}
	}
}
