#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>

int main(int argc,char* argv[]){
	
	int port2=atoi(argv[1]);
	int masocket2=socket(AF_INET,SOCK_DGRAM,0);
	int masocket=socket(AF_INET,SOCK_DGRAM,0);
	if (masocket2<0){
		exit(-1);
	}
	int reuse=1;	
//socket UDP declaration
	printf("descripteur socket UDP serveur %d\n",masocket2);
	
	//control socket
	setsockopt(masocket2,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
	struct sockaddr_in my_addr2;
	memset((char*)&my_addr2,0,sizeof(my_addr2));
	struct sockaddr_in client_addr2;
	memset((char*)&client_addr2,0,sizeof(client_addr2));
	
	my_addr2.sin_family=AF_INET;
	my_addr2.sin_port=htons(port2);
	my_addr2.sin_addr.s_addr=INADDR_ANY;
	printf("socket UDP ok\n");
	
	//data socket
	setsockopt(masocket,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
	struct sockaddr_in my_addr;
	memset((char*)&my_addr,0,sizeof(my_addr2));
	struct sockaddr_in client_addr;
	memset((char*)&client_addr,0,sizeof(client_addr));
	
	int port=port2+1;
	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(port);
	my_addr.sin_addr.s_addr=INADDR_ANY;
	printf("socket UDP ok\n");
	
	//buffers
	
	char buffer[10];
	char imagebuff[1030];
	char databuff[1024];
	char seqbuff[6]={'0','0','0','0','0','0'};
	char ackbuff[10];
	ackbuff[0]='A';
	ackbuff[1]='C';
	ackbuff[2]='K';
	ackbuff[3]='_';
	socklen_t taille_client2 = sizeof(client_addr2);
	socklen_t taille_addr2 = sizeof(my_addr2);
//bind	
	int b2=bind(masocket2,(struct sockaddr *)&my_addr2,taille_addr2);
	int b1=bind(masocket,(struct sockaddr *)&my_addr,taille_addr2);
	printf("bind gets: %d\n",b1);
//synchro
	int rcv,snd,i;
	rcv=recvfrom(masocket2,buffer,10,0,(struct sockaddr *)&client_addr2,&taille_client2);
	if(buffer[0]=='S'&&buffer[1]=='Y'&&buffer[2]=='N'){
		snd=sendto(masocket2,"SYN-ACK",sizeof("SYN-ACK"),0,(struct sockaddr *)&client_addr2,taille_client2);
		printf("syn-ack sent \n");
	} else {
		printf("no SYN err\n");
	}
	rcv=recvfrom(masocket2,buffer,10,0,(struct sockaddr *)&client_addr2,&taille_client2);
	if(buffer[0]=='A'&&buffer[1]=='C'&&buffer[2]=='K') {
		
		//send new port
		sprintf(buffer, "%d", port);
		if((snd=sendto(masocket2,buffer,10,0,(struct sockaddr *)&client_addr2,taille_client2))==-1) {
			printf("new port failed to send\n");
			exit(-1);
		}
		
		
		FILE *image=fopen("empty.jpg","w");
		int im, img;
		int j;
		
		//infinite loop
		for(;;){
			//receive data
			if((rcv=recvfrom(masocket,imagebuff,sizeof(imagebuff),0,(struct sockaddr *)&client_addr,&taille_client2))==-1){
				printf("recv err\n");
				exit(-1);
			}
			//separate data, seq 
			for(j=0;j<6;j++){
				seqbuff[j]=imagebuff[j];
				//printf("%c",seqbuff[j]);
				ackbuff[j+4]=seqbuff[j];
			}
			for(j=0;j<1024;j++){
				databuff[j]=imagebuff[j+6];
			}
			
			//write data on file
			im=fwrite(databuff,rcv-6,1,image);
			//check pointer position and stuff
			printf("pointer pos: %d, rcv: %d, im: %d\n",ftell(image),rcv,im);
			//printf("message received: %s \n",buffer);
			
			
			//send ack
			snd=sendto(masocket2,ackbuff,sizeof(ackbuff),0,(struct sockaddr *)&client_addr2,taille_client2);
			printf("message sent: %s \n",ackbuff);
			if (rcv < 1030) {
				break;
			}
		}
		fclose(image);
	}
}
