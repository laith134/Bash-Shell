/*
  Name: Laith Marzouq
*/
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10     // Mav shell only supports five arguments

int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  int history_index=0;
  // Started history_count at -1 so the if statement in history coomand will run 15 times
  int history_count=-1;
  char history[14][MAX_COMMAND_SIZE];
  int pid_history[15];
  int pid_index=0;
  int pid_count=0;
  int conv[15];
  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    //Spamming enter or  space then enter will reset the loop
    if (cmd_str[0]=='\n'|| cmd_str[0]==' ') continue;
    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr;
    char *working_str  = strdup( cmd_str );
    char *working_root = working_str;
    // Copy input into History array, once there are 15 elements the index will reset to 0
    // causing the oldest input to be replaced
    strcpy(history[history_index], cmd_str);
    history_index++;
    if (history_count<15) history_count++;
    if(history_index > 15)
    {
      history_index=0;
      strcpy(history[history_index], cmd_str);
      history_index++;
    }
    // Creating a dummy array that will give me the right index for history array
    // when using ! command
    int index2= history_index;
    for(int j=0;j<15;j++)
    {
      conv[j]=index2;
      index2++;
      if(index2 > 14) index2=0;
    }
    // ! command that will take a number <= history counter and bigger than
    if(cmd_str[0] == '!')
    {
      int value= atoi(&cmd_str[1]);
      value--;
      if(history_count<15 && value+1 < history_count+1 && value>=0)
      {
        strcpy(working_str, history[value]);
      }
      else if (history_count==15 && value+1 < history_count+1 && value>=0)
      {
        strcpy(working_str, history[conv[value-1]]);
      }

      else
      {
        printf("Command not in history\n");
        continue;
      }

    }

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }
    if(strcmp(token[0], "exit")==0 || strcmp(token[0], "quit")==0)
    {
      exit(0);
    }
    // chdir() will be called on token[1] which is the argument after cd
    else if(strcmp(token[0], "cd")==0)
    {
      chdir(token[1]);
    }
    else if(strcmp(token[0], "history")==0)
    {
      // Simply printing out to screen the history array when there are less than 15 inputs
      if(history_count<15)
      {
        for(int i=0;i<history_index;i++)
        {
          printf("%d: %s",i+1,  history[i]);
        }
      }
      else
      {
        // Will print out from history_index +1 to 14, then will reset to index 0 and print out the
        // beginning to history_index
        int index1= history_index;
        for(int i=0;i<15;i++)
        {
          printf("%d: %s", i+1, history[index1++]);
          if(index1 > 14) index1=0;
        }
      }
    }
    else if(strcmp(token[0], "showpids")==0)
    {
      // Simply printing out to screen the pid array when there are less than 15 inputs
      if(pid_count<15)
      {
        for(int i=0;i<pid_index;i++)
        {
          printf("%d: %d\n", i+1, pid_history[i]);
        }
      }
      else
      {
        int index= pid_index;
        // Will print out from pid_index +1 to 14, then will reset to index 0 and print out the
        // beginning to pid_index
        for(int i=0;i<15;i++)
        {
          printf("%d: %d\n", i+1, pid_history[index++]);
          if(index > 14) index=0;
        }
      }

    }
    //
    else
    {
      // Creating child
      pid_t pid = fork( );
      // Saving pids in array for showpids command to use, once there are 15 elements. The index
      // will reset to 0 causing the oldest pid to be replaced
      pid_history[pid_index++]= pid;
      if (pid_count<15) pid_count++;
      if(pid_index > 14) pid_index=0;
      // Child running first argument through execvp, accessing /bin library
      if( pid == 0 )
      {

        int ret = execvp( token[0], &token[0] );
        if( ret == -1 )
        {
          printf("%s: Command not found\n", token[0]);
        }
        exit(0);

      }
      else if(pid == -1)
      {
        perror("Error with fork. \n");
      }
      // parent waiting until child is done
      else
      {
        int status;
        wait( & status );
      }
    }
    free( working_root );
  }
  return 0;
}
