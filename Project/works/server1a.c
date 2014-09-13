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

int main (int argc, char* argv[]){
    /*******************************************************************************************
    UDP socket declaration
    *******************************************************************************************/
    //Data Socket
    int dataSocket = socket (AF_INET, SOCK_DGRAM, 0);
    if(dataSocket<0) {printf("error socket creation\n");exit(-1);}
    int reuse=1;
    setsockopt(dataSocket,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));


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

    int dataBind = bind(dataSocket,(struct sockaddr *)&dataServer_addr,serverSize);
    int controlBind = bind(controlSocket,(struct sockaddr *)&controlServer_addr,serverSize);

    /*******************************************************************************************
    Buffers
    *******************************************************************************************/
    char filebuff[20];
    char portbuff[5];
    char buffer[11];
    char ackbuff[9];
	char imagebuff[506];
	char databuff[500];
	char seqbuff[7];



	/*******************************************************************************************
	Test
	*******************************************************************************************/

	int rcv,snd,i,namesize=55;
    /********************************
    expect SYN
    ********************************/
        printf("reception start\n");
        rcv=recvfrom(controlSocket,buffer,sizeof(buffer),0,(struct sockaddr *)&controlClient_addr,&clientSize);
        printf("message received: \n");
        for (i=0;i<rcv;i++){
            printf("%c",buffer[i]);
        }
        printf("\n");
	/********************************
	send SYN-ACK<dataPort>
	********************************/
        sprintf(portbuff,"%d",dataPort);
        sprintf(buffer,"%s","SYN-ACK");
        for(i=7;i<11;i++) buffer[i]=portbuff[i-7];
        printf("sending SYN-ACK<portnumber>\n");
        if((snd=sendto(controlSocket,buffer,sizeof(buffer),0,(struct sockaddr*)&controlClient_addr,clientSize))==-1) {
            printf("send error\n");
            exit(-1);
        }
	/********************************
	expect ACKACK<controlPort>
	********************************/
        rcv=recvfrom(controlSocket,buffer,sizeof(buffer),0,(struct sockaddr *)&controlClient_addr,&clientSize);
        printf("message received: \n");
        for (i=0;i<11;i++){
            printf("%c",buffer[i]);
        }
        printf("\n");
	/********************************
	expect filename
	********************************/
        rcv=recvfrom(dataSocket,filebuff,sizeof(filebuff),0,(struct sockaddr *)&dataClient_addr,&clientSize);
        printf("message received: \n");
        for (i=0;i<rcv;i++){
            if(filebuff[i]=='.'){
                namesize=i+4;
            }
            if(i<namesize){
                printf("%c",filebuff[i]);
            } else {
                printf("%c",' ');
            }
        }
        printf("\n");
        //filename creation
        char filename[namesize+1];
        for (i=0;i<namesize;i++) {
			filename[i]=filebuff[i];
		}
		for (i=0;i<namesize;i++) {
			printf("%c",filename[i]);
		}
		filename[namesize]='\0';//make it a string pls
       printf("\n");
	/********************************
	open,read,det. the length of the requested file
	********************************/
		int filesize,steps,sentsize,remaining,j,k;
		FILE *image=fopen(filename,"r");
		fseek(image,0,SEEK_END);
		filesize=ftell(image);
		fseek(image,0,SEEK_SET);
		steps=filesize/500 + 1;
		remaining=filesize%500;
		int im=1;
		int p_number=1;
		int p_number_transmited=0;
		int window=1;
	/********************************
	 * TRANSMISSION STARTS
	 * *****************************/

		i=1;
		while(i<=steps){
            if(i+window<steps){
            while((p_number<=steps)&&(p_number<=i+window)){/// fast retransmit
                if (p_number == steps) {
                    im=fread(databuff,remaining,1,image);
                    sentsize= remaining+6;

                } else {
                    im=fread(databuff,500,1,image);
                    sentsize= 506;
                }
        /*******************************
         * sequence buffer generation
         * ****************************/
                //generate seqbuff
                sprintf(seqbuff,"%d",p_number);
                if(p_number<10){
                    k=1;
                }else if(p_number<100){
                    k=2;
                }else if(p_number<1000){
                    k=3;
                }else if(p_number<10000){
                    k=4;
                }else if(p_number<100000){
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
        /*****************************
         * merge data and seq into imagebuff
         * *************************/
                for(j=0;j<6;j++){
                    imagebuff[j]=seqbuff[j];
                }
                for(j=0;j<500;j++){
                    imagebuff[j+6]=databuff[j];
                }
        /******************************
         * send stuff
         * **************************/
                if((snd=sendto(dataSocket,imagebuff,sentsize,0,(struct sockaddr *)&dataClient_addr,clientSize))==-1) {
                    printf("send error\n");
                    exit(-1);
                }
                p_number++;
            }
            p_number_transmited=p_number;
            for (p_number=i;p_number<=p_number_transmited-1;p_number++){///fast retransmit, receive the exact same amount that was transmited
                /******************************
                 * rcv stuff
                 * **************************/
                rcv=recvfrom(controlSocket,ackbuff,9,0,(struct sockaddr *)&controlClient_addr,&clientSize);
                printf("message received: %s \n",ackbuff);
            }
            i=p_number;
            /** change window size according to specified rules**/
            window=window*2;
            }
		}
	snd=sendto(dataSocket,"FIN",sizeof("FIN"),0,(struct sockaddr *)&dataClient_addr,clientSize);

return 0;
}


/**
 * client 1a 			serv
 * 		__SYN__>(service)
 * 	  <__SYN-ACK1234__(service)
 * 		__ACK__>(service)
 * 		__nom-fich__>(sur 1234)
 * 		< "00123"DATA (sur 1234)
 * 		"ACK00123" >(service) (client1a)
 * 		"ACK00123" >(sur 1234) (client1b)
 * 		<__FIN__(sur 1234)
 **/
