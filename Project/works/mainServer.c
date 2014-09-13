#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>


int main (int argc, char* argv[]){
	int reuse=1;
    /*******************************************************************************************
    UDP socket declaration
    *******************************************************************************************/
 /*   //Data Socket
    int dataSocket = socket (AF_INET, SOCK_DGRAM, 0);
    if(dataSocket<0) {printf("error socket creation\n");exit(-1);}
    setsockopt(dataSocket,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
*/

    //Control Socket
    int controlSocket = socket (AF_INET, SOCK_DGRAM, 0);
    if(controlSocket<0) {printf("error socket creation\n");exit(-1);}
    setsockopt(controlSocket,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));

    /*******************************************************************************************
    Address declaration
    *******************************************************************************************/
    //Data Server
    struct sockaddr_in dataServer_addr;
    memset((char*)&dataServer_addr,0,sizeof(dataServer_addr));
    dataServer_addr.sin_family=AF_INET;

    //Data Client
    struct sockaddr_in dataClient_addr;
    memset((char*)&dataClient_addr,0,sizeof(dataClient_addr));
    dataClient_addr.sin_family=AF_INET;

    //Ctrl Server
    struct sockaddr_in controlServer_addr;
    memset((char*)&controlServer_addr,0,sizeof(controlServer_addr));
    controlServer_addr.sin_family=AF_INET;

    //Ctrl Client
    struct sockaddr_in controlClient_addr;
    memset((char*)&controlClient_addr,0,sizeof(controlClient_addr));
    controlClient_addr.sin_family=AF_INET;

    /*******************************************************************************************
    Port declaration
    *******************************************************************************************/
    int controlPort=atoi(argv[1]);
    controlServer_addr.sin_port=htons(controlPort);
    controlServer_addr.sin_addr.s_addr=INADDR_ANY;

    int dataPort=controlPort+1;
    dataServer_addr.sin_port=htons(dataPort);
    dataServer_addr.sin_addr.s_addr=INADDR_ANY;

    /*******************************************************************************************
    Bind
    *******************************************************************************************/

    socklen_t clientSize = sizeof(dataClient_addr);
    socklen_t serverSize = sizeof(dataServer_addr);

    //int dataBind = bind(dataSocket,(struct sockaddr *)&dataServer_addr,serverSize);
    int controlBind = bind(controlSocket,(struct sockaddr *)&controlServer_addr,serverSize);

    /*******************************************************************************************
    Buffers & Clocks
    *******************************************************************************************/
    char buffer[11];
    char portbuff[5];

	/*******************************************************************************************
	TEST!!!!
	 *******************************************************************************************/

	int portNumber, nbPorts=1;
    int rcv,snd,i,k,j,p,namesize=55;
    char serverAndArgs[19];
    serverAndArgs[0]='.';serverAndArgs[1]='/';serverAndArgs[2]='s';serverAndArgs[3]='e';serverAndArgs[4]='r';serverAndArgs[5]='v';
    serverAndArgs[6]='e';serverAndArgs[7]='r';serverAndArgs[8]='T';serverAndArgs[9]='e';serverAndArgs[10]='2';serverAndArgs[11]='b';
    serverAndArgs[12]=' ';
    
    while(1){
		/********************************
		  expect SYN
		 ********************************/
		printf("mainServer: reception start\n");
		rcv=recvfrom(controlSocket,buffer,sizeof(buffer),0,(struct sockaddr *)&controlClient_addr,&clientSize);
		printf("mainServer: message received: \n");
		for (i=0;i<rcv;i++){
			printf("%c",buffer[i]);
		}
		printf("\n");
		
			if(buffer[0]=='S'){
			/********************************
			  launch server2 and send SYN-ACK<dataPort>
			 ********************************/
			portNumber=controlPort+1+nbPorts;
			
			sprintf(portbuff,"%d",portNumber);
			
			for(i=13;i<19;i++){
					serverAndArgs[i]=portbuff[i-13];
			}
			
			printf("serverAndArgs = ");
			for(i=0;i<19;i++){
					printf("%c",serverAndArgs[i]);
			}
			printf("\n");
			
			
			if(fork()==0){
				
				if(system(serverAndArgs)==-1){
					printf("error opening new server process with portNumber=%d \n",portNumber);
					exit(-1);
				}
			}
			
			sprintf(portbuff,"%d",portNumber);
			sprintf(buffer,"%s","SYN-ACK");
			for(i=7;i<11;i++) buffer[i]=portbuff[i-7];
			printf("sending SYN-ACK<dataport> = ");
			for(i=0;i<11;i++) printf("%c",buffer[i]);
			printf("\n");
				
			if((snd=sendto(controlSocket,buffer,sizeof(buffer),0,(struct sockaddr*)&controlClient_addr,clientSize))==-1) {
				printf("send error\n");
				exit(-1);
			}
			
			/********************************
			  receive ACKACK<portnumber>
			 ********************************/
			rcv=recvfrom(controlSocket,buffer,sizeof(buffer),0,(struct sockaddr *)&controlClient_addr,&clientSize);
			printf("message received: \n");
			for (i=0;i<11;i++){
				printf("%c",buffer[i]);
			}
			printf("\n");
			nbPorts++;
		}
	}
	
		
		
}

