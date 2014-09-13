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
    int dataPort=atoi(argv[1]);
    //controlServer_addr.sin_port=htons(controlPort);
    //controlServer_addr.sin_addr.s_addr=INADDR_ANY;

    //int dataPort=controlPort;
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
    char filebuff[20];
    char portbuff[5];
    char buffer[11];
    char ackbuff[9];
    char oldackbuff[9]={'A','C','K','0','0','0','0','0','0'};
	char imagebuff[1030];
	char databuff[1024];
	char seqbuff[7];

	///clocks
	struct timespec requestStart[64], requestEnd[64],srtt,rtt,pseudosrtt;
	pseudosrtt.tv_nsec=1000*1000*15;
	pseudosrtt.tv_sec=0;
	srtt.tv_nsec=1000*1000*10;///10msec default
	srtt.tv_sec=0;
	double start[64], end[64];
	int ackcounter=1,intack,trans;
    int pselectControl;

    ///pour select
    fd_set fds;
	/*******************************************************************************************
	TEST!!!!
	 *******************************************************************************************/

    int rcv,snd,i,namesize=55, ackdropped=0;
    /********************************
      expect SYN
     ********************************/
    /*printf("reception start\n");
    rcv=recvfrom(controlSocket,buffer,sizeof(buffer),0,(struct sockaddr *)&controlClient_addr,&clientSize);
    printf("message received: \n");
    for (i=0;i<rcv;i++){
	    printf("%c",buffer[i]);
    }
    printf("\n");
    */
    /********************************
      send SYN-ACK<dataPort>
     ********************************/
    /*sprintf(portbuff,"%d",dataPort);
    sprintf(buffer,"%s","SYN-ACK");
    for(i=7;i<11;i++) buffer[i]=portbuff[i-7];
    printf("sending SYN-ACK<portnumber>\n");
    if((snd=sendto(controlSocket,buffer,sizeof(buffer),0,(struct sockaddr*)&controlClient_addr,clientSize))==-1) {
	    printf("send error\n");
	    exit(-1);
    }
    */
    /********************************
      expect ACKACK<controlPort>
     ********************************/
    /*rcv=recvfrom(controlSocket,buffer,sizeof(buffer),0,(struct sockaddr *)&controlClient_addr,&clientSize);
    printf("message received: \n");
    for (i=0;i<11;i++){
	    printf("%c",buffer[i]);
    }
    printf("\n");
    */
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
    int filesize,steps,sentsize,remaining,j,k,p;
    FILE *image=fopen(filename,"r");
    fseek(image,0,SEEK_END);
    filesize=ftell(image);
    fseek(image,0,SEEK_SET);
    steps=filesize/1024 + 1;
    remaining=filesize%1024;
    int im=1;
    /********************************
     * TRANSMISSION STARTS
     * *****************************/
    i=1;
    while(i<=steps){

	    if (i == steps) {
		    im=fread(databuff,remaining,1,image);
		    sentsize= remaining+6;

	    } else {
		    im=fread(databuff,1024,1,image);
		    sentsize= 1030;
	    }
	    /*******************************
	     * sequence buffer generation
	     * ****************************/
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
	    /*****************************
	     * merge data and seq into imagebuff
	     * *************************/
	    for(j=0;j<6;j++){
		    imagebuff[j]=seqbuff[j];
	    }
	    for(j=0;j<1024;j++){
		    imagebuff[j+6]=databuff[j];

	    }
	    /******************************
	     * send stuff
	     * **************************/
	    if((snd=sendto(dataSocket,imagebuff,sentsize,0,(struct sockaddr *)&dataClient_addr,clientSize))==-1) {
		    printf("send error\n");

		    exit(-1);
	    }
	    ///start clock
	    if(clock_gettime(CLOCK_REALTIME,&requestStart[0]) == -1) {
		    printf("time start error\n");
	    }
	    /******************************
	     * rcv stuff
	     * **************************/
	    FD_ZERO (&fds);
	    FD_SET (dataSocket,&fds);
	
	    pselectControl=pselect(
			    dataSocket+1,
			    &fds,
			    NULL,
			    NULL,
			    &pseudosrtt,
			    NULL);		

	    if(pselectControl==0) {
		    i--;
	    }else {
		    rcv=recvfrom(dataSocket,ackbuff,9,0,(struct sockaddr *)&controlClient_addr,&clientSize);
	    }
	    ///just to make sure things are nice and tidy
	    printf("\n step %d \n",i);
	    ///stop clock
	    if( clock_gettime(CLOCK_REALTIME,&requestEnd[0]) == -1) {
		    printf("time stop error\n");
	    }

	    if(oldackbuff == ackbuff) {
		    ackdropped++;
	    }
	    ///calc rtt and srtt
	    rtt.tv_nsec=requestEnd[0].tv_nsec - requestStart[0].tv_nsec;
	    rtt.tv_sec=requestEnd[0].tv_sec - requestStart[0].tv_sec;
	    srtt.tv_sec=0.5*srtt.tv_sec+0.5*rtt.tv_sec;
	    srtt.tv_nsec=0.5*srtt.tv_nsec+0.5*rtt.tv_nsec;
	    pseudosrtt.tv_nsec=3*srtt.tv_nsec;
	    pseudosrtt.tv_sec=3*srtt.tv_sec;
	    printf("message received: %s RTT= %d µs\n",ackbuff,(int)rtt.tv_nsec/1000);

	    for(j=0;j<9;j++){
		    oldackbuff[j]=ackbuff[j];
	    }
	    i++;
	    ackcounter++;

	    if(ackdropped>=4){
		i=i-3;
	    }
    }
    snd=sendto(dataSocket,"FIN",sizeof("FIN"),0,(struct sockaddr *)&dataClient_addr,clientSize);

    return 0;
}


/**
 * client 1a 			serv
 * 		__SYN__>(service)
 * 			*launch new server 1234
 * 	  <__SYN-ACK1234__(service) with old program
 * 		__ACK__>(service)
 * 		__nom-fich__>(sur 1234)
 * 		< "00123"DATA (sur 1234)
 * 		"ACK00123" >(service) (client1a)
 * 		"ACK00123" >(sur 1234) (client1b)
 * 		<__FIN__(sur 1234)
 **/
