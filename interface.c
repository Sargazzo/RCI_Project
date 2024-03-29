#include "interface.h"
#include "structs_n_main.h"


void print(const char* msg){
  printf("%s\n", msg);
}

void Display_menu(){
  printf("|-------------------!Main Menu!------------------|\n");
  printf(" (Choose the number corresponding to your option)\n\n");
  printf("1.New i\n");
  printf("2.Entry i\n");
  printf("3.Sentry i\n");
  printf("4.Leave \n");
  printf("5.Show \n");
  printf("6.Find k i\n");
  printf("7.Exit \n");
}
//Used to get the option in the main Menu
//Returns the chosen option

int get_option(){
  int option = 0;
  char buffer[VETOR_SIZE];
  if(!fgets(buffer, VETOR_SIZE, stdin))
    exit(1);
  sscanf(buffer,"%d", &option);
  if(option > 0 && option < 8)
    return option;
  else
    return -1;
}

int new_i(){

  int key=0;
  char buffer[VETOR_SIZE];
  clrscreen();
  printf("Enter server key:\n");

  do{
    if(key <= 0 || key> RING_SIZE)
      printf("Key must be between the server limits(0-%d)\n",RING_SIZE);

    if(!fgets(buffer, VETOR_SIZE, stdin))
      exit(1);

  }while (sscanf(buffer,"%d", &key) != 1 ||  key <= 0 || key > RING_SIZE);
  return key;
}

//Definir limites para numero de chaves pelo ring size
void sentry(all_info* sv_info){
  char buffer[VETOR_SIZE];
  int key =0;
  sv_info->key=new_i();

  printf("Enter successor key:\n");
  do{
    if(key <= 0 || key> RING_SIZE)
      printf("Key must be between the server limits(0-%d)\n",RING_SIZE);

    if(!fgets(buffer, VETOR_SIZE, stdin))
      exit(1);

  }while (sscanf(buffer,"%d", &key) != 1 ||  key <= 0 || key > RING_SIZE);
  sv_info->succ_key=key;


  printf("Enter successor IP:\n");
  do{
    key=0;
    if(!fgets(buffer, IP_SIZE, stdin))
      exit(1);
    //IP always has dots
    if(strstr(buffer,".")==NULL)
      key=-1;

    //IP length has a limit
    if(strlen(buffer) >= IP_SIZE)
      key=-1;

    if(key==-1)
      printf("Invalid IP, try again\n");

  }while(key==-1);
  strcpy(sv_info->Next_info.IP,strtok(buffer, "\n"));


  printf("Enter successor Port:\n");
  do{
    key=0;
    if(!fgets(buffer, PORT_SIZE, stdin))
      exit(1);
    //Port limits
    if(sscanf(buffer,"%d", &key) < 1 ||  key <= 1024 || key > 64000)
      key=-1;

    if(key==-1)
      printf("Invalid port! It should be between 1024 and 64000\n");

  }while(key==-1);
  strcpy(sv_info->Next_info.port,strtok(buffer, "\n"));
}
//Definir limites para numero de chaves pelo ring size
void entry_i(all_info* sv_info){
  int key=0;
  char buffer[VETOR_SIZE];
  sv_info->key=new_i();

  printf("Enter IP:\n");
  do{
    key=0;
    if(!fgets(buffer, IP_SIZE, stdin))
      exit(1);
    //IP always has dots
    if(strstr(buffer,".")==NULL)
      key=-1;

    //IP length has a limit
    if(strlen(buffer) >= IP_SIZE)
      key=-1;

    if(key==-1)
      printf("Invalid IP, try again\n");

  }while(key==-1);
  strcpy(sv_info->Next_info.IP,strtok(buffer, "\n"));

  printf("Enter Port:\n");
  do{
    key=0;
    if(!fgets(buffer, PORT_SIZE, stdin))
      exit(1);
    //Port limits
    if(sscanf(buffer,"%d", &key) < 1 ||  key <= 1024 || key > 64000)
      key=-1;

    if(key==-1)
      printf("Invalid port! It should be between 1024 and 64000\n");

  }while(key==-1);
  strcpy(sv_info->Next_info.port,strtok(buffer, "\n"));
}

void show(all_info sv_info){
  printf("\n\n=========== SERVER STATUS ===========\n");
  if(sv_info.inRing == false){
    printf("Status: DOWN\n");
    printf("Server IP: %s:%s\n", sv_info.Myinfo.IP, sv_info.Myinfo.port);
    printf("=====================================\n");
    printf("press enter to continue\n");
    return;
  }
  printf("Status: OPERATIONAL\n");
  printf("Server IP: %s::%s\n", sv_info.Myinfo.IP, sv_info.Myinfo.port);
  printf("Server key: %d\n", sv_info.key);
  if(strcmp(sv_info.Next_info.port, sv_info.Myinfo.port))
    printf("Connected to %s::%s key: %d\n", sv_info.Next_info.IP, sv_info.Next_info.port, sv_info.succ_key);
  if(strcmp(sv_info.SecondNext_info.port, sv_info.Myinfo.port) && strcmp(sv_info.SecondNext_info.port, sv_info.Next_info.port))
    printf("Second next server: %s::%s key: %d\n", sv_info.SecondNext_info.IP, sv_info.SecondNext_info.port, sv_info.second_succ_key);
  printf("===================================\n");
  printf("press enter to continue\n");
}

void clrscreen(){
  printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}

void mystrcat(char* result,char* first,char* secnd,char* thrd,char* fourth,char* fifth){
  strcpy(result, first);
  strcat(result, " ");
  strcat(result, secnd);
  strcat(result, " ");
  strcat(result, thrd);
  strcat(result, " ");
  strcat(result, fourth);
  if (fifth != NULL)
  {
    strcat(result, " ");
    strcat(result, fifth);
  }
  strcat(result,"\n");
}

void create_msg(char* msg, all_info sv_info, const char* type)
{
  char key[12];
  memset(key,'\0',12);
  memset(msg,'\0',50);
  if (!strcmp(type, "SUCC"))
  {
    sprintf(key,"%d",sv_info.succ_key);
    mystrcat(msg, "SUCC", key , sv_info.Next_info.IP, sv_info.Next_info.port, NULL);
  }
  else if(!strcmp(type, "NEW"))
  {
    sprintf(key,"%d",sv_info.key);
    mystrcat(msg,"NEW", key, sv_info.Myinfo.IP, sv_info.Myinfo.port, NULL);
  }
  else if(!strcmp(type, "KEY"))
  { //Message telling the key is in the successor.Second_key was changed temporarly to the key we are finding
    sprintf(msg,"KEY %d %d %s %s\n",sv_info.second_succ_key, sv_info.succ_key,
    sv_info.Next_info.IP,sv_info.Next_info.port);
  }
  else if(!strcmp(type, "EFND")){
    sprintf(msg,"EFND %d", sv_info.key);
  }
}

//Saves the data in the specific area
void parse_new(char* msg, server_info* server, int* key){
  char *aux;
  aux = (char*) malloc(sizeof(char) * 50);
  memset(aux,'\0',50);
  strcpy(aux,msg);
  strtok(aux," ");
  *key = atoi(strtok(NULL, " "));
  strcpy(server->IP, strtok(NULL," "));
  strcpy(server->port, strtok(NULL,"\n"));
  free(aux);

}

int parse_EKEY(char*msg, all_info *server){
  char *aux;
  int find_k=0;

  aux = (char*) malloc(sizeof(char) * 50);
  memset(aux,'\0',50);
  strcpy(aux,msg);
  strtok(aux," ");
  find_k=atoi(strtok(NULL," "));
  server->succ_key = atoi(strtok(NULL, " "));

  //Key is already in the ring
  if(server->succ_key == find_k)
  {
    server->succ_key=-1;
    free(aux);
    return -1;
  }
  else
  {
    strcpy(server->Next_info.IP, strtok(NULL," "));
    strcpy(server->Next_info.port, strtok(NULL,"\0"));
    free(aux);
    return 0;
  }
}

//Return values:
//0-If the successor does not have the Key_
//1-The sucessor has the key
int Key_Distance(int find_key, int my_key,int succ_key)
{
  int diff_succ=0;
  int my_diff=0;

  //eg Ringsize 16: d(9,3)=10
  if(find_key>my_key)
    my_diff=(RING_SIZE+my_key)-find_key;

  else
    my_diff=my_key-find_key;

  if(find_key>succ_key)
    diff_succ=(RING_SIZE+succ_key)-find_key;

  else
    diff_succ=succ_key-find_key;

  //The key is not in my successor
  if(diff_succ>my_diff)
    return 0;
  //My successor has the key
  else
    return 1;

}

void Show_where_is_key(char* message)
{
  char*IP;
  char*PORT;
  int find_key=0;
  int node_key=0;
  char aux[50];
  memset(aux,'\0',50);
  strcpy(aux,message);
  strtok(aux," ");
  find_key=atoi(strtok(NULL," "));
  node_key=atoi(strtok(NULL," "));
  IP = strtok(NULL," ");
  PORT = strtok(NULL,"\n");
  clrscreen();
  printf("=============================================\n");
  printf("I FOUND WHERE THE KEY %d IS KEPT\n",find_key);
  printf("=============================================\n");
  printf("Server who keeps it:\n");
  printf("IP & PORT: %s::%s\n",IP,PORT);
  printf("Key:%d\n", node_key);
  printf("=============================================\n");
  printf("(Press Enter to return to the main menu)\n");
}

void create_EKEY(char * msg, int key){
  char *aux;
  aux = (char*) malloc(sizeof(char) * 50);
  memset(aux,'\0',50);
  strtok(msg," ");
  strtok(NULL," ");
  sprintf(aux,"EKEY %d ",key);
  strcat(aux, strtok(NULL," "));
  strcat(aux," ");
  strcat(aux, strtok(NULL," "));
  strcat(aux," ");
  strcat(aux, strtok(NULL,"\n"));
  memset(msg, '\0', 50);
  strcpy(msg, aux);

  free(aux);

}



void Start_Search(char* msg,all_info _server)
{
  memset(msg,'\0',50);

  int key=0;

  char buffer[VETOR_SIZE];
  printf("Enter server key:\n");

  do{
    if(key <= 0 || key> RING_SIZE)
      printf("Key must be between the server limits(0-%d)\n",RING_SIZE);

    if(!fgets(buffer, VETOR_SIZE, stdin))
      exit(1);


  }while (sscanf(buffer,"%d", &key) != 1 ||  key <= 0 || key > RING_SIZE);

  sprintf(msg,"FND %d %d %s %s\n",key, _server.key,_server.Myinfo.IP, _server.Myinfo.port);

}
