
#include "Network.h"
#include "interface_n_aux.h"


int init_UDPsv (all_info* _server)
{
  int sockfd,n,errcode;
  struct addrinfo hints, *res;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0); //UDP SOCKET
  if (sockfd==-1)
    exit(1); //error

  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET; //IPV4
  hints.ai_socktype=SOCK_DGRAM;//TCP SOCKET
  hints.ai_flags=AI_PASSIVE;


  errcode = getaddrinfo(NULL, _server->Myinfo.port, &hints, &res);

  if((errcode)!=0)
    exit(1); //error

  //binded
  n=bind(sockfd, res->ai_addr, res->ai_addrlen);
  if (n==-1)
    exit(1);

  freeaddrinfo(res);

  return sockfd;
}

int init_TCP_listen (char PORT[])
{
  int errcode, newfd;
  int n;
  struct addrinfo  hints;
  struct addrinfo * res;


  newfd=socket(AF_INET,SOCK_STREAM,0); //TCP socket

  if (newfd==-1)
    exit(1); //error

  memset(&hints, 0, sizeof hints);

  hints.ai_family=AF_INET; //IPv4
  hints.ai_socktype=SOCK_STREAM; //TCP socket
  hints.ai_flags=AI_PASSIVE;

  errcode = getaddrinfo(NULL,PORT,&hints,&res);

  if((errcode)!=0)
    exit(1); //error

  //Binded
  n=bind(newfd,res->ai_addr,res->ai_addrlen);

  if(n==-1)
    exit(1);//error


  if(listen(newfd,6)==-1)
    exit(1); //error

  freeaddrinfo(res);

  return newfd;

}

//This function initializes a TCP socket and prepare a connection request
//It returns a File descriptor which indicates the established connection
int TCP_InitnConect( char IP[], char PORT[])
{
  int fd,errcode;
  ssize_t n;
  struct addrinfo hints,*res;



  fd=socket(AF_INET,SOCK_STREAM,0);

  if (fd==-1)
    exit(1); //error

  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//TCP socket
  hints.ai_socktype=SOCK_STREAM;//IPv4
  hints.ai_protocol=IPPROTO_TCP;//TCP socket

  errcode= getaddrinfo (IP,PORT,&hints,&res);

  if(errcode!=0)
    exit(1);//error


  n= connect (fd,res->ai_addr,res->ai_addrlen);

  if(n==-1)
    exit(1);//error

  freeaddrinfo(res);

  return fd;

}

/*type indicates if the server wars created by the sentry command or
the new i command.
type -1 = server with a single node
type 0 = normal server
type 1 = server entry

*/
all_info* Server_Heart(all_info* myserver, int type, ringfd mainfds)
{
  //Max nb in select, receiving filedescriptor and temporary fd
  int maxfd = 0,newfd = 0, auxfd=0;
  int n;
  int option = -1;
  //Time lapse variable
//  struct timeval timeout;


  struct sockaddr_in addr;
  socklen_t addrlen;

  fd_set sock_set;
  char buffer[VETOR_SIZE];
  char buffer2[VETOR_SIZE];
  char buff[10];

  if(type==1)
    mainfds = Sentry_Startup(&myserver);

  mainfds.udp = init_UDPsv(myserver);

  if(type != 1)
    mainfds.listen = init_TCP_listen(myserver->Myinfo.port);


  //Main cicle- The server heart
  do
  {
    Display_new_menu();
    //needs to be reset every iterationallfds.listen = init_TCP_listen((*myserver)->Myinfo.port);
    FD_ZERO(&sock_set);

    //Add stdin Inputs
    FD_SET(STDIN_FILENO, &sock_set);
    maxfd = max(maxfd, STDIN_FILENO);

    //Add udp
    FD_SET(mainfds.udp, &sock_set);
  	maxfd = max(maxfd, mainfds.udp);

    FD_SET(mainfds.listen, &sock_set);
    maxfd = max(maxfd, mainfds.listen);

    //If it is the first server node.  Can't set until we have a connection
    if(type != -1 )
    {
      if(type == 0)
      {
        FD_SET(mainfds.pre, &sock_set);
        maxfd = max(maxfd, mainfds.pre);
      }

      FD_SET(mainfds.next, &sock_set);
      maxfd = max(maxfd, mainfds.next);
    }

    //Central Master controller
  	select(maxfd+1, &sock_set, (fd_set*) NULL, (fd_set*) NULL, (struct timeval*) NULL);

    //If the keyboard is pressed
    if(FD_ISSET(STDIN_FILENO, &sock_set))
    {
       scanf("%d",&option);//Por aqui um fgets e sscanf em vez do scanf
      switch (option)
      {
        case 1: //Temporary auxiliar func
          printf("OPCAO 1-Teste-write succ\n");
          write(mainfds.next,"OLA next\n",8);
    		break;

    		case 2:
          //Print the server state variables
          ServerState(myserver);
    		break;

    		case 3: //Temporary auxiliar func
          printf("OPCAO 3-Teste-write pred\n");
          write(mainfds.pre,"OLA pre\n",8);
    		break;

        default:
          system("clear");
      }
    }

    //UDP talking
    if(type != 1)
    {
      if (FD_ISSET(mainfds.udp, &sock_set))
		    {
          recvfrom(mainfds.udp, buff, 7, 0, NULL, NULL);
			    printf("%s\n", buff);
		    }
    }

    //Unknown talking
    if (FD_ISSET(mainfds.listen, &sock_set))
		{
      //Accept the new connection
      if((newfd = accept(mainfds.listen,(struct sockaddr*)&addr,&addrlen)) == -1)
        exit(1);

      if((n = read (newfd,buffer,VETOR_SIZE))!=0)
      {

        if(n==-1)
        {
          printf("Read error\n");
          exit(1);//error
        }

        //Se for um pedido de entrada no anel
        if(strstr(buffer,"NEW")!=NULL)
        {
          //First connection of the new ring
          if(type ==-1)
          {
            mainfds.pre=newfd;
            myserver = Message_Analysis (buffer,myserver,-1);

            //Excreve 2 sucessor no predecessor
            n=sprintf(buffer,"SUCC %d %d.%s %d.%s\n", myserver->succ_key, myserver->succ_key,
            myserver->Prev_info.IP,myserver->succ_key,myserver->Prev_info.port);
            printf("buffer:%s",buffer);

            write(mainfds.pre, buffer, n);
            if(n==-1)
            {
              printf("Read error\n");
              exit(1);//error
            }

            mainfds.next = TCP_InitnConect(myserver->Next_info.IP, myserver->Next_info.port);

            n=write(mainfds.next,"SUCCCONF\n", 9);
            if(n==-1)
            {
              printf("write error\n");
              exit(1);//error
            }

            n=write(mainfds.next,"FIRST\n", 6);
            if(n==-1)
            {
              printf("write error\n");
              exit(1);//error
            }

            type=0;
            //close(mainfds.pre);
            printf("FIZ TUDO\n");
          }
          else
          {
            //auxfd passa a ser o antigo predecessor
            auxfd = mainfds.pre;
            //novo predecessor e atualizado
            mainfds.pre = newfd;

            myserver = Message_Analysis (buffer,myserver,0);

            //Avisa o antigo predecessor do novo Sucessor
            n=write(auxfd, buffer, n);
            if(n==-1)
            {
              printf("write error\n");
              exit(1);//error
            }
            close(auxfd);

            //Diz qual o 2º sucessor do predecessor
            n=sprintf(buffer,"SUCC %d %d.%s %d.%s\n", myserver->succ_key, myserver->succ_key,
            myserver->Next_info.IP,myserver->succ_key,myserver->Next_info.port);

            n=write(mainfds.pre, buffer, n);
            if(n==-1)
            {
              printf("write error\n");
              exit(1);//error
            }
          }
        }
        if(strstr(buffer,"SUCCCONF\n")!=NULL)
        {
          if(type==1)
          {
            if((n = read (newfd,buffer2,VETOR_SIZE))!=0)
            {

              if(n==-1)
              exit(1);//error
              printf("JA estou ca\n");

              //We are the second node so we have all we need already
              if(strstr(buffer2,"FIRST\n")!=NULL)
              {
                strcpy(myserver->SecondNext_info.IP,myserver->Myinfo.IP);
                strcpy(myserver->SecondNext_info.port,myserver->Myinfo.port);
                strcpy(myserver->Prev_info.IP,myserver->Next_info.IP);
                strcpy(myserver->Prev_info.port,myserver->Next_info.port);
                mainfds.pre=newfd;
                type=0;
              }
              //We are at least the 3rd node, lets save all the info.
              if(strstr(buffer2,"SUCC")!=NULL)
              {
                myserver = Message_Analysis (buffer2,myserver,0);
                mainfds.pre=newfd;
                type=0;
              }
            }
            else
            {
              close(newfd);
                exit(1);
            }
          }
        }
      }
      //If what it reads is incorret, maybe that's not our conexion
    /*  else
        close(newfd);*/
		}
    //Ring with at least 2 nodes
    if(type == 0 )
    {
      //Predecessor is talking
      if (FD_ISSET(mainfds.pre, &sock_set))
      {
        if( (n = read (mainfds.pre,buffer, VETOR_SIZE)) != 0)
        {
          if(n==-1)
            exit(1);

          if(strstr(buffer,"OLA")!=NULL)
            write(1,buffer,n);
        }
      }
    }
    //The ring node has a successor
    if(type != -1)
    {
      //Successor is talking
      if (FD_ISSET(mainfds.next, &sock_set))
	    {
        //Recebe algo do sucessor
        if((n = read (mainfds.next,buffer, VETOR_SIZE)) != 0)
        {
          if(n==-1)
            exit(1);

          //eg: SUCC 12 12.IP 12.TCP - 2º Sucessor
          if(strstr(buffer,"SUCC")!=NULL)
            myserver=Message_Analysis(buffer, myserver,2);

          //Significa que vai ter um novo sucessor
          if(strstr(buffer,"NEW")!=NULL)
          {
            //Encerra sessão com antigo sucessor
            close(mainfds.next);

            //The successor becomes the Second Successor
            strcpy(myserver->SecondNext_info.IP,myserver->Next_info.IP);
            strcpy(myserver->SecondNext_info.port,myserver->Next_info.port);
            myserver=Message_Analysis(buffer, myserver,1);

            //Nova sessão com novo sucessor
            mainfds.next = TCP_InitnConect(myserver->Next_info.IP, myserver->Next_info.port);

            n=write(mainfds.next,"SUCCCONF\n", 9);

            if(n==-1)
            {
              printf("write error\n");
              exit(1);//error
            }
            //Da ao novo sucessor as informações do predecessor (as suas)
            n=sprintf(buffer,"SUCC %d %d.%s %d.%s\n", myserver->key, myserver->key,
            myserver->Myinfo.IP,myserver->key,myserver->Myinfo.port);

            n=write(mainfds.next,buffer, n);

            if(n==-1)
            {
              printf("write error\n");
              exit(1);//error
            }
            //Diz ao predecessor que o segundo sucessor deste mudou
            n=sprintf(buffer,"SUCC %d %d.%s %d.%s\n", myserver->succ_key, myserver->succ_key,
            myserver->Next_info.IP,myserver->succ_key,myserver->Next_info.port);

            n=write(mainfds.pre,buffer,n);

            if(n==-1)
            {
            printf("write error\n");
            exit(1);//error
            }
          }
          //Auxiliar function TEMPORARY
          if(strstr(buffer,"OLA")!=NULL)
            write(1,buffer,n);
        }
		  }
    }

  }while(myserver->inRing != false);

  return myserver;
}

ringfd Sentry_Startup(all_info** myserver)
{
  char buffer[VETOR_SIZE];

  ringfd allfds;
  allfds.pre=0;
  allfds.listen=0;
  allfds.next=0;
  allfds.udp=0;

  int n;
  int stop=-1;

  allfds.listen = init_TCP_listen((*myserver)->Myinfo.port);
  //Connect to the successor
  allfds.next = TCP_InitnConect( (*myserver)->Next_info.IP, (*myserver)->Next_info.port);

  //write ex: NEW 8 8.IP 8.TCP with his data
  n=sprintf(buffer,"NEW %d %d.%s %d.%s\n", (*myserver)->key, (*myserver)->key,
  (*myserver)->Myinfo.IP,(*myserver)->key,(*myserver)->Myinfo.port);



  write(allfds.next,buffer,n);
  if(n==-1)
  {
    printf("write error\n");
    exit(1);//error
  }
  //First Part set!!!!
  (*myserver)->inRing=true;

  return allfds;
}