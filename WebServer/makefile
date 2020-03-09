

myhttpd:Main.o WebServer.o
	g++  Main.o WebServer.o -o myhttpd -lpthread
	rm -f *.o


Main.o:Main.cpp InputCheck.h
	g++ -Wall -g3 -c Main.cpp

WebServer.o:WebServer.cpp WebServer.h ThreadPool.h
	g++ -Wall -g3 -c WebServer.cpp


clean:
	rm -f WebServer
