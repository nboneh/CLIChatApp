/* message.c -- Nir Boneh messaging application
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include "hashmap.h"

map_t contacts;

int parseShow(char *ip, char *contactname){
  printf("%s, %s\n", ip, contactname);
  return MAP_OK;
}

void loadContacts(){
  contacts = hashmap_new();
}

void saveContacts(){

}

void  parseCmd(char *cmd, char **argv)
{    
   //Spliting basd on white spaces
   //While not end of cmd
     while (*cmd != '\0') {   
        //Replace all white spaces with 0, this will act as seperator between argv strings
          while (*cmd == ' ' || *cmd == '\t' || *cmd == '\n')  
               *cmd++ = '\0'; 
           //As soon as we reach a character that's not space add its position to argv
          *argv++ = cmd; 
          //Keep cycling until reach next white space or reach end of cmd  or still insideParenthesis    
          while (*cmd != '\0' && *cmd != ' ' && 
                 *cmd != '\t' && *cmd != '\n'){
               cmd++;   
          }
     } 
     //Adding terminating 0 to end of argument list
     *argv = '\0';                
}

void add(char *IP, char *contactname){
  int error = hashmap_put(contacts, contactname, IP);
  if(error==MAP_OK){
    printf("Successfully added %s\n", contactname);
  }else {
    printf("Error adding %s\n", contactname);
    return;
  }
  saveContacts();
}

void removec(char *contactname){
  int error = hashmap_remove(contacts, contactname);
  if(error==MAP_OK){
    printf("Successfully removed %s\n", contactname);
  }else {
    printf("Error removing %s\n", contactname);
    return;
  }
  saveContacts();
}


int main() {

  //Loading contacts
  loadContacts();

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

    //Exiting on 'Ctrl-D'
    if(res == NULL){
      printf("\n");
      printf("Goodbye\n");
      break;
    }
    if(strlen(cmd) >= 3 && cmd[0] == 'a' && cmd[1] == 'd' && cmd[2] == 'd'){
      //add contact
      int argv_size = 3;
      char *argv[argv_size];
      argv[1] = NULL;
      argv[2] = NULL;
      parseCmd(cmd, argv);

      if(argv[1] == NULL || argv[2] == NULL){
        printf("Invalid use of add command, type help for list of commands\n");
        continue;
      }
      add(argv[1], argv[2]);
    } else if(strlen(cmd) >= 6 && cmd[0] == 'r' && cmd[1] == 'e' && cmd[2] == 'm' && cmd[3] == 'o' && cmd[4] == 'v' && cmd[5] == 'e'){
       //remove contact
      int argv_size = 2;
      char *argv[argv_size];
      argv[1] = NULL;
      parseCmd(cmd, argv);

      if(argv[1] == NULL){
        printf("Invalid use of remove command, type help for list of commands\n");
        continue;
      }
      removec(argv[1]);

    } else if(strcmp(cmd, "contacts") == 0){
      //Display contacts
      printf("Contact, IP Address\n");
      PFany parser = (PFany)&parseShow;
      hashmap_iterate(contacts, parser);
    } else if(strcmp(cmd, "help") == 0){
      //help
      printf("Commands:\n");
      printf("message (IP/contactname) - start messaging with the specific IP or contanct\n");
      printf("add (IP) (contactname) - this will add a new contact based on IP address\n");
      printf("remove (contactname) - allows you remove a contanct\n");
      printf("contacts - shows all your contacts");
      printf("quit or Ctrl-D - in order to exit the application\n");
    } else if(strcmp(cmd,"quit") == 0){
      //Exiting on quit command
      printf("Goodbye\n");
      break;
    } else{
      printf("Invalid command, type help for list of commands\n");
    }
  } 

  saveContacts();
  hashmap_free(contacts);
  return 0; 
}