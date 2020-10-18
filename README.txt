README


K24: System Programming Project3
Thodoris Maximilianos Chytis 
1115201700197


This project successfully tackles the task of interprocess communication through network socket programming. Furthermore two out of the three executables are multithreaded programs the threads of which are handled with mutexes and conditional variables. The programming language used is C++.

	Components:
    1) source : master.cpp masterClasses.cpp Worker.cpp Workerfuncts.cpp WorkerDS.cp whoServer.cpp whoServerfuncts.cpp cbuffer.cpp whoClient.cpp customio.cpp		#customio read and write functions , cbuffer circular buffer class and list class
    2) header : masterClasses.h Workerfuncts.h WorkerDS.h whoServerfuncts.h cbuffer.h customio.h
    3) makefile : 	Makefile   #compiles  3 programs master,whoServer,whoClient. The Worker is called by the maaster via execv 
    4) diseasesFile,CountriesFileEU,queryFile.
    5)bash script: create_infiles.sh #creates the directories and records needed for the program	
    6)NamedPipes: directory where the the fifo files for ipc master-workers are stored



   Important Notes:
   	-workers first connect to whoserver to send information and then become mini servers waiting for queries from a whoserverthread
   	-whoserver first creates threads, accepts and connects to all numwork servers with help of threads
   	-then whoserver proceeds to accept connections from whoClient
   	-whoserver threads proceed to connect to miniserverworkers to get answer to queries and forward it back to the client
   	-the whoClient creates numthreads threads and starts them all together to race for a connection to whoserver
   	-whoserver keeps a  global list of file descriptors to all workers after the threads connected to the workers by obtaining sockets from the circular buffer
   	-inside the circular buffer the origin of the fd is marked.0=worker 1=client
   	-whoServer stout is the summary statistics
   	-whoClient stout is the answers of the threads from whoserver to the queries send by the threads of whoClient
   	-in the stout of whoServer its hostnameaddress is printed which must be entered as a parameter to ./master and ./whoClient
   	-caution:
   			 i)parameter -sip servIP of whoclient must be == parameter -s serverIP of master
   			 ii)parameter -sp servPort of whoClient must be == parameter -q queryPortNum of whoServer
   			 iii)parameter -p serverPort of master must be == parameter -s statisticsPortNum of whoserver 
   	-muteces protect all stdout for synced, correct output
   	-SOCKBUFFERSIZE is defined at the beggining of the programs
   	-to sync threads of whoClient c++11 cond vars and muteces are used
   	-to sync threads of whoServer c++11 cond vars and traditional pthread muteces are used
   	-there are signal handler installed so that a sigint sent to master,workers and whoserver leads to correct exit of all running processes
   	-if server is siginted before master then master shoutsdown smoothly aswell
   	-The dates entered follow the format dd-mm-yyyy and date1 < date2.
	-master keeps information about workers in an array of pointers to Child Classes
	-strings and string::append is used for IPC the protocol is:
		@:end of word
		#:end of message
	-No slow worker impacts the communication because first the message is transmitted to the corrrect process,processes (write) while they already proceeed acccomplishing the job
	-if buffersize for fifo files is smaller than message then #buffersize bytes are sent multiple times until message is complete


compilation and execution:

#pwd is ThodorisMaxmilianosChytis-Project2


-bash file: 
	
	./create_infiles.sh diseasesFile countriesFileEU input_dir 8 8

-complex of master,Worker,whoServer,whoClient
	
	make

1)	./whoServer -s 8080 -b 4 -w 8 -q 8000
	AT THIS POINT STOUT PRINTS THE HOSTADDRESS OF whoServer call it IP  
	
2)	./master -b 8 -w 5 -i input_dir -s IP -p 8080

3)	./whoClient -sip IP -sp 8000 -w 3 -q queryFile

4)	sigint on whoServer

	make clean

not mandatorily on same host
