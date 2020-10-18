#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h> 
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>

#define SOCKBUFFSIZE 64


#include "Workerfuncts.h"






// global flag to handle the sig int and sig quit signal
volatile sig_atomic_t wflag_int_q=0;
// int wflag_int_q=0;
volatile sig_atomic_t flag_usr1=0;



int main(int argc,char ** argv){
	
	//set up sigaction struct and link handlers
	struct sigaction action;
	action.sa_handler=sig_handler_worker;
	action.sa_flags = 0;
	sigemptyset (&action.sa_mask);
	
	sigaction(SIGINT,&action,NULL);
	// sigaction(SIGQUIT,&action,NULL);
	// sigaction(SIGUSR1,&action,NULL);


//nBflah shows if the worker is reforked or not
	// int nBflag=0;
	cout << "getpid is " << getpid() << endl;
	


	char input_dir[100];
	int numWorkers;
	int bufferSize;
	


	if (argc!=11){
		
			 cout << "Input error" << endl;
			 exit(1);
		
	}else{	
	

		for(int i=1;i<=9;i+=2){
			if(strcmp(argv[i],"-i")==0){
				strcpy(input_dir,argv[i+1]);
			}else if(strcmp(argv[i],"-b")==0){
				bufferSize = atoi(argv[i+1]);
			}else if(strcmp(argv[i],"-w")==0){
				numWorkers = atoi(argv[i+1]);
			}
		}

	}

//create fifofile paths
			 char temp[100];
			 char path1[50];
			 char path2[50];
			 int readfd;
			 int writefd;
			strcpy(path1,"./NamedPipes/fifo1.");
			strcpy(path2,"./NamedPipes/fifo2.");
			sprintf(temp, "%d", getpid());
			// cout << temp << endl;
			strcat(path1 ,temp);
			strcat(path2 ,temp);
			 // cout << path1 << endl;
			 // cout << path2 << endl;


		// // cout << "fifo files open" << endl;

			if( (readfd = open(path2,O_RDONLY)) < 0 ){
				cout << "problem worker rfile" << endl;
				exit(1);
			}
			if( (writefd = open(path1,O_WRONLY)) < 0 ){
				cout << "problem worker wfile" << endl;
				exit(1);
			}


			// cout << "kane plaka" << endl;


//read the directories worker is responsible for
				string* message = new string();
				Read(readfd,bufferSize,message);
				// cout << *message << endl;
//decode message from aggregator @ means end of word # means end of message
			
				int pos=0;
				char serverIP[100];
				int serverPort;
				SSList * dirs = new SSList();
				for(int j=0;j<message->length();j++){
					if (message->at(j)=='@'){
						if(message->substr(pos,2)=="-s"){
							strcpy(serverIP,(message->substr(pos+2,j-pos-2).c_str()));
							// cout << serverIP << endl;
						}
						else if(message->substr(pos,2)=="-p"){
							serverPort=stoi(message->substr(pos+2,j-pos-2));
						}
						else{
							dirs->Insert(message->substr(pos,j-pos));
						}
						pos = j+1;
					}
				}
//connect each worker to whoserver
			struct sockaddr_in whoserver;
		
			whoserver.sin_family = AF_INET;
			whoserver.sin_port = htons(serverPort); 
			
			if(inet_pton(AF_INET, serverIP , &whoserver.sin_addr)<=0){
				cout << "adress not supported" << endl;
			}  
			int sockfd;
			if ((sockfd = socket(AF_INET, SOCK_STREAM,0)) == -1){
				cout << "error opening socket" << endl;
			}

			if (connect(sockfd, (struct sockaddr *)&whoserver, sizeof(whoserver)) < 0) { 
		        cout << "connection fail" << endl; 
		    }



		    char masterIP[256];
			strcpy(masterIP,GetHostAddress());
			// cout << GetHostAddress() << endl;

//prepare the worker's ground to become miniserver
		    int workerS_fd, new_socket;
			struct sockaddr_in workerS;
			int addrlenworkerS = sizeof(workerS); 

			if ((workerS_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
				cout << "socket creattion prolem server" << endl;
			}

			workerS.sin_family=AF_INET;
			 
			workerS.sin_port=0;					//any available port
			
			if(inet_pton(AF_INET, masterIP , &workerS.sin_addr)<=0){
				cout << "adress not supported" << endl;
			} 

//resuse port and address
			int option = 1;
			if(setsockopt(workerS_fd,SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
				cout << "setsockopt failed" << endl;
				exit(1);
			}
		    
			if (bind(workerS_fd, (struct sockaddr *)&workerS, sizeof(workerS))<0){ 
		   		cout << "bind problem" << endl;
		        exit(1); 
		    }

		    

	//retrieve the available port chosen from port 0	   
		    if(getsockname(workerS_fd,(struct sockaddr *)&workerS,(socklen_t*)&addrlenworkerS) <0 ){
		    	cout << "getsock errror" << endl;
		    }
		    





				// dirs->Print();
//Create and fill data structures for each worker 
				LList *  Records = new LList();		
				Hashtable 	* ChildHT = new Hashtable(3,4);
				SSLNode* tempN=dirs->SSLHead;
//some parameteres neede from thw whoserver to complete job sent with summary statistics
				message->assign("-p");
				message->append(to_string(ntohs(workerS.sin_port)));
				message->append("@");
				message->append("-w");
				message->append(to_string(numWorkers));
				message->append("@");
				message->append("-a");
				message->append(masterIP);
				message->append("@");
				
				while (tempN!=NULL){
					
					DIR *dp;
					struct dirent * e;
//List of date file names			
					SSList* files = new SSList();

					int filecount = 0;
					strcpy(temp,input_dir);
					strcat(temp,"/");
					strcat(temp,tempN->data->c_str());
					dp = opendir(temp);
					//cout << temp << endl;

					if (dp==NULL){
						cout << "eddw problem" << endl;
						exit(1);
					}

					while ( (e=readdir(dp)) != NULL){
						if (e->d_type == DT_REG){
							filecount++;	
							files->Insert(e->d_name);
						}
					} 
					rewinddir(dp);				
					closedir(dp);

					//files->Print();
					//cout << "--------------" << endl;
//Sort datefiles to be in correct chronological order for insertion					
					files->Sort(1);
					//files->Print();

					InsertChildFileRecords(Records, ChildHT, tempN, files,temp);
//Message of summary statistics for each file of the same directory
					message->append(*tempN->data);
					message->append("\n");

					SSLNode* idate=files->SSLHead;				
					while(idate!=NULL){
						message->append(*idate->data);
						message->append("\n");
						//cout << "ola kala" << endl;
						ChildHT->SummaryStatistics(idate->data,message);


						idate = idate->Next;

					}
					delete files;
					tempN= tempN->Next;
				}
			



		    if (listen(workerS_fd, 100) < 0){ 
		        cout << "listen error" << endl;
		        exit(1); 
		    }
		   
//sent summary stats to whoserver
		    Write(sockfd,SOCKBUFFSIZE,message);

		    close(new_socket);
		    close(sockfd);
//miniserver worker accepts connections from whoserver thread to anser queries
		    if ((new_socket = accept(workerS_fd, (struct sockaddr *)&workerS, (socklen_t*)&addrlenworkerS))<0) 
		    { 
		    	cout << "accept error" << endl;
		        exit(1); 
		    }


   	 		close(workerS_fd);



    		Read(new_socket,SOCKBUFFSIZE,message);

    		cout << *message << endl;

    		int SUCCESS=0;
    		int FAIL=0;
   		
    		if(wflag_int_q==0)
    			QueriesAnswer(Records,ChildHT,dirs,new_socket,SOCKBUFFSIZE,SUCCESS,FAIL,input_dir);

    		
			//	Records->Print();
   	 		close(new_socket);
			

			delete dirs;
			
			
			delete message;
			delete Records;
			delete ChildHT;

//close and delete fifo files

			close(readfd);
			close(writefd);

			if(unlink(path1) < 0){
				cout << "problem" << endl;
				exit(1);
			}
			if(unlink(path2) < 0){
				cout << "problem" << endl;
				exit(1);
			}

}