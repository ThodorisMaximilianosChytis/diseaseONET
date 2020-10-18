#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <pthread.h>
#include <netdb.h>
#include <signal.h>

#define SOCKBUFFSIZE 64

#include "whoServerfuncts.h"
// #include "customio.h"
// #include "cbuffer.h"


using namespace std;


volatile sig_atomic_t flag_int=0;		//signal flag



pthread_mutex_t nWrmtx=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t thrmtx=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;


pthread_cond_t numWorkready;			//to handle if the numworker is initiliased
pthread_cond_t cond_nonempty;			//cond vaiables used for c buffer
pthread_cond_t cond_nonfull;

Cbuff buffer;				//global circular buffer
int numWorkers=-1;			//the number of workers
IList* WsockfdL;			//list of sockets











void Sthread_f(){

		 
		int sock;
		int origin;
		char masterIP[256];	//master and workers' ip
	 while(flag_int==0){

		int sock;
		int origin;
		buffer.optain(sock,origin);		//get a fd from cbuffer
		if(flag_int==0){	//no sigint
			pthread_cond_signal(&cond_nonfull);

			string* message = new string("");
			if (origin==0){					//fd was from a worker
				int workerSport;
				Read(sock,SOCKBUFFSIZE,message);		//message of summary statistics and information
				pthread_mutex_lock(&thrmtx);
				cout << *message << endl;			//lock output
				pthread_mutex_unlock(&thrmtx);
				int pos=0;
				for(int j=0;j<message->length();j++){			//getinformation from message
					if (message->at(j)=='@'){
						// cout << message->substr(pos+2,j-pos-2) << endl;
						if(message->substr(pos,2)=="-p"){
							workerSport=stoi(message->substr(pos+2,j-pos-2));

						}
						if(message->substr(pos,2)=="-w"){
							int temp=numWorkers;
							numWorkers=stoi(message->substr(pos+2,j-pos-2));
							
							if(temp==-1){
								// cout << "signal numWorkers ready" << endl;
								pthread_cond_signal(&numWorkready);}
						}
						if(message->substr(pos,2)=="-a"){
							strcpy(masterIP,message->substr(pos+2,j-pos-2).c_str());
							// cout << masterIP << endl;
						}
						pos = j+1;
					}
				}
				close(sock);
				//connect to workerminiServer
				struct sockaddr_in workerS;
				workerS.sin_family = AF_INET;
				workerS.sin_port = htons(workerSport); 

				if(inet_pton(AF_INET, masterIP , &workerS.sin_addr)<=0){
					cout << "adress not supported" << endl;
				}  

				int workerSsockfd;
				if ((workerSsockfd = socket(AF_INET, SOCK_STREAM,0)) == -1){
					cout << "error opening socket" << endl;
				}

				if (connect(workerSsockfd, (struct sockaddr *)&workerS, sizeof(workerS)) < 0) { 
			        cout << "connection fail" << endl; 
			    }
				pthread_mutex_lock(&thrmtx);		//lock common resources
			    WsockfdL->Insert(workerSsockfd);
				pthread_mutex_unlock(&thrmtx);

				message->assign("WhoServer connected to a Worker");
				Write(workerSsockfd,1,message);

			}else if(origin==1){	//fd is from client
									//fd came from whoclient
				Read(sock,SOCKBUFFSIZE,message);
				int length=message->length();

				pthread_mutex_lock(&thrmtx);		//lock common resources
				// cout << message->substr(0,length-1) << endl;
				string answer;
				UserQueries(message->substr(0,length-1),WsockfdL,SOCKBUFFSIZE,numWorkers,answer);
				// cout << answer << endl;
				pthread_mutex_unlock(&thrmtx);
				Write(sock,SOCKBUFFSIZE,&answer);

				close(sock);
			}	
	    	delete message;
	    }
	}	
	return;

}








int main(int argc,char ** argv){



	struct sigaction action;
	action.sa_handler=sig_handler;
	action.sa_flags = 0;
	sigemptyset (&action.sa_mask);
	
//signal handlers	
	sigaction(SIGINT,&action,NULL);




	int queryPortNum;
	int bufferSize;
	int numThreads;
	int statisticsPortNum;

	if (argc!=9){
		cout << "Input error" << endl;
		exit(1);
	}else{			
		int i;
		for(i=1;i<=7;i+=2){
			if(strcmp(argv[i],"-s")==0){
				statisticsPortNum = atoi(argv[i+1]);
			}else if(strcmp(argv[i],"-b")==0){
				bufferSize = atoi(argv[i+1]);
			}else if(strcmp(argv[i],"-w")==0){
				numThreads = atoi(argv[i+1]);
			}else if(strcmp(argv[i],"-q")==0){
				queryPortNum = atoi(argv[i+1]);
			}else exit(1);			
		}

		 if(bufferSize<1) exit(1);
		 if(numThreads<1) exit(1);
		 if(statisticsPortNum<1024) exit(1);			//0-2013 reserved
		 if(queryPortNum<1024) exit(1);

		 cout << "number of threads is: " << numThreads << endl;
		 cout << "bufferSize is: " <<bufferSize << endl;
		 cout << "statisticsPortNum is: " << statisticsPortNum << endl;
		 cout << "queryPortNum is: " << queryPortNum << endl;
	}


	char whoServerIP[256];
	strcpy(whoServerIP,GetHostAddress());
	cout << GetHostAddress() << endl;

//create global circula buffer	
	buffer.CreateDataArray(bufferSize);

	WsockfdL = new IList();			//list of file descriptors connected to miniserverworkers

	thread thrA[numThreads];
	for (int i=0;i<numThreads;i++){
		thrA[i]=thread(Sthread_f);
	}

	if(flag_int==0){				//no sigint
		
		//socket to receive workers connections
		int server_fd, new_socket;
		struct sockaddr_in server;
		int addrlen = sizeof(server); 

		if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
			cout << "socket creattion prolem server" << endl;
		}
		server.sin_family=AF_INET;
		// server.sin_addr.s_addr = INADDR_ANY; 
		server.sin_port=htons(statisticsPortNum);

		if(inet_pton(AF_INET, whoServerIP , &server.sin_addr)<=0){
					cout << "adress not supported" << endl;
		}  
		
		int option=1;
		if(setsockopt(server_fd,SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
			cout << "setsockopt failed" << endl;
			exit(1);
		}

		if (bind(server_fd, (struct sockaddr *)&server, sizeof(server))<0){ 
	   		cout << "bind problem" << endl;
	        exit(1); 
	    }
	   
	    
	    if (listen(server_fd, 100) < 0){ 
	        cout << "listen error" << endl;
	        exit(1); 
	    }




	    
		int WorkersAccepted=0;
	    while(numWorkers!=WorkersAccepted && flag_int==0){		//sigint or all workers accepted
		    if ((new_socket = accept(server_fd, (struct sockaddr *)&server, (socklen_t*)&addrlen))<0){ 
			    	if (flag_int==1) break;
			    	cout << "accept error" << endl;
			        exit(1); 
			}
				WorkersAccepted++;
				buffer.place(new_socket,0);	//zero means that this fd is coming from a worker and not from whoclient
				pthread_cond_signal(&cond_nonempty);	

				if (numWorkers==-1){
					pthread_mutex_lock(&nWrmtx);
					pthread_cond_wait(&numWorkready,&nWrmtx);
					pthread_mutex_unlock(&nWrmtx);
				}
			// cout << "workers WorkersAccepted is:  " << WorkersAccepted  << "=="<< numWorkers  << endl;	

	    } 
	    close(server_fd);
	    
	
	 		cout << "WHOSERVER READY TO RECEIVE QUERIES" << endl;
//socket to recice whoclient connections
		int qserver_fd;
		struct sockaddr_in qserver;
		addrlen = sizeof(qserver); 

		if ((qserver_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
			cout << "socket creattion prolem server" << endl;
		}
		qserver.sin_family=AF_INET;
		// qserver.sin_addr.s_addr = INADDR_ANY; 
		qserver.sin_port=htons(queryPortNum);
		
		if(inet_pton(AF_INET, whoServerIP , &qserver.sin_addr)<=0){
					cout << "adress not supported" << endl;
		}

		option=1;  
		if(setsockopt(qserver_fd,SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
					cout << "setsockopt failed" << endl;
					exit(1);
		}
		if (bind(qserver_fd, (struct sockaddr *)&qserver, sizeof(qserver))<0){ 
	   		cout << "bind problem" << endl;
	        exit(1); 
	    }
	    
	    if (listen(qserver_fd, 100) < 0){ 
	        cout << "listen error" << endl;
	        exit(1); 
	    }


	    while(flag_int==0){				//accept from client
		    if ((new_socket = accept(qserver_fd, (struct sockaddr *)&qserver, (socklen_t*)&addrlen))<0){ 
			    	if (flag_int==1){
			    		break;
			    	} 
			        exit(1); 
			}
				buffer.place(new_socket,1);						//1: origin is whoclient
				pthread_cond_signal(&cond_nonempty);			
	    }
		
		close(qserver_fd);


	}
	
	string exit;
	exit.assign("/exit");
	ILNode * temp;
	temp=WsockfdL->ILHead;
	while(temp!=NULL){			//close fds to workers and tell them to shutdown
		// cout << "ante geia" << endl;
		Write(temp->data,SOCKBUFFSIZE,&exit);			//dinei shma stous workers oti termatize wste na termatisoun ki autoi
		close(temp->data);
		temp=temp->Next;
	}
	    

    for (auto& th : thrA) th.join();




	delete WsockfdL;

    return 0;

}