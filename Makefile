all: master Worker whoServer whoClient

objects1=master.o WorkerDS.o masterClasses.o customio.o
objects2=Worker.o WorkerDS.o Workerfuncts.o customio.o
objects3=whoServer.o cbuffer.o whoServerfuncts.o customio.o
objects4=whoClient.o customio.o 

master: $(objects1)
	g++ -std=c++11 -o master $(objects1)

WorkerDS.o: WorkerDS.cpp
	g++ -std=c++11 -c WorkerDS.cpp

masterClasses.o: masterClasses.cpp
	g++ -std=c++11 -c masterClasses.cpp

master.o: master.cpp
	g++ -std=c++11 -c master.cpp

customio.o: customio.cpp
	g++ -std=c++11 -c customio.cpp



Worker :$(objects2)
	g++ -std=c++11 -o Worker $(objects2)

Workerfuncts.o :Workerfuncts.cpp
	g++ -std=c++11 -c Workerfuncts.cpp

Worker.o :Worker.cpp
	g++ -std=c++11 -c Worker.cpp



whoServer :$(objects3)
	g++ -std=c++11 -o whoServer $(objects3) -lpthread

whoServer.o : whoServer.cpp
	g++ -std=c++11 -c whoServer.cpp

cbuffer.o : cbuffer.cpp
	g++ -std=c++11 -c cbuffer.cpp


whoServerfuncts.o : whoServerfuncts.cpp
	g++ -std=c++11 -c whoServerfuncts.cpp



whoClient: $(objects4)
	g++ -std=c++11 -o whoClient $(objects4) -lpthread

whoClient.o: whoClient.cpp
	g++ -std=c++11 -c whoClient.cpp



clean  :
	rm master $(objects1)
	rm Worker Worker.o Workerfuncts.o
	rm whoServer whoServer.o cbuffer.o whoServerfuncts.o
	rm whoClient whoClient.o
	rm ./NamedPipes/*


