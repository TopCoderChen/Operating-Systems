/* COMP 530: Tar Heel SHell */

/*
Lab1 by Ruibin Ma & Qidi Chen.

UNC Honor Code: 

I certify that no unauthorized assistance has been received or
given in the completion of this work

Ruibin Ma ; Qidi Chen

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <termios.h>
#include <errno.h>
#include <signal.h>
// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024
#define DELIMITERS " \n\t\r\a"
#define NUM_TOK 64
#define MAX_TOK_LENGTH 256
#define NUM_CMD 64
#define MAX_PATH_LENGTH 1024
#define MAX_ENV_LENGTH 128

// define process
typedef struct process
{
    char part_cmd[MAX_INPUT];
    char **argv;
    char infile[MAX_PATH_LENGTH];
    char outfile[MAX_PATH_LENGTH];
    char errfile[MAX_PATH_LENGTH];
    pid_t pid; //process id
    char finished;
    char stopped;
    int status;
    struct process* next;
} process;

//define job
typedef struct job
{
    char cmd[MAX_INPUT]; //command line
    int jid; //job id
    pid_t pgid;
    struct termios terminalattr;
    process* head_process;// the first process in this job
    char notified;
    char builtin;
    struct job* next;
} job;


job *head_job = NULL;
pid_t sh_pgid;
struct termios sh_terminalattr;
int sh_terminal;
int interactive;
int debugornot=0;

// built-in cd
int th_cd(char **argv)
{
    char buffer[MAX_PATH_LENGTH + 1];
    char p[MAX_PATH_LENGTH];
    char temp_oldpwd[MAX_PATH_LENGTH];
    strcpy(temp_oldpwd, getenv("OLDPWD"));
    setenv("OLDPWD", getenv("PWD"), 1);
    if(debugornot) printf("OLDPWD = %s\n", getenv("OLDPWD"));
    if(argv[1] == NULL|| strcmp(argv[1], "~") == 0)
    {
        strcpy(p, getenv("HOME"));
        if(chdir(p)!=0)
        {
            fprintf(stderr, "cannot change to home directory\n");
        }
    }
    else if(strcmp(argv[1], "-") == 0)
    {
        strcpy(p, temp_oldpwd);
        if(chdir(p)!=0)
        {
            fprintf(stderr, "cannot change to the last working directory\n");
        }
    }
    else
    {
        strcpy(p, argv[1]);
        if(argv[1][0] == '~')
        {
            strcpy(p, getenv("HOME"));
            strcat(p, &argv[1][1]);
        }
        if(chdir(p)!=0)
        {
            fprintf(stderr, "cannot change to this directory\n");
        }
    }
    
    setenv("PWD", getcwd( buffer, MAX_PATH_LENGTH), 1);
    
    return 0;
}



int th_pwd(void)
{
    char *currentpath;
    currentpath = getenv("PWD");
    if(currentpath!=NULL)
    {
        printf("%s\n", currentpath);
    }
    return 0;
}
/*int launchjob(char **argv)
{
    int state;
    pid_t pid,wpid;
    pid = fork();
    if(pid == 0)
    {
        execvp(argv[0], argv);
        fprintf(stderr, "failed to execute %s\n", argv[0]);
        exit(1);
    }
    else
    {
        printf("this is parent's job\n");
        waitpid(0, &state, WUNTRACED);
        printf("parent process returned\n");
    }
    return 0;
}*/
void view_jobs(job*);

//function for redirection
void arrangeinout(int in, int out, int err)
{
    if(in!=0)
    {
        dup2(in, 0);
        close(in);
    }
    if(out!=1)
    {
        dup2(out, 1);
        close(out);
    }
    if(err!=2)
    {
        dup2(err, 2);
        close(err);
    }
}


void launch_process(process* p, job *j, int in, int out, int err, int foreground)
{
    pid_t pgid;
    pid_t pid;
    pgid = j->pgid;
    int status = 0;
    if(interactive)
    {
        pid = getpid();
        if(pgid == 0) pgid = pid;
        setpgid(pid, pgid);
        j->pgid = pgid;
        p->pid = pid;
        if(debugornot) printf("[%d]   PGID:%ld   PID:%ld         RUNNING:%s\n",j->jid, (long)j->pgid, (long)p->pid,  p->part_cmd);
        if(foreground)
            tcsetpgrp(sh_terminal, pgid);
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        //signal(SIGCHLD, SIG_DFL);
    }
    
    arrangeinout(in, out, err);
    execvp(p->argv[0], p->argv);
    fprintf(stderr, "failed to execvp: %s\n", p->argv[0]);
    status = 1;
    exit(status);
}


int job_stopped(job *j)
{
    process *p;
    p = j->head_process;
    while(p!=NULL)
    {
        if(p->finished==0 && p->stopped==0)
        {
            return 0;
        }
        p = p->next;
    }
    return 1;
}


int job_finished(job *j)
{
    process *p;
    p = j->head_process;
    while(p!=NULL)
    {
        if(p->finished==0)
        {
            return 0;
        }
        p = p->next;
    }
    return 1;
}


int mark_status(pid_t pid, int status)
{
    job *j;
    process *p;
    if(pid > 0)
    {
        for(j = head_job; j!=NULL; j = j->next)
            for(p = j->head_process; p!=NULL; p = p->next)
                if(p->pid == pid)
                {
                    p->status = status;
                    if(WIFSTOPPED(status))
                    {
                        p->stopped = 1;
                        if(debugornot) printf("[%d]   PGID:%ld   PID:%ld         STOPPED:%s\n", j->jid,(long)j->pgid, (long)p->pid, p->part_cmd);
                    }
                    else
                    {
                        p->finished = 1;
                        if(debugornot) printf("[%d]   PGID:%ld   PID:%ld         FINISHED:%s\n", j->jid,(long)j->pgid, (long)p->pid, p->part_cmd);
                        if(WIFSIGNALED(status))
                            if(debugornot) fprintf(stderr, "[%d] PGID:%ld           TERMINATED BY SIGNAL:%d\n", j->jid, (long)j->pgid, WTERMSIG(status));
                    }
                    //printf("mark pid:%d succeeded %d %d\n", (int)pid, p->stopped, p->finished);
                    return 0;
                }
        fprintf(stderr, "No process %d\n", (int)pid);
        return -1;
    }
    else if(pid == 0 || errno == ECHILD)
        return -1;
    else
        fprintf(stderr, "weird error\n");
    return -1;
}


void update_status()
{
    int status;
    pid_t pid;
    do
    {
        pid = waitpid(-1, &status, WUNTRACED|WNOHANG);
        //printf("update wait get pid:%d errno:%d\n", (int)pid, (int)errno);
    }while(!mark_status(pid, status));
}


void wait_job(job *j)
{
    pid_t pid;
    int status;
    char rv[8] = {'\0'};
    do
    {
        pid = waitpid(-1, &status, WUNTRACED);
        sprintf(rv, "%d", status);
        setenv("?", rv , 1);
        //printf("wait_job wait get pid:%d errno:%d\n", (int)pid, (int)errno);
    }while(!mark_status(pid, status) && job_finished(j)==0 && job_stopped(j)==0);
}


void fg_job(job *j, int cont)
{
    tcsetpgrp(sh_terminal, j->pgid);
    
    if(cont)
    {
        tcsetattr(sh_terminal, TCSADRAIN, &j->terminalattr);
        if(kill(-j->pgid, SIGCONT) < 0)
            fprintf(stderr, "kill (SIGCONT)\n");
    }
    wait_job(j);
    tcsetpgrp(sh_terminal, sh_pgid);
    tcgetattr(sh_terminal, &j->terminalattr);
    tcsetattr(sh_terminal, TCSADRAIN, &sh_terminalattr);
}


void bg_job(job *j, int cont)
{
    if(cont)
        if(kill(-j->pgid, SIGCONT)<0)
            fprintf(stderr, "kill (SIGCONT)");
}


void free_job(job *j)
{
    if(j==NULL)
    {
        fprintf(stderr, "cannot free NULL job");
        exit(1);
    }
    process *p, *q;
    p = j->head_process;
    while(p!=NULL)
    {
        q = p->next;
        free(p->argv);
        free(p);
        p = q;
    }
    free(j);
}


void print_job()
{
    job *j, *lastj, *nextj;
    process *p;
    update_status();
    lastj = NULL;
    j = head_job;
    while(j!=NULL)
    {
        nextj = j->next;
        if(!j->notified)
        {
            if(j->builtin == 1)
            {
                lastj = j;
                j->notified = 1;
            }
            else if(job_finished(j))
            {
                //printf("[%d]   PGID:%ld         FINISHED: %s\n", j->jid, (long)j->pgid, j->cmd);
                lastj = j;
                j->notified = 1;
            }
            else if(job_stopped(j))
            {
                printf("[%d]   PGID:%ld         STOPPED: %s\n", j->jid, (long)j->pgid, j->cmd);
                for(p = j->head_process; p; p = p->next)
                {
                    if(p->stopped == 1)
                        printf("[%d]    PID:%ld         STOPPED: %s\n",j->jid, (long)p->pid, p->part_cmd);
                }
                lastj = j;
            }
            else
            {
                printf("[%d]   PGID:%ld         RUNNING: %s\n", j->jid, (long)j->pgid, j->cmd);
                lastj = j;
                for(p = j->head_process; p; p = p->next)
                {
                    if(p->stopped == 0 && p->finished==0)
                        printf("[%d]    PID:%ld         RUNNING: %s\n", j->jid, (long)p->pid, p->part_cmd);
                }
            }
        }
        j = nextj;
    }
}


void continue_job(int jid, int foreground)
{
    job *j;
    process *p;
    j = head_job;
    while(j!=NULL)
    {
        if(j->jid == jid)
        {
            break;
        }
        j = j->next;
    }
    if(j->notified == 1)
    {
        fprintf(stderr, "this job is a built-in command or has finished\n");
        exit(1);
    }
    else if(job_stopped(j)!=1)
    {
        printf("[%d]         %s\n", j->jid, j->cmd);
        if(foreground)
            fg_job(j, 0);
        else
            bg_job(j, 0);
    }
    else
    {
        printf("[%d]         %s\n", j->jid, j->cmd);
        p = j->head_process;
        while(p!=NULL)
        {
            p->stopped = 0;
            p = p->next;
        }
        j->notified = 0;
        if(foreground)
            fg_job(j, 1);
        else
            bg_job(j, 1);
    }
}


int launch_job(job *j, int foreground)
{
    pid_t pid;
    process* p;
    int fd[2], in, out, err, state, status;
    char *position_equ;
    char env_name[MAX_ENV_LENGTH];
    char *env_value;
    char hold;
    status = 0;
    in = 0;
    int oldout, olderr, oldin;
    //oldout = dup(1);
    //olderr = dup(2);
    //oldin  = dup(0);
    if(debugornot) printf("RUNNING: %s\n", j->cmd);
    for(p = j->head_process; p!=NULL; p = p->next)
    {
        if(p!=j->head_process && strlen(p->infile)!=0)
        {
            fprintf(stderr, "ambiguous input redirection!\n");
            exit(1);
        }
        else if(strlen(p->infile)!=0)
        {
            in = open(p->infile, O_RDONLY);
            if(in < 0)
            {
                fprintf(stderr, "cannot open file %s\n", p->infile);
                exit(1);
            }
        }
        if(p->next!=NULL)
        {
            if(!(strlen(p->outfile)==0 && strlen(p->errfile)==0))
            {
                fprintf(stderr, "ambiguous output redirection!\n");
                exit(1);
            }
            pipe(fd);
            out = fd[1];
        }
        else if(strlen(p->outfile)>0)
        {
            out = open(p->outfile, O_WRONLY | O_CREAT | O_EXCL, S_IRWXU | S_IRWXO | S_IRWXG);
            if(out < 0)
            {
                fprintf(stderr, "cannot open file %s\n", p->outfile);
                exit(1);
            }
        }
        else out = 1;
        if(strlen(p->errfile)>0)
        {
            err = open(p->errfile, O_WRONLY | O_CREAT, S_IRWXU, S_IRWXO, S_IRWXG);
            if(err < 0)
            {
                fprintf(stderr, "cannot open file %s\n", p->errfile);
                exit(1);
            }
        }
        else err = 2;
        // ---------- built-in command
        //printf("p-argv = %s: %d%d%d\n", p->argv[0], in, out, err);
        if(strcmp(p->argv[0], "cd") == 0)
        {
            j->builtin = 1;
            if(status = th_cd(p->argv)==0);
            else
            {
                fprintf(stderr, "cannot change to this directory\n");
                status = 1;
            }
        }
        else if(strcmp(p->argv[0], "exit") == 0)
        {
            j->builtin = 1;
            exit(0);
            fprintf(stderr, "exit(0) failed\n");
            status = 1;
        }
        else if(strcmp(p->argv[0], "set") == 0)
        {
            j->builtin = 1;
            position_equ = strchr(p->argv[1], '=');
            if(position_equ == NULL)
            {
                fprintf(stderr, "wrong syntax using set\n");
                status = 1;
            }
            else
            {
                strncpy(env_name, p->argv[1], position_equ - p->argv[1]);
                env_value = position_equ + 1;
                setenv(env_name, env_value, 1);
                status = 0;
            }
        }
        else if(strcmp(p->argv[0], "jobs") == 0)
        {
            j->builtin = 1;
            arrangeinout(in, out, err);
            print_job(head_job);
            //dup2(oldin, 0);
            //dup2(oldout,1);
            //dup2(olderr,2);
            status = 0;
        }
        
        else if(strcmp(p->argv[0], "goheels")==0){
		  j->builtin = 1;
		  arrangeinout(in, out, err);
		  status = goheels();
        }
        
        else if(strcmp(p->argv[0], "fg") == 0)
        {
            j->builtin = 1;
            continue_job(atoi(p->argv[1]), 1);
        }
        else if(strcmp(p->argv[0], "bg") == 0)
        {
            j->builtin = 1;
            continue_job(atoi(p->argv[1]), 0);
        }
        // ---------- built-in command
        else{
            pid = fork();
            if(pid == 0)
            {
                launch_process(p, j, in, out, err, foreground);
            }
            else if(pid < 0)
            {
                fprintf(stderr, "fork failed\n");
                exit(1);
            }
            else
            {
                p->pid = pid;
                if(interactive)
                {
                    if(!j->pgid) j->pgid = pid;
                    setpgid(pid, j->pgid);
                }
            }
        }
        if(in != 0) close(in);
        if(out!= 1) close(out);
        
        in = fd[0];
    }
    //close(oldin);
    //close(oldout);
    //close(olderr);
    
    if(j->builtin!=1)
    {
        char rv[8]={'\0'};
        sprintf(rv, "%d", status);
        setenv("?", rv , 1);

        if(!interactive)
            wait_job(j);
        else if(foreground)
            fg_job(j, 0);
        else
            bg_job(j, 0);
    }
    if(debugornot) printf("ENDING:%s\n",j->cmd);
    return status;
}

int launch(job* j, int foreground)
{
    int status = 0;
    char *position_equ;
    char env_name[MAX_ENV_LENGTH];
    char *env_value;
    if(j->head_process->argv[0] == NULL)
    {
        fprintf(stderr, "empty command");
        status = 1;
    }
    
    else
    {
        status = launch_job(j, foreground);
    }
    return status;
}


//parse the input command
char **parsing(char * command_line)
{
    int i=0;
    char *tok;
    char **toks = malloc( NUM_TOK * sizeof(char*));
    if(!toks){ fprintf(stderr, "memory allocation of toks failed\n");exit(1);}
    
    tok = strtok(command_line, DELIMITERS);
    while(tok != NULL)
    {
        toks[i++] = tok;
        tok = strtok(NULL, DELIMITERS);
    }
    toks[i] = NULL;
    return toks;
}

//replace environment
void env_replace(char *cmd)
{
    char *cmd_position_1;
    char *cmd_position_2;
    char new_cmd[MAX_INPUT] = {'\0'};
    int env_position = 0;
    char env_name[MAX_ENV_LENGTH + 1] = {'\0'};
    char emptychar[MAX_ENV_LENGTH + 1] = {'\0'};
    char *env_value;
    char *cmd_start;
    cmd_start = cmd;
    cmd_position_1 = strchr(cmd, '$');
    
    
    while(cmd_position_1 != NULL)
    {
        cmd_position_2 = cmd_position_1 + 1;
        if(cmd_position_2[0] == '?')
        {
            env_name[0] = '?';
            env_name[1] = '\0';
            cmd_position_2++;
        }
        else
        {
            while((cmd_position_2[0] >= 'a'&& cmd_position_2[0] <= 'z' || cmd_position_2[0] >= 'A' && cmd_position_2[0] <= 'Z') && env_position<MAX_ENV_LENGTH)
            {
                env_name[env_position++] = cmd_position_2[0];
                cmd_position_2++;
            }
            env_name[env_position] = '\0';
        }
        printf("env_name: %s\n", env_name);
        strncat(new_cmd, cmd_start, cmd_position_1 - cmd_start);
        env_value = getenv(env_name);
        if(env_value!=NULL)
        {
            strcat(new_cmd, env_value);
        }
        
        env_position = 0;
        strcpy(env_name, emptychar);
        cmd_start = cmd_position_2;
        cmd_position_1 =strchr(cmd_start, '$');
    }
    strcat(new_cmd, cmd_start);
    strcpy(cmd, new_cmd);
    return;
}


void free_jobs(job* head_job)
{
    job* j;
    job* temp;
    process* p;
    process* q;
    j = head_job;
    while(j!=NULL)
    {
        temp = j->next;
        p = j->head_process;
        while(p!=NULL)
        {
            free(p->argv);
            q = p;
            p = p->next;
            free(q);
        }
        j->head_process = NULL;
        free(j);
        j = temp;
    }
}


void view_jobs(job* head_job)
{
    job* j;
    process* p;
    j = head_job;
    while(j!=NULL)
    {
        printf("%d-->%s\n", j->jid, j->cmd);
        p = j->head_process;
        while(p!=NULL)
        {
            printf("process: %s\n", p->argv[0]);
            p = p->next;
        }
        j = j->next;
    }
}



void redirection(process* p)
{
    char *position, *tp;
    char *r = "<>>";
    char *rn= "012";
    char file[3][MAX_PATH_LENGTH] = {'\0'};//in out err
    int  i=0, n;
    char temp_part_cmd[MAX_INPUT];
    char emptychar[MAX_INPUT] = {'\0'};
    strcpy(temp_part_cmd, p->part_cmd);
    for(n=0; n<3; n++)
    {
        char final_part_cmd[MAX_INPUT] = {'\0'};
        int fn;
        fn = n;
        position = strchr(temp_part_cmd, r[n]);
        if(position!=NULL)
        {
            tp = position + 1;
            if(position - temp_part_cmd != 0 && strchr(rn, (position-1)[0])!=NULL && (position - temp_part_cmd == 1 || (position-2)[0] == ' '))
            {
                char f;
                f = (position-1)[0];
                if(n==0 && f=='0')
                {
                    position = position - 1;
                    fn = 0;
                }
                else if(n==0 && f!='0')
                {
                    fprintf(stderr, "wrong syntax!\n");
                    exit(1);
                }
                else if((n==1||n==2) && f=='1')
                {
                    position = position - 1;
                    fn = 1;
                }
                else if((n==1||n==2) && f=='2')
                {
                    position = position - 1;
                    fn = 2;
                }
                else
                {
                    fprintf(stderr, "wrong syntax!\n");
                    exit(1);
                }
            }
            
            while(tp!=NULL && strchr(DELIMITERS, tp[0])!=NULL) tp++;
            while(tp!=NULL && strchr(DELIMITERS, tp[0])==NULL && tp[0]!='<' && tp[0]!='>')
            {
                file[fn][i++] = tp[0];
                tp++;
            }
            file[fn][i] = '\0';
            
            strncpy(final_part_cmd, temp_part_cmd, position - temp_part_cmd);
            
            strcat(final_part_cmd, tp);
            strcpy(temp_part_cmd, emptychar);
            strcpy(temp_part_cmd, final_part_cmd);
            strcpy(final_part_cmd, emptychar);
            i = 0;
        }
    }
    strcpy(p->infile, file[0]); //printf("infile :%s |%d\n", p->infile, (int)(strlen(p->infile )));
    strcpy(p->outfile,file[1]); //printf("outfile:%s |%d\n", p->outfile,(int)(strlen(p->outfile)));
    strcpy(p->errfile,file[2]); //printf("errfile:%s |%d\n", p->errfile,(int)(strlen(p->errfile)));
    
    strcpy(p->part_cmd, temp_part_cmd);
    
    
    return;
}



void pipeline(job* j, char *cmd)
{
    char part_cmd[MAX_INPUT] = {'\0'};
    char emptychar[MAX_INPUT] = {'\0'};
    char *position_pipe;
    char *cmd_start;
    int i;
    process *lastp;
    position_pipe = strchr(cmd, '|');
    cmd_start = cmd;
    lastp = j->head_process;
    while(position_pipe!=NULL)
    {
        process *p = malloc(sizeof(process));
        strncpy(part_cmd, cmd_start, position_pipe - cmd_start);
        cmd_start = position_pipe + 1;
        //printf("partcommand: %s\n", part_cmd);
        strcpy(p->part_cmd, part_cmd);
        redirection(p);
        //printf("after redirection: %s\n", p->part_cmd);
        strcpy(part_cmd, emptychar);
        p->argv = parsing(p->part_cmd);
        
        if(j->head_process == NULL)
        {
            p->next = NULL;
            j->head_process = p;
            lastp = p;
        }
        else
        {
            p->next = NULL;
            lastp->next = p;
            lastp = p;
        }
        position_pipe = strchr(cmd_start, '|');
    }
    process *p = malloc(sizeof(process));
    strcpy(part_cmd, cmd_start);
    //printf("partcommandlast: %s\n", part_cmd);
    strcpy(p->part_cmd, part_cmd);
    redirection(p);
    //printf("after redirection: %s\n", p->part_cmd);
    strcpy(part_cmd, emptychar);
    p->argv = parsing(p->part_cmd);
    
    if(j->head_process == NULL)
    {
        p->next = NULL;
        j->head_process = p;
        lastp = p;
    }
    else
    {
        p->next = NULL;
        lastp->next =p;
        lastp = p;
    }
}


void init_sh()
{
    sh_terminal = 0;//stdin
    interactive = isatty(sh_terminal);
    if(interactive)
    {
        if(debugornot) printf("interactive\n");
        while(tcgetpgrp(sh_terminal)!=(sh_pgid = getpgrp())) kill(-sh_pgid, SIGTTIN);
        signal(SIGINT,  SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        //signal(SIGCHLD, SIG_IGN);
        sh_pgid = getpid();
        setpgid(sh_pgid, sh_pgid);
        tcsetpgrp(sh_terminal, sh_pgid);
        tcgetattr(sh_terminal, &sh_terminalattr);
    }
}

//foreground & background
int fore_back(char *cmd)
{
    char new_cmd[MAX_INPUT] = {'\0'};
    char *position;
    position = strchr(cmd, '&');
    if(position==NULL)
    {
        return 1;
    }
    else
    {
        if(strchr(position + 1, '&')!=NULL)
        {
            fprintf(stderr, "Syntax error: Should be only one & in a job command!\n");
            exit(1);
        }
        strncpy(new_cmd, cmd, position - cmd);
        strcat(new_cmd, position+1);
        strcpy(cmd, new_cmd);
        return 0;
    }
}



int goheels(){
    
    printf("\n");
    printf("     @  @  @    \n");
    printf("  @  * * * *  @    \n");
    printf("   *         *  \n");
    printf("  *           * \n");
    printf(" *    T      *   \n");
    printf(" *    A     * \n");
    printf(" *    R    * \n");
    printf("  *       * \n");
    printf("  *   H   * \n");
    printf("  *   E   * \n");
    printf("  *   E   * \n");
    printf("  *   L   * \n");
    printf("  *      * \n");
    printf("   *     * \n");
    printf("    * * * \n");
    printf("\n");
    return 0;
}


int main (int argc, char ** argv, char **envp) {
    
    int finished = 0;
    int i;
    char *prompt = "thsh> ";
    char cmd[MAX_INPUT];
    char buffer[MAX_PATH_LENGTH+ 1];
    char *currentpath = getcwd(buffer, MAX_PATH_LENGTH);
    setenv("OLDPWD", currentpath, 0);
    setenv("PWD", currentpath, 1);
    setenv("?", "0\0", 1);
    
    init_sh();
    
    job* lastj;
    lastj = head_job;
    
    int foreground;
    if(argc==2 && strcmp(argv[1],"-d")==0) {
        debugornot=1;
    }
    else if(argc>1)
	{
        char nameOfScript[MAX_INPUT];
        if(strcmp(argv[1],"-d")==0)
		{
            debugornot=1;
            strcpy( nameOfScript, argv[2]);
        }
        else
		{
             strcpy( nameOfScript, argv[1]);
        }
        char cmdCopy[MAX_INPUT];
        FILE *stream;
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        
        stream = fopen(nameOfScript, "r");
        if (stream == NULL)
            exit(1);
        
        while ((read = getline(&line, &len, stream)) != -1)
		{   
	        if(strlen(line)==1&&line[0] == '\n')
            {
              update_status();
            }
            else
            {
              strcpy(cmdCopy,line);
              char *k;
              k = strchr(cmdCopy, '\n');
              k[0] = '\0';
              const char ch = '#';
              if(strchr(cmdCopy,ch)!=NULL)
			  {
                continue;
              }
              env_replace(cmdCopy);
              foreground = fore_back(cmd);
              job *j;
              j = malloc(sizeof(job));
              strcpy(j->cmd, cmdCopy);
              pipeline(j, j->cmd);
              if(head_job == NULL)
              {
                j->jid = 0;
                head_job = j;
                lastj = j;
              }
              else
              {
                lastj->next = j;
                j->jid = lastj->jid + 1;
                lastj = j;
              }
              if(j->head_process->argv[0]!=NULL)
              {
                launch(j,foreground);
              }
            }
		}
        fclose(stream);
        exit(0);
    }
    while (!finished) { //this is a job
        char *cursor;
        char last_char;
        int rv;
        int pv;
        int count;
        int foreground;
        char **argv;
        int i, var_position;
        job *j;
        
        // Print the prompt
        currentpath = getenv("PWD");
        rv = write(1, "[", 1);
        rv = rv && write(1, currentpath, strlen(currentpath));
        rv = rv && write(1, "] ", 2);
        rv = rv && write(1, prompt, strlen(prompt));
        if (!rv) { 
            finished = 1;
            break;
        }
        
        // read and parse the input
        for(rv = 1, count = 0, 
            cursor = cmd, last_char = 1;
            rv 
            && (++count < (MAX_INPUT-1))
            && (last_char != '\n');
            cursor++) { 
            rv = read(0, cursor, 1);
            last_char = *cursor;
        } 
        *cursor = '\0';
        if(last_char == '\n') *(cursor-1) = '\0';
        
        if (!rv) { 
            finished = 1;
            break;
        }
        
        // Execute the command, handling built-in commands separately  
        if(cmd[0]!='\0')
        {
            env_replace(cmd);
            foreground = fore_back(cmd);
            j = malloc(sizeof(job));
            strcpy(j->cmd, cmd);
            pipeline(j, j->cmd);
            if(head_job == NULL)
            {
                j->notified = 0;
                j->jid = 0;
                head_job = j;
                lastj = j;
            }
            else
            {
                j->notified = 0;
                lastj->next = j;
                j->jid = lastj->jid + 1;
                lastj = j;
            }
            j->next = NULL; 
            if(j->head_process->argv[0]!=NULL)
            {
                /*finished =*/ launch(j, foreground);
            }
        }
        else
        {
            update_status();
        }
    }
    free(head_job);
    return 0;
}

