cc=g++
cpp=-fPIC -Wall -g

inc+=-I./
libs+=-lprotobuf -lpthread
%*.o: %*.cc
	$(cc) $(cpp) $(inc) -o $@ $^ $(libs)
%*.o: %*.cpp
	$(cc) $(cpp) $(inc) -o $@ $^ $(libs)
all:server client
server: reactor_server_test.o  event_demultiplexer.o reactor.o global.o protocol.pb.o
	$(cc) $(cpp) $(inc) -o $@ $^ $(libs)
client: reactor_client_test.o  event_demultiplexer.o reactor.o global.o protocol.pb.o
	$(cc) $(cpp) $(inc) -o $@ $^ $(libs)
clean:
	rm server client *.o
.PHONY:clean all
