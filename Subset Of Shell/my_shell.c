/*
MY SHELL PROGRAME...

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

extern int errno;


#define MAX_PROFILE_CHR 256   /* maximum lines on prompt*/
#define MAX_PROFILE_LINES 20  /*maximum entries into profile file*/


/*normal command holders...*/
typedef void (*sighandler_t)(int);
static char *string_tokens[100];
static char *path_tokens[10];

/*alias command holders...*/
void free_alias_argv();
static char *alias_tokens[100];
static int alp;

/*environment variable holders...*/
static char *env_tokens[100];
static int ind;

/*environment variable holders...*/
static char *ifelse_tokens[100];


/*directory holders...*/
char home_dir_path[MAX_PROFILE_CHR];
char path_dir_path[256];
int home_flag, path_flag;

/*common functions...*/
void create_prompt();
void welcome_msg();
void profile_file(char *);
void find_path(char * , char *, int );
void handle_signal();
int check_command(char *);
void free_argv();
void process_execute(char *);
void fill_argv(char *);
void shell_cmd_exec(char *);


/*built-in command functions...*/
void exec_cd(char *dirs);
void exec_echo(char *);
void exec_exit();

/*alias command functions...*/
int search_alias(char *);
int check_alias(char *);
char *find_alias(char *);

/*if-else construct functions...*/
int env_check( char *);
int search_env_var(char *);
int ifelse_check(char *);
void parse(char *);
void ifelse_process(char *);
int find_values(char *, char *, int);
void print_stmnts(char *);
void process_blocks();


/*global declarations...*/
char *shell_cmd[4]={"cd","exit","echo","alias"};
char *alias_filename= "alias_store.txt";
static int ifelse_cnt;

/*The Shell Starts here.....*/

int main(int argc, char *argv[])
{

    /*variable declaration...*/
    char c, *env_token;
	char *profile_filename = "my_shell_profile";        
    int p=0, fd, a_ret, i=0;    
    
    char *comand = (char *)malloc(sizeof(char ) * 100);
    char *cmd= (char *)malloc(sizeof(char) * 100);
    
    //checking memory allocation.....
    if (comand == NULL  || cmd==NULL)
        printf("\n unable to allocate memory...to comand or string-token");
    
    welcome_msg();
    
     //to extract strings from .profile_file.
    profile_file(profile_filename);  
    
	//Signal handlers...	
  	signal(SIGINT, SIG_IGN);
	signal(SIGINT, handle_signal);

    //to crate present directory prompt...
	create_prompt();
	
	fflush(stdout);
	
	//accept character from STDIN...
	while(c != EOF)   
	{
	c = getchar();
	switch(c)
	{
	    
	    case'\n':if(comand[0] == '\0' || comand[0]== '\n')       //check whether \n pressed...
	             {
	                create_prompt();
	             }			  			     			     
                 else if(env_check(comand) == 1 ) //if...else...block...starts...
                 { 
                    /*check whether environment variable entered of command*/
                    create_prompt();         
                 }
                 else if(check_alias(comand)==3)   //check for alias existance...
                 {
                      create_prompt();
                 }  
                 else
                 {                                
                    /*Shell supports basic IF..ELSE statements...*/      
                    int ret_if;
                    ret_if=ifelse_check(comand);
                    if(ret_if == 1 || ret_if==2 ) 
                    {   
                        //IF--ELSE block found...now execute on it...                                     
                        ifelse_process(comand);                                                                    
                    }
                    else
                    {                                                                        
                       comand=find_alias(comand);    // Check for Aliases
  				       
  				       if(strstr(comand,"echo"))
  				       {
  				            exec_echo(comand);
  	                        create_prompt();
  				            break;
  				       }
  				       
  				       //normal shell command execution...
  				       fill_argv(comand);            
  				       /* it stores command into global array ...*/
					   strncpy(cmd, string_tokens[0], strlen(string_tokens[0]));
					   strncat(cmd, "\0", 1);
					   
					   if(index(cmd, '/') == NULL)  //check for directory...
					   {
					       int ret_val;					
					        //partial check for command...for in-built & normal commands...       
					       ret_val=check_command(cmd); 
					       if(ret_val == 0)
						   {
						       process_execute(cmd); //execute the command...
						   }
						   else if(ret_val == 1)
						   {
						     //execute some in-built commands...
						       shell_cmd_exec(cmd);     
						       
						   }
						   else 
						   {
						        printf("%s: command not found\n", cmd);
						   }
					    }
					    else
					    {
						    if((fd = open(cmd, O_RDONLY)) > 0)
						    {
							    close(fd);
							    process_execute(cmd);
						    }
						    else { printf("%s: command not found\n", cmd); }
					     }
					    }					    
					    //free the memory associated with declarations....
					        free_argv();
					        create_prompt();
					        bzero(cmd, 100);  //regain to normal state for other commands processing...
				        }
				        //free_alias_argv();
				        bzero(comand, 100);
				        break;
			default: strncat(comand, &c, 1); //default commands accumulation...
				        break;
		}
	}
	free_alias_argv();
	printf("\n");
	return 0;
}


void welcome_msg()
{
    printf("\n Welcome to Shell !");
    printf("\n Created by Sunny & Parth !");
}

/*Function to store profile file variables & PATH directories
into data structures for commands processing...
it also separate arguments from commands....*/

void profile_file(char *profile_filename)
{
    FILE *fp;
    int i=0;
    char *home1_str="HOME=";
    char *path_str="PATH=";
    char *temp_str=NULL;
    char **fileptr=NULL;
    char *tmp, *home_path;
  
    int len_cnt=256;
          
    printf("\n file  is=%s", profile_filename);
    fp= fopen(profile_filename, "r");
    
    if (fp ==NULL)
    {
        printf("\n\t file open failed");
        exit(1);
    }
    
    fileptr = (char **) malloc(len_cnt * 50);
          
    if(fileptr==NULL)
    {
        printf("\n Unable to allocate the memory...malloc failed..");
    }
  
    //home_str is HOME path defined into profile file...    
    //path_str is PATHE path defined into profile file...    
    
    while (getline(&fileptr[i], &len_cnt, fp) != -1)
    {        
        if (strstr(fileptr[i],home1_str))                                                                  
        {
            home_flag=1;                      
            find_path(fileptr[i], home1_str, home_flag);
        }            
        else if(strstr(fileptr[i],path_str))                                                                  
        {        
            path_flag=2;                       
            find_path(fileptr[i], path_str, path_flag);
        }
        i++;
    }          
    if(home_flag!=1)
        printf("\n HOME path does not exists !");
    if(path_flag!=2)       
        printf("\n PATH variable does not exists !");
        
    fclose(fp);
}

/*Find path for HOME & PATH directory into
.profile file.....& store them*/

void find_path(char *dest_str, char *src_str, int flag)
{

    char temp[MAX_PROFILE_CHR], temp1[MAX_PROFILE_CHR];
    char ret[100];
    char *temp2;
    int i=0, j=0, k=0;
    int  index1=0, index=0;       
    bzero(ret, 100);
    
    while(dest_str[i] == src_str[i])
    {        
        i++;
        index1++;
    }    
    for(i=0; dest_str[i] != '\n'; i++)
    {
        temp[i]=dest_str[i+index1];
    }
    
    strncat(temp,"\0",1);
            
    if(flag==1)
    {
        strncpy(home_dir_path,temp, strlen(temp)-1);            
        strcat(home_dir_path,"\0");
    }
    else if (flag==2)
    {   
        strcpy(path_dir_path,temp);        
        char *temp2=temp;
        
      	while(*temp2 != '\0') 
      	{
        	if(*temp2 == 58 || *temp2 == 10) 
        	{
    			strncat(ret, "/", 1);
    			path_tokens[index] = (char *) malloc(sizeof(char) * (strlen(ret) + 1));
    			strcpy(path_tokens[index], ret);
    			strncat(path_tokens[index], "\0", 1);
    			index++;
    			bzero(ret, 100);
    		} 
    		else
		    {
			    strncat(ret, temp2, 1);
		    }
		    temp2++;
	    }                                       
    }        
}

// search environment variable into memory....
int search_env_var(char *shell_str)
{
    int count = 0;
    int i = 0;
    char *tkn, val[10],var[10];
	char shell_var[50];
    char *src=shell_str;
    
    strcpy(shell_var, src);	
    
	//printf("incoming string is %s\n", shell_var);

    if( strchr(shell_var,' ') )
    {
		//printf("ohoooo  not a shell variable\n");
                return -1;
	}
	i = strlen(shell_var);
	while((shell_var[count] != '=') && (count <= i))
	{
		//printf("at %d is %c\n", count, shell_var[count]);
		count++;
	}
	if(count == i)
	{
        //printf("no key value pair, its invalid =\n");
        return -1;
    }
    
    int flag=0;
    i=0;
    while(shell_var[i] != '\0')
    {
        if(shell_var[i] == '=')
        {
            flag=1;
        }
        i++;
    }
    
    if(flag != 1)
        return -1;
        
    return 1;	
}

//env_check to store all entered variables on Shell....
int env_check( char *shell_str)
{

    if(search_env_var(shell_str) != 1)
    {
        //printf("Not a shell variable\n");
        return -1;
    } 
	else
	{
        env_tokens[ind]=(char*)malloc(sizeof(char) * strlen(shell_str)+1);
        strcpy(env_tokens[ind], shell_str);
        strncat(env_tokens[ind],"\0",1);
        ind++;    
	}
	return 1;
}


int execute_if_else()
{
    int i=0;
    int found=-1;        
    char *ptr2=ifelse_tokens[0]; //stores IF..else parsed strings...
    char temp[100];
    
    bzero(temp,100);
    
    while(*ptr2 != '\0')
    {
        if(*ptr2 == '=' ||*ptr2 == '>' || *ptr2 == '<')
        {
            strncat(temp,"\0",1);
            break;
        }    
        else
        {   
            strncat(temp,ptr2,1);             
        }
        ptr2++;
    }
    
    for(i=0;i<ind;i++)   // ind= env global counter...
    {        
        if(strstr(env_tokens[i],temp))
        {
            found=1;
            return found;
        }             
    }            
    return found;
}

/*to find the comparison in IF...ELSE construct...*/
int find_values(char *val1, char *val2, int k)
{
    int len1=strlen(val1);
    int len2=strlen(val2);
    int sum1=0,sum2=0, i=0;
    char str1[100], str2[100];
    
    strcpy(str1,val1);
    strcpy(str2,val2);
    
    for(i=0;i<len1;i++)
    {
        sum1=sum1 + str1[i];
    }
    for(i=0;i<len2;i++)
    {
        sum2=sum2 + str2[i];
    }
    
    //checking on ASCII values for strings....
    if((sum1==sum2) && k==1)
        return 0;
    else
        return 10;
    
    if((sum1>sum2) && k==2)
        return 0;
    else
        return 10;
    
    if((sum1<sum2) && k==3)
        return 0;
    else
        return 10;
}

/*print statements from IF...ELSE loop....*/
void print_stmnts(char *stmnt)
{
    char *tmp=stmnt;
    char dest[100];
    
    bzero(dest,100);
    
    while(*tmp != '\0')
    {
        if(*tmp == '=')
        {
            bzero(dest,100);            
        }
        else
        {
            strncat(dest,tmp,1);
        }                
        tmp++;
    }
    strncat(dest,"\0",1);
    
    printf("\n %s", dest);
        
}

/*Main process block for IF..ELSE struct...*/
void process_blocks()
{
    char *ptr1=env_tokens[0];
    char *ptr2=ifelse_tokens[0];
    int found,result;
    char var1[30],val1[30],ops1;
    char var2[30],val2[30];
    int ops;
    
    char env_val[20], if_val[20];
    
    char dest[100];
    bzero(dest,100);
    
    bzero(env_val,20);
    bzero(if_val,20);
    
    found=execute_if_else(); 
    
    /*if..else loop variable match found...so process*/
    
    if(found==1)        
    {
        //variable is there...& match found..        
        
        //get value of environment variable stored into memory...       
        while(*ptr2 != '\0') 
        {
            if(*ptr2== '=' || *ptr2== '>' || *ptr2== '<' )
            {
                if(*ptr2 == '=')
                    ops=1;
                else if(*ptr2 == '>')
                    ops=2;
                else if(*ptr2 == '<')
                    ops=3;                                                 
                bzero(if_val,100);
            }
            else
            {
                strncat(if_val,ptr2,1);
            }
            ptr2++;            
        }       
       
        strncat(if_val,"\0",1); 
 
        //get value from IF...ELSE construct...             
        while(*ptr1 != '\0')    
        {
           if(*ptr1 == '=')
           {
             bzero(dest,100);  
           }
           else
           {
                strncat(dest,ptr1,1);
           }
           ptr1++;                                           
        }                              
        strcpy(env_val,dest);        
                   
        // check for operator found....                   
        if(ops==1)
        {
            // "=" comparison operator found..
            result=find_values(if_val,env_val,ops);
            if(result==0)
                //printing THEn part of IF...LOOP
                print_stmnts(ifelse_tokens[1]);
            else if(result==10 && ifelse_cnt==2 )
                print_stmnts(ifelse_tokens[2]);            
            else
                printf("\n nothhing to display...");
                                    
        }
        else if(ops==2)
        {
            // > operator found...
            result=find_values(if_val,env_val,ops);
            if(result==0)
                print_stmnts(ifelse_tokens[1]);
            else if(result==10 && ifelse_cnt==2)
                print_stmnts(ifelse_tokens[2]);
            else
               printf("\n nothhing to display...");
        }
        else if(ops==3)
        {
            //< operator found....
            result=find_values(if_val,env_val,ops);
            if(result==0)
                print_stmnts(ifelse_tokens[1]);
            else if(result==10 && ifelse_cnt==2)
                print_stmnts(ifelse_tokens[2]);                                               
            else
                printf("\n nothhing to display...");
        } 
        else // other invalid operator found...
            printf("\n invalid operand or not found...");
                                                                             
    }
    else
    {
        //found does not match...so first store variable 
        //into memory & then execute IF...ELSE..
        printf("\n variable does not exists into memory...");
    }
                 
}

//this is the main function to if...execute...

void ifelse_process(char *stmnt)
{
    char temp[100];
    
    strcpy(temp,stmnt);
    
    parse(temp); //Parse tokens now in ifelse_tokens...
    
    process_blocks();    // building block for IF...ELSE    
}

//To Parse IF...ELSE structure....
void parse(char *parse_str)
{
    
    int i=0,k=0;
    char *src=parse_str;
    char dest[100], tmp[100];
               
    bzero(tmp,100); 
    bzero(dest,100);
          
    strcpy(tmp,parse_str);
    strncat(tmp,"\0",1);
            
    while(*src != '\0' )
    {
        if(*src == ' ' || *src == ';' ) 
        {           
            if(*src == ';')
            {
                //store values into global struct for IF...ELSE for
                //further processing....
                ifelse_tokens[k]=(char *)malloc(sizeof(char) * strlen(dest)+1);                                
                strcpy(ifelse_tokens[k], dest);
                strcat(ifelse_tokens[k++],"\0");
                bzero(dest,100);
                //put this expr into ifelse--tokens        
            }
            src++;
        }
        else
        {
            //check for IF..ELSE construct & divide into strings...           
            if(strcmp(dest,"if")==0 || strcmp(dest,"then")==0 || strcmp(dest,"else")==0)
            {
                bzero(dest,100);
                
            }
            else if(strcmp(dest,"echo") == 0 )
            {
                strncat(dest,"=",1);
                strncat(dest, src,1);
                src++;
            }
            else
            {
                strncat(dest,src,1);
                src++;
            }
        }    
    
    }
        
}

//Check for IF...ELSE Loop existance...
int ifelse_check(char *expr)
{
    char *temp, tmp1[100];
    char str[100];
    char *token;
    int cnt2=0, ret;
    
    ifelse_cnt=0;
    
    bzero(str, 100);

    strcpy(tmp1,expr);
    
    temp=tmp1;
        
    token=strtok(temp," ;then ");
    
    if(strcmp(token,"if") == 0)
    {
        cnt2++;
        if(strtok(NULL," ;then"))
        {
            ifelse_cnt++;
            if(strtok(NULL," ;else"))
                ifelse_cnt++;                
        }
        return ifelse_cnt;
    }                       
    else if(cnt2==1)
        printf("\n error on if...else..construct!!!...");        
    return ifelse_cnt;        
}



/*Shell Built-In commands starts here....
we suport CD, EXIT commands....*/


//Execute CD commands.....
void exec_cd(char *dirs)
{    
    char cwd[200];
    if (string_tokens[1] == NULL)
    {
        fprintf(stderr, " expected argument to \"cd\"\n");
    }     
    else
    {
        
        if (chdir(string_tokens[1]) != 0)
        {
          perror("cd");
          printf("\n directory does not exists...");
        }
        getcwd(cwd,sizeof(cwd));
        strcpy(home_dir_path,cwd);
    }       
}

void exec_echo(char *dirs)
{
    char cwd[1200];
    char *token;
    
    token=strtok(dirs," ");
    if(strcmp(token,"echo")==0)
    {
        token=strtok(NULL,"\n");
        printf("\n %s", token);
            
    }
    

}

//Execute EXIT command....
void exec_exit()
{
    exit(0);
}

//Function to route calls depending on built-in functions....
void shell_cmd_exec(char *scmd)
{

    if(strcmp(scmd,"cd")==0 )
    {
        exec_cd(scmd);   
    }
    else if(strcmp(scmd,"echo")==0 )
    {
        exec_echo(scmd);
    }
    else if(strcmp(scmd,"exit")==0 )
    {
        printf("\n Shell Exiting...Bye");
        exec_exit();
    }
    else
    {
        printf("\n Unknown In-build command...");    
        create_prompt();
    }
}

/*ALIAS construct starts here....
check whether alias exists or not.....*/

int check_alias(char *argu)
{
    char *temp, tmp1[100];
    char str[100];
    char *token;
    FILE *fptr;
    int cnt=0, ret=-1;
    
    bzero(tmp1,100);
    bzero(str, 100);
    strcpy(tmp1,argu);
    
    temp=tmp1;
        
    token=strtok(temp," ");
    
    //if keyword found......
    if(strstr(token,"alias"))
    {
        cnt++;
            
        token=strtok(NULL,"='");
        if(token)
        {
            strcat(str,token);
            strcat(str,"=");
            strcat(str,"\0");
            cnt++;
        }
        else
            printf("\n alias format is invalid...");

        token=strtok(NULL,"'");

        if(token)
        {
            strcat(str,token);
            strcat(str, "\0");  
            cnt++;
        }
        else
            printf("\n alias format is invalid...");
    }
                      
    ret=search_alias(temp);
    if(cnt==3 && ret==0)
    {
        
        alias_tokens[alp]=(char*)malloc(sizeof(char) * strlen(str)+1);
        strcpy(alias_tokens[alp], str);
        strncat(alias_tokens[alp],"\0",1);
        alp++;
    } 
          
    return cnt;
}

//Search alias into preddefined memory of shell...
int search_alias(char *key)
{
    int k, found=0;
    
    for(k=0; k<alp; k++)
    {
        if(strstr(alias_tokens[k],key) != NULL)
        {
            found = 1;
            //printf("\n alias already exists...Re-Enter New");                                 
        }        
        
    }    
    return found;
}


//Find alias from memory & validate.....
char *find_alias(char *alia)
{
    FILE *fp;
    int i=0,found=0, len=256;
    int k=0;
    char *temp;    
    char ret[100], *src, tmp1[100];
    
    bzero(ret,100);
    bzero(tmp1,100);    
    strcpy(tmp1,alia);
    temp=tmp1;        
    strcat(temp,"=");
    strcat(temp,"\0");
    
    /*alias_tokens keeps all alias variables & process them 
    for later use,....
    ind variable maintains alias count....*/
    
    for(k=0; k<alp; k++)
    {
        if(strstr(alias_tokens[k],temp) != NULL)
        {
            src=alias_tokens[k];

            while(*src != '=')
                src++;                     

            src++;

            while(*src != '\0')
            {
                strncat(ret,src,1);
                src++;
            }
            strcat(ret,"\0");            
            strcpy(alia,ret);            
        }
        
    }    
    return alia; 
}

/*store entered command into memory 
and check whether it is valiad or not...*/

void fill_argv(char *tmp_argv)
{

	char *foo = tmp_argv;
	int index = 0;
	char ret[100];
	bzero(ret, 100);
	
	while(*foo != '\0')
	{
		if(index == 10)
			break;
        /*store entered commands into global
        array to send it to the EXEC family of calls...*/
        
		if(*foo == ' ')
		{
			if(string_tokens[index] == NULL)
				string_tokens[index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
			else {
				bzero(string_tokens[index], strlen(string_tokens[index]));
			}
			strcpy(string_tokens[index], ret);
			strncat(string_tokens[index], "\0", 1);
			bzero(ret, 100);
			index++;
		} 
		else
		{
			strncat(ret, foo, 1);
		}
		foo++;		
	}
	string_tokens[index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
	strcpy(string_tokens[index], ret);
	strncat(string_tokens[index], "\0", 1);
}

/*EXEC calls for shell commands are executed here....*/

void call_execve(char *cmd)
{
	int i;
	int pid;
	printf("cmd is %s\n", cmd);	
		
	pid=fork();
	if(pid == 0)
	{
		i = execv(cmd, string_tokens);
		
		if(i < 0)
		{
		    printf("errno is %d\n", errno);
			printf("%s: %s\n", cmd, "command not found");
			exit(1);		
		}
	} 
	else
	{
		wait(NULL);
	}
}

/*To create prompt on shell & Display it....*/
void create_prompt()
{
    
    char prompt[256];
    char cwd[100];
    int i=0;
    bzero(prompt,256);

    for(i=0; home_dir_path[i]!='\0'; i++)
    {
        prompt[i]=home_dir_path[i];
    }
    //printf("\n prompt=%d", strlen(prompt));
    if (chdir(prompt)!= 0)
    {
        printf("\n Errorooo...");
        perror("chdir");
        exit(1);
    }

    strcat(prompt,":$$]#");    
    printf("\n%s",prompt);
}

/*check whether it is a command or random word...*/
int check_command(char *temp)
{
    char sample[100];
    bzero(sample, 100);
    
    int i,fd, cnt;
    
    for(i=0; i<3; i++)
    {
        if(strcmp(temp,shell_cmd[i])==0)
        {
            cnt=1;
            return cnt;
        }
        else
            cnt=2;    
    }    
    if(cnt==2)
    {    
        for(i=0; path_tokens[i]!=NULL; i++)
        {
            strcpy(sample, path_tokens[i]);

            strcat(sample,temp);

            if((fd=open(sample,O_RDONLY)) > 0)
            {
                strncpy(temp, sample, strlen(sample));
                close(fd);
                return 0;
            }            
            else
                bzero(sample,100);
        }      
    }
    return 0; 
}


/*to handle CTRL-C signal....*/
void handle_signal(int signo)
{
	create_prompt();
	fflush(stdout);
}

/*free the buffer used for shell process...*/
void free_argv()
{
	int index;
	for(index=0;string_tokens[index]!=NULL;index++)
	{
		bzero(string_tokens[index], strlen(string_tokens[index])+1);		
		string_tokens[index] = NULL;		
		free(string_tokens[index]);
	}
	
	/*for(index=0;alias_tokens[index]!=NULL; index++)
	{
	    bzero(alias_tokens[index], strlen(alias_tokens[index])+1);		
		alias_tokens[index] = NULL;		
		free(alias_tokens[index]);
	
	}*/

	for(index=0;ifelse_tokens[index]!=NULL; index++)
	{
	    bzero(ifelse_tokens[index], strlen(ifelse_tokens[index])+1);		
		ifelse_tokens[index] = NULL;		
		free(ifelse_tokens[index]);
	
	}

}

void free_alias_argv()
{
	int index;
	
	for(index=0;alias_tokens[index]!=NULL; index++)
	{
	    bzero(alias_tokens[index], strlen(alias_tokens[index])+1);		
		alias_tokens[index] = NULL;		
		free(alias_tokens[index]);
	
	}

}



/*Call to EXEC family of calls here....*/
void process_execute(char *cmd)
{
    int pid, i;    
    pid= fork();
    if(pid==0)
    {
        i=execv(cmd, string_tokens);
        if(i<0)
        {
            printf("\n command not found...%s", cmd);
            perror("cmd");
            exit(EXIT_FAILURE);
        }            
    }
    else
    {
        //parent process...
        wait(NULL);      
    }    
}



