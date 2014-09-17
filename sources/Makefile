all:
	$(MAKE) server1 
	$(MAKE) server2 
	$(MAKE) serverTe2b 
	$(MAKE) mainServer 
	echo "WARNING server 1 and 2 use a-type clients, server3 (mainServer) uses b-type"

server1: server1.c
	gcc server1.c -o ../bin/server1
server2: server2.c
	gcc server2.c -o ../bin/server2
serverTe2b: serverTe2b.c
	gcc serverTe2b.c -o ../bin/serverTe2b
mainServer: mainServer.c
	gcc mainServer.c -o ../bin/mainServer
