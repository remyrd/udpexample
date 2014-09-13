#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

/*void seqmod(int i, char seq[6]) {
	int j=0;
	
	for(j=5;j>=0;j--){
		seqbuff[j]=(char)i%10;
		i=i/10;
	}
}*/

void main(int argc, char* argv[]) {

	int port2=atoi(argv[2]);
	int port;
//UDP socket
	int masocket=socket(AF_INET,SOCK_DGRAM,0);
	int masocket2=socket(AF_INET,SOCK_DGRAM,0);
	if (masocket<0){
		exit(-1);
	}
	int reuse=1;
	setsockopt(masocket2,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
	setsockopt(masocket,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
	//local address
	struct sockaddr_in my_addr;
	memset((char*)&my_addr,0,sizeof(my_addr));
	
	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(port);
	my_addr.sin_addr.s_addr=INADDR_ANY;
	
	//inet_aton(argv[1], &my_addr.sin_addr);
	//server address
	struct sockaddr_in server_addr2;
	memset((char*)&server_addr2,0,sizeof(server_addr2));
	
	server_addr2.sin_family=AF_INET;
	server_addr2.sin_port=htons(port2);
	inet_aton(argv[1], &server_addr2.sin_addr);
	
	//data socket
	struct sockaddr_in server_addr;
	memset((char*)&server_addr,0,sizeof(server_addr));
	
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(port);
	inet_aton(argv[1], &server_addr.sin_addr);

	socklen_t taille_socket=sizeof(server_addr);
	
	//buffers
	char buffer[10];
	char imagebuff[1030];
	char databuff[1024];
	char seqbuff[7]={'0','0','0','0','0','0','0'};
	int snd,rcv,k;
	//clocks
	struct timespec requestStart, requestEnd;
	double start, end, rtt, srtt=1;
	int ackcounter=1,intack,trans;
	
	
	//SYN and ACK
	snd=sendto(masocket2,"SYN",sizeof("SYN"),0,(struct sockaddr *)&server_addr2,taille_socket);
	printf("message sent: SYN\n");
	rcv=recvfrom(masocket2,buffer,10,0,(struct sockaddr *)&server_addr2,&taille_socket);
	printf("message rcv: %s\n",buffer);
	if(buffer[0]=='S'&&buffer[1]=='Y'&&buffer[2]=='N'&&buffer[3]=='-'&&buffer[4]=='A'&&buffer[5]=='C'&&buffer[6]=='K'){
		snd=sendto(masocket2,"ACK",sizeof("ACK"),0,(struct sockaddr *)&server_addr2,taille_socket);
		printf("message sent: ACK\n Connection Can Begiiin \n\n");
		//receive new port
		rcv=recvfrom(masocket2,buffer,10,0,(struct sockaddr *)&server_addr2,&taille_socket);
		//change data port
		port=atoi(buffer);
		printf("new data port number= %d\n",port);
		server_addr.sin_port=htons(port);
		//loop
		
		FILE *image=fopen("halo.JPG","r");
		int im,sz,i=0,j,k=0;
		fseek(image,0,SEEK_END);
		sz=ftell(image);
		printf("file size= %d\n",sz);
		fseek(image,0,SEEK_SET);
		int steps=(sz/1024)+1;
		int remaining= sz%1024;
		int sentsize;
		im=1;
		for(;;){
			//scanf("%s",buffer);
			//send data
			//printf("position pointeur %d\n",ftell(image));
			while(i<steps){
				for(trans=0;trans<ackcounter;trans++){
					if (i == steps-1) {
						im=fread(databuff,remaining,1,image);
						sentsize= remaining+6;
						
					} else {
						im=fread(databuff,1024,1,image);
						sentsize= 1030;
					}
					
					//generate seqbuff
					sprintf(seqbuff,"%d",i);
					if(i<10){
						k=1;
					}else if(i<100){
						k=2;
					}else if(i<1000){
						k=3;
					}else if(i<10000){
						k=4;
					}else if(i<100000){
						k=5;
					}
					
					switch(k){
						case 1: 
							seqbuff[5]=seqbuff[0];
							for(j=0;j<5;j++) seqbuff[j]='0';
							break;
						case 2: 	
							seqbuff[5]=seqbuff[1];
							seqbuff[4]=seqbuff[0];
							for(j=0;j<4;j++) seqbuff[j]='0';
							break;
						case 3:
							seqbuff[5]=seqbuff[2];
							seqbuff[4]=seqbuff[1];
							seqbuff[3]=seqbuff[0];
							for(j=0;j<3;j++) seqbuff[j]='0';
							break;
						case 4:
							seqbuff[5]=seqbuff[3];
							seqbuff[4]=seqbuff[2];
							seqbuff[3]=seqbuff[1];
							seqbuff[2]=seqbuff[0];
							for(j=0;j<2;j++) seqbuff[j]='0';
							break;
						case 5:
							seqbuff[5]=seqbuff[4];
							seqbuff[4]=seqbuff[3];
							seqbuff[3]=seqbuff[2];
							seqbuff[2]=seqbuff[1];
							seqbuff[1]=seqbuff[0];
							seqbuff[0]='0';
							break;
					}
					printf("seqbuff: ");
					for(k=0;k<6;k++) printf("%c",seqbuff[k]);
					printf("\n");
					
					//merge data and sequence (magic)
					for(j=0;j<6;j++){
						imagebuff[j]=seqbuff[j];
					}
					for(j=0;j<1024;j++){
						imagebuff[j+6]=databuff[j];
					}
					
					
					
					if((snd=sendto(masocket,imagebuff,sentsize,0,(struct sockaddr *)&server_addr,taille_socket))==-1) {
						printf("send error\n");
						exit(-1);
					}
					i++;
				}
				//start clock
				if(clock_gettime(CLOCK_REALTIME,&requestStart) == -1) printf("time error\n");
				
				//printf("frame %d sent \n pointer at position %d\n",i,ftell(image));
				
				//printf("message sent: %s\n",buffer);
				//receive ack
				intack=ackcounter;
				for(trans=0;trans<intack;trans++){
					
					rcv=recvfrom(masocket2,buffer,10,0,(struct sockaddr *)&server_addr2,&taille_socket);
					printf("message received: %s \n RTT=%f \n",buffer,rtt);
					ackcounter++;
					
				}
				//stop clock
				if( clock_gettime(CLOCK_REALTIME,&requestEnd) == -1) printf("time error\n");
				rtt=(requestEnd.tv_sec - requestStart.tv_sec)*1000000+(requestEnd.tv_nsec - requestStart.tv_nsec)/1000;
				srtt=0.5*srtt+0.5*rtt;
					
				if(snd<1030){
					break;
				}
			}
		}
	} else {
		printf("error: no SYN-ACK\n");
	}
}
