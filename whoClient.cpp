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
#include <pthread.h>
#include <thread> 
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <fstream>

#include "customio.h"

#define SOCKBUFFSIZE 64 

using namespace std;

mutex coutmtx;
mutex mtx;
condition_variable cv;
bool ready;

//C++11 based conditional variables and mutexes


void go() {
  std::unique_lock<std::mutex> lck(mtx);
  ready=true;

  cv.notify_all();
}



void qthread_f(string query,int servPort,char* servIP){
	// cout << "just started thread: " << pthread_self() <<endl;
	//prepare connect to whoserver
	struct sockaddr_in server;
	struct hostent *rem;
	server.sin_family = AF_INET;
	server.sin_port = htons(servPort); 

	if(inet_pton(AF_INET, servIP , &server.sin_addr)<=0){
		cout << "adress not supported" << endl;
	}  

	//sockfd will be stored inside circular buffer
	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM,0)) == -1){
		cout << "error opening socket" << endl;
	}


	std::unique_lock<std::mutex> lck(mtx);			//object to handle cond variable correctly
	 while (!ready){
    	cv.wait(lck);			//wait till all threads are spawed
	}

	//connect
	if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) { 
		        cout << "connection fail" << endl; 
	}
	//send query
	Write(sockfd,SOCKBUFFSIZE,&query);
	
	string answer;
	Read(sockfd,SOCKBUFFSIZE,&answer);

	//lock output
	coutmtx.lock();
	cout << query << "\n" << answer << "\n\n";
	coutmtx.unlock();

	close(sockfd);
	return;

}	
//C++11 based conditional variables and mutexes


int main(int argc,char ** argv){


	char queryFile[100];
	int numThreads;
	char servIP[100];
	int servPort;



	if (argc!=9){
		cout << "Input error" << endl;
		exit(1);
	}else{			
		int i;
		for(i=1;i<=7;i+=2){
			if(strcmp(argv[i],"-sip")==0){
				strcpy(servIP,argv[i+1]);
			}else if(strcmp(argv[i],"-sp")==0){
				servPort = atoi(argv[i+1]);
			}else if(strcmp(argv[i],"-w")==0){
				numThreads = atoi(argv[i+1]);
			}else if(strcmp(argv[i],"-q")==0){
				strcpy(queryFile,argv[i+1]);
			}else exit(1);			
		}

		 // if(bufferSize<1) exit(1);
		 if(numThreads<1) exit(1);
		 if(servPort<1024) exit(1);

		 cout << "number of threads is: " << numThreads << endl;
		 cout << "servPort is : " << servPort << endl;
		 cout << "servIP is: " << servIP << endl;
		 cout << "queryFile is: " << queryFile << endl;
	 
	}

	string query;
	
	int lines=0;
	ifstream fsqueryFile(queryFile);
	if (!fsqueryFile.good()){
		cout << "error opening file" << endl;
		exit(1);	
	}
	bool eof=false;
	do{
		ready=false;
		int ethreads=0;
		thread thrT[numThreads];
		for(int i=0; i<numThreads; ++i){
			if(getline(fsqueryFile,query)){
				ethreads++;
				thrT[i]=thread(qthread_f,query,servPort,servIP);		//create a thread for each query
    
			
			}else{
				eof=true;
				break;
			}

		}

		if(ethreads>0){
			go();
			// for (auto& th : thrT) th.join();
			for(int i=0; i<ethreads;i++){
				thrT[i].join();
			}
		}		

	}while(eof==false);			//create more threads if there are more queries

	fsqueryFile.close();

}