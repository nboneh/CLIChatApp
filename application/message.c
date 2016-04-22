/* hw1.c -- Nir Boned cmd line
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>



void interactiveMode(){
 
}

int main() {
  char cmd[BUFSIZ];

  //Getting username for prompt   
  char username[BUFSIZ];
  getlogin_r(username, BUFSIZ);

  printf("Welcome %s to CLI Messaging\n", username);              

  while(1){
    char * res;
    //Prompting user for input
    printf(">>> ");                                                                                                                                                                
    res = fgets (cmd, BUFSIZ, stdin);

    //Removing new line from user input
    if ((strlen(cmd)>0) && (cmd[strlen (cmd) - 1] == '\n'))
        cmd[strlen (cmd) - 1] = '\0';


    //Exiting on quit command
    if(strcmp(cmd,"quit") == 0){
      printf("Goodbye\n");
       break;
     }
    
    //Exiting on 'Ctrl-D'
    if(res == NULL){
      printf("\n");
      printf("Goodbye\n");
      break;
    }

  } 
  return 0; 
}