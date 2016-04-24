/* message.c -- Nir Boneh messaging application
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include "hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define APP_PORT "3490" 

map_t contacts;
// a backwards version of the contacts hash map to look up a contact name based on ip
map_t backwardscontacts;
FILE * contactsWriteFile;

int messageMode = 0;
int receiveRequest =0 ;
char * messagingContactName;
char * messagingContactIP;

char messageFileName[80];
char mypublicKey[2048];
char otherpublicKey[2048];
char cmd[BUFSIZ];
int sockfd =-1;
int listenfd = -1;
int closeThreads = 0;


pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;


void handShake(int sender){
  printf("Establishing handshake...");
  //Performing Diffie-Hellman
  if(sender){
    //Sender generates public dh key, his private key and the mixture to send
    system("bash gendhpub.sh");
    system("bash gendhpri.sh");
    system("bash gendhpub2.sh");

    //Reading in dh public key 
    FILE *  file = fopen("savefiles/dhp.pem","r");
    char public[2048];
    int i = 0;
    int c;
    while ((c = fgetc(file)) != EOF)
    { 
        public[i++] = (char) c;
    }
    public[i] = '\0';
    fclose(file);

    //Sending dh public key
    send(sockfd, public, 2048, 0);

    //Reading in mixture to send
    file = fopen("savefiles/dhpub.tem", "r");
    char mixture[2048];
    i = 0;
    while ((c = fgetc(file)) != EOF)
    { 
        mixture[i++] = (char) c;
    }
    mixture[i] = '\0';
    fclose(file);

    //Sending mixture
    send(sockfd, mixture, 2048, 0);

    //Waiting to receive recpient mixture
    char mixture2[2048];
    recv(sockfd, mixture2, 2048,0);

    //Writing needed mixture to a file
    file = fopen("savfile/dhpubmix.tem", "w");
    fputs(mixture2, file);
    fclose(file);

    //Generating the secret key
    system("bash gendhpri2.sh");

    //...
    //send(sockfd, encpublicKey, 2048, 0);
    //recv(sockfd, encotherpublicKey,2048, 0);
  } else {
    //Receiving public DH from sender
     char public[2048];
    recv(sockfd, public, 2048, 0); 
    //Writing public key to file
    FILE *  file = fopen("savefiles/dhp.pem","w");
    fputs(public, file);
    fclose(file);

    //Generating private key and mixture to send
    system("bash gendhpri.sh");
    system("bash gendhpub2.sh");
    //Waiting to receive needed mixture to generate secret
    char mixture2[2048];
    recv(sockfd, mixture2, 2048,0);

    //Writing needed mixture to a file
    file = fopen("savfile/dhpubmix.tem", "w");
    fputs(mixture2, file);
    fclose(file);

    //Generating the secret key
    system("bash gendhpri2.sh");

    //Reading in mixture to send
    file = fopen("savefiles/dhpub.tem", "r");
    char mixture[2048];
    int i = 0;
    int c;
    while ((c = fgetc(file)) != EOF)
    { 
        mixture[i++] = (char) c;
    }
    mixture[i] = '\0';
    fclose(file);

    //Sending mixture
    send(sockfd, mixture, 2048, 0);
    //..
    //recv(sockfd, encotherpublicKey,2048, 0);
   // send(sockfd, encmypublicKey, 2048, 0);
  }
}

void print_current_time_with_ms ()
{
  system("bash time.sh");
}

void encryptRSA(char *inString, char *outString){
  FILE * file = fopen("temppubkey", "w");
  fputs(otherpublicKey,file);
  fclose(file);
  file = fopen("tempintext","w");
  fputs(inString,file);
  fclose(file);
  system( "bash encryptrsa.sh");

  unlink("temppubkey");
  unlink("tempintext");

  file = fopen("file.bin","r");
  int i = 0;
  int c;
  while ((c = fgetc(file)) != EOF)
  { 
        outString[i++] = (char) c;
   }
   outString[i] = '\0';
  fclose(file);
  unlink("file.bin");
}

void decryptRSA(char *inString, char *outString){
   FILE * file = fopen("temppubkey", "w");
  fputs(otherpublicKey,file);
  fclose(file);
  file = fopen("tempintext","w");
  fputs(inString,file);
  fclose(file);
  system( "bash decryptrsa.sh");

  unlink("temppubkey");
  unlink("tempintext");

  file = fopen("file.bin","r");
  int i = 0;
      int c;
  while ((c = fgetc(file)) != EOF)
  { 
        outString[i++] = (char) c;
   }
   outString[i] = '\0';
  fclose(file);
  unlink("file.bin");

}

void loadPubKey(){
  FILE * fp = fopen("savefiles/my.key.pub","r");
  int i = 0;
      int c;
  while ((c = fgetc(fp)) != EOF)
  { 
        mypublicKey[i++] = (char) c;
   }
   mypublicKey[i] = '\0';
  fclose(fp);
}

int fileEmpty(FILE * file){
   fseek(file, 0, SEEK_END);

    if (ftell(file) == 0){
        return 1;
    }

    return 0;

}

void printMessageMode(){
   //Getting number of lines in the terminal to set up a nice window
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  int lines = w.ws_row;
  int i;
  FILE * fp;
  fp = fopen(messageFileName, "r");
  //Writing to screen from messaging file to show converstation
  if(fp != NULL){
    //Read from message file
   char * line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, fp)) != -1) {
      if(line[0] == '*'){
        //A * indicate my message
        line[0] = ' ';
        printf("Me%s",line);
      }
      else
        printf("%s: %s", messagingContactName, line);
      lines--;
    }
  }

  for(i = 0; i < (lines); i++){
      printf("\n");
  }

  printf("Messaging %s, type quit to stop: ",messagingContactName);

  if(fp != NULL)
    fclose(fp);
}

void sendMessage(char* message){
  time_t rawtime;
  char * timestr;
  rawtime = time(NULL);


  timestr = ctime ( &rawtime );
  timestr[strlen(timestr) -1] = '\0';

  char compmessage[1024];
  snprintf(compmessage, 1024, "[%s]: %s", timestr, message);

  //PerformingRsa
  char encryptmessage[1024];
  encryptRSA(compmessage, encryptmessage);
 
  pthread_mutex_lock(&lock); 
  //Critical section do not want to edit file at the same time as listener receives message
  if(send(sockfd, encryptmessage, 1024, 0) < 0){
    //Error occured exiting
    printf("Error occured\n");
    pthread_mutex_unlock(&lock); 
    messageMode = 0;
  }

  FILE * messagefile = fopen(messageFileName, "a");
  if(!fileEmpty(messagefile)){
    fputs("\n", messagefile);
  }
  //indicating message from self
  fputs("*", messagefile);
  fputs(compmessage, messagefile);

  fclose(messagefile);
  fflush(stdout);
  pthread_mutex_unlock(&lock); 
}

void *receiveMessage(){
  while(1){
    char encryptedmessage[1024];
    int ret =recv(sockfd, encryptedmessage,1024, 0);

    if(messageMode == 0){
      //Exiting when message mode is done
      return NULL;
    }

    if(ret <= 1){
      //Remote hang up exiting
      printf("\n");
      printf("Remote hang up\n");
      printf(">>> ");
      fflush(stdout);
     messageMode = 0;
      pthread_mutex_unlock(&lock); 
      return NULL;
    }

    char message[1024];
    decryptRSA(encryptedmessage, message);
   pthread_mutex_lock(&lock); 
  //Critical section do not want to edit file at the same time as sender sends message
  FILE * messagefile = fopen(messageFileName, "a");
  if(!fileEmpty(messagefile)){
    fputs("\n", messagefile);
   }
    //indicating message from self
    fputs(message, messagefile);

    fclose(messagefile);

    printf("\n");
    printMessageMode();
    fflush(stdout);
    struct timeval start;
    pthread_mutex_unlock(&lock); 
  }
}

void setupMessaging(char* reference){
  char * referencecop = malloc(strlen(reference) + 1); 
  strcpy(referencecop, reference);
  referencecop[strlen(referencecop) + 1] ='\0';

  messagingContactName = referencecop;

  char  *value;
  int error = hashmap_get(contacts, referencecop, (void**)(&value));
  if(error==MAP_OK){
    //User enter a contact using that contact's ip
    messagingContactIP = value;
  } else {
    //User entered an ip using that ip
    messagingContactIP = referencecop;
  }
  snprintf(messageFileName,sizeof messageFileName,"savefiles/%s.txt",messagingContactIP );

}

void *listenForConn()
{
  //Runs on seperate thread waiting for connection calls
     struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
  while(1 ){
        //Runs on seperate thread listening for connection

       // first, load up address structs with getaddrinfo():

       memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;  
        hints.ai_socktype = SOCK_STREAM;
       hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

      getaddrinfo(NULL, APP_PORT, &hints, &res);

    if(listenfd < 0){
      int reuse_addr = 1;
       // make a socket, bind it, and listen on it:
      listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
      setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(int));
      bind(listenfd, res->ai_addr, res->ai_addrlen);

      listen(listenfd, 2);
    }
    // now accept an incoming connection:

    addr_size = sizeof their_addr;
    struct sockaddr_in *sin = (struct sockaddr_in *)&their_addr;
    int newSock = accept(listenfd, (struct sockaddr *)&their_addr, &addr_size);
    if(closeThreads ){
      return NULL;
    }
    if(messageMode || receiveRequest){
      //Do not disturb user while he is messaging
      close(newSock);
      continue; 
    }
    sockfd = newSock;
    unsigned char *brokeIp = (unsigned char *)&sin->sin_addr.s_addr;
    char ip[80];
    snprintf(ip, sizeof ip, "%d.%d.%d.%d", brokeIp[0], brokeIp[1], brokeIp[2], brokeIp[3]);  

  char  *reference;
   int error = hashmap_get(backwardscontacts, ip, (void**)(&reference));
   if(error==MAP_OK){
     //Use reference to set up messaging
    } else {
      //Use ip to set up messaging
      reference = ip;
    }
    setupMessaging(reference);
      receiveRequest = 1;
       printf("\n%s would like to start messaging, type a to accept and r to reject\n", messagingContactName); 
       fflush(stdout);

  }
}

void startMessaging(char* reference)
{

 setupMessaging(reference);
  struct addrinfo hints, *res;
  // first, load up address structs with getaddrinfo():
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET; 
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

  getaddrinfo(messagingContactIP, APP_PORT, &hints, &res);

  // make a socket:

  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

// connect!

if(connect(sockfd, res->ai_addr, res->ai_addrlen)!= 0){
  printf("Something went wrong\n");
  return;
}
printf("Connection made waiting for other user to accept or reject\n");
char buf[1];
int ret =recv(sockfd, buf,1, 0);
if(ret == 0 || buf[0] == 'r'){
  printf("Connection rejected\n");
  close(sockfd);
} else if(buf[0] == 'a'){

  messageMode = 1;
  handShake(1);
  pthread_t recthread;
  pthread_create(&recthread, NULL, receiveMessage, NULL);     

} else {
  printf("Something went wrong\n");
  close(sockfd);
}
}

void  splitBySpaces(char *cmd, char **argv)
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
     *argv = NULL;                
}


int parseShow(char *ip, char *contactname){
  printf("%s, %s\n", ip, contactname);
  return MAP_OK;
}

int parseWrite(char *ip, char *contactname){
    fputs(ip, contactsWriteFile);
    fputs(" ", contactsWriteFile);
    fputs(contactname, contactsWriteFile);
    fputs("\n", contactsWriteFile);
    return MAP_OK;
}

void loadContacts(){
  contacts = hashmap_new();
  backwardscontacts = hashmap_new();

  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  fp = fopen("savefiles/contacts.txt", "r");
  if (fp == NULL){
      //Contacts file not created yet 
      return;
  }

  while ((read = getline(&line, &len, fp)) != -1) {
      int argv_size = 2;
      char *argv[argv_size];
      splitBySpaces(line, argv);
      hashmap_put(contacts,argv[0], argv[1]);
      hashmap_put(backwardscontacts, argv[1], argv[0]);
  }

  fclose(fp);
  if (line)
      free(line);
}

void saveContacts(){
  contactsWriteFile = fopen("savefiles/contacts.txt","w");
  PFany parser = (PFany)&parseWrite;
  hashmap_iterate(contacts, parser);
  fclose(contactsWriteFile);
}

void add(char *IP, char *contactname){
  int error = hashmap_put(contacts, contactname, IP);
  hashmap_put(backwardscontacts, IP, contactname);
  if(error==MAP_OK){
    printf("Successfully added %s\n", contactname);
  }else {
    printf("Error adding %s\n", contactname);
    return;
  }
  saveContacts();
}

void removec(char *contactname){

  char  *ip;
  int error = hashmap_get(contacts, contactname, (void**)(&ip));
  if(error==MAP_OK){
     hashmap_remove(contacts, contactname);
    hashmap_remove(backwardscontacts, ip);
    printf("Successfully removed %s\n", contactname);
  }else {
    printf("Error removing %s\n", contactname);
    return;
  }
  saveContacts();
}


int main() {
  //Creating save files folder if does not exist
   struct stat st = {0};
  if (stat("savefiles", &st) == -1) {
       mkdir("savefiles", 0700);
       //Generating RSA keys using bash
       system("bash genrsa.sh");
  }
  //Loading contacts
  loadContacts();

  //loading public key 
  loadPubKey();

  //Getting username for prompt   
  char username[BUFSIZ];
  getlogin_r(username, BUFSIZ);


  printf("Welcome %s to CLI Messaging\n", username);      

    //Creating listening connection thread
    pthread_t thread;
    pthread_create(&thread, NULL, listenForConn, NULL);      

  while(1){
    char * res;
    //Prompting user for input
    if(messageMode){
      printMessageMode();
    } else if(receiveRequest){
         printf("%s would like to start messaging, type a to accept and r to reject ", messagingContactName);
    }
    else 
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

    if(receiveRequest){
      if(strcmp(cmd, "a") ==0 ){    
        messageMode = 1;
        receiveRequest = 0;
        char * acceptmsg = "a";
        send(sockfd, acceptmsg, strlen(acceptmsg),0);
        handShake(0);
        pthread_t recthread;
        pthread_create(&recthread, NULL, receiveMessage, NULL); 
      } else if(strcmp(cmd, "r") ==0 ){
        receiveRequest = 0;
        char * rejectmsg = "r";
        send(sockfd, rejectmsg, strlen(rejectmsg),0);
        close(sockfd);
      }
    } else if(strlen(cmd) >= 7 && cmd[0] == 'm' && cmd[1] == 'e' && cmd[2] == 's' && cmd[3] == 's' && cmd[4] == 'a' && cmd[5] == 'g' && cmd[6] == 'e'){
      //Message contact 
       //add contact
      int argv_size = 2;
      char *argv[argv_size];
      argv[1] = NULL;
      splitBySpaces(cmd, argv);

      if(argv[1] == NULL){
        printf("Invalid use of message command, type help for list of commands\n");
        continue;
      }
      startMessaging(argv[1]);
    } else if(strlen(cmd) >= 3 && cmd[0] == 'a' && cmd[1] == 'd' && cmd[2] == 'd'){
      //add contact
      int argv_size = 3;
      char *argv[argv_size];
      argv[1] = NULL;
      argv[2] = NULL;
      splitBySpaces(cmd, argv);

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
      splitBySpaces(cmd, argv);

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
    } else if(strcmp(cmd, "IP") == 0){
      system("bash myip.sh");
    } else if(strcmp(cmd, "help") == 0){
      //help
      printf("Commands:\n");
      printf("message (IP/contactname) - start messaging with the specific IP address or contact name\n");
      printf("add (IP) (contactname) - this will add a new contact based on IP address\n");
      printf("remove (contactname) - allows you remove a contanct\n");
      printf("contacts - shows all your contacts\n");
      printf("IP - shows you your IP addresses\n");
      printf("quit or Ctrl-D - in order to exit the application\n");
    } else if(strcmp(cmd,"quit") == 0){
      if(messageMode){
        messageMode = 0;
        close(sockfd);
      }
      //Exiting on quit command
      else {
        printf("Goodbye\n");
        break;
      }
    }else{
      if(messageMode){
        sendMessage(cmd);
      }
      else 
        printf("Invalid command, type help for list of commands\n");
    }
  } 

  saveContacts();
  hashmap_free(contacts);
  closeThreads = 1;
  if(sockfd >= 0){
    shutdown(sockfd,2);
    close(sockfd);
  }
  if(listenfd >= 0){
    shutdown(listenfd,2);
    close(listenfd);
  }

  return 0; 
}