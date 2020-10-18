#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h> 
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <netdb.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 


#include "Workerfuncts.h"

using namespace std;


// sig int and quit handler change value of global flag
void sig_handler_worker(int signo){


	if (signo == SIGINT){
		wflag_int_q=1;
	}
	// if (signo == SIGUSR1){
	// 	flag_usr1=1;
	// }
	
}






void InsertChildFileRecords(LList* Records,Hashtable * ChildHT,SSLNode* dirnode,SSList* files,char* path){
	
//for a country directory read throught its files' content	

	SSLNode * iter = files->SSLHead;
	while (iter!=NULL){
		char filepath[100];
		ifstream infile;
		
		strcpy(filepath,path);
		strcat(filepath, "/");						
		strcat(filepath, iter->data->c_str());
		
		infile.open(filepath);
		if(infile.fail()) exit(1);
		string record_id,Move,Fname,Lname,Disease,Date,Age;
		LList * holdexits = new LList();


		while(infile>>record_id){		//import the data from the file
			bool insert = true;;
			LLNode* found = Records->Search(record_id);		
			infile >> Move;
			infile >> Fname;
			infile >> Lname;
			infile >> Disease;
			infile >> Age;

			if (stoi(Age)<=0 || stoi(Age)>120){
				// cout << "Wrong age "<< Age << endl;
				insert = false;

			}else if(found!=NULL){					//list insert in increasing date order, so :
				if (Move=="ENTER"){				//the one found is chonologiacally before so same rid double Enter, error
					insert=false;
					//found->Print();
					// cout << "date is" << *(iter->data) << endl;
					//cout << "this record already exists" << endl;
				}

			}else{							//not found
				if (Move=="EXIT"){			//no enter beforehand so keep it in a list beacuse there might be an enter  on the same date further down the file
					insert=false;
					holdexits->Insert(record_id,Fname,Lname,Disease,*(dirnode->data),"--",*(iter->data),stoi(Age));

					//cout << " possible wrong chonologiacall exit" << endl;
				}
			} 

			//cout << "Insert record: Record_id "<< record_id <<"||"<<Fname << "||"  <<Lname<<  "||" << Disease << "||" << *(tempN->data) << "||" << *(iter->data)<< "||" << Move << "||" <<Age << endl;
			if(insert==true){
				if (Move=="ENTER"){
					Records->Insert(record_id,Fname,Lname,Disease,*(dirnode->data),*(iter->data),"--",stoi(Age));
					ChildHT->Insert(Disease,Records->getLLHead());
				}else if(Move=="EXIT"){
					found->setExit(*(iter->data));
				}
			}else{
				//cout << "ERROR" << endl;	
			}
		}
		LLNode * find;
		LLNode *i=holdexits->getLLHead();			//chekarw gia ta exits tou idiou arxeio hmeromhnias
		while(i!=NULL){
			// cout << "problem  is here obviously" << endl;
 			find=Records->Search( *(i->getid()) );		//brhke enter prin ara to bazw
			if(find!=NULL){
				// find->Print();
				find->setExit(*(i->getExit()) );
				find=NULL;
			}
			i=i->getNext();
		}
		delete holdexits;					
		iter = iter->Next;
		infile.close();
	}
}






void QueriesAnswer(LList* Records,Hashtable* HT,SSList * Countries,int &fd,int bufferSize,int& SUCCESS,int& FAIL,char* input_dir){
	 string Request,disease,country,date1,date2;
 	int k;
 	int exit=0;
		// write()
//communication interface to parent
 	while(1){

 		string* message = new string("");
 		if(wflag_int_q==0)
 			Read(fd,bufferSize,message);
 		// cout << *message << endl;
 		if(wflag_int_q==1 || *message=="/exit#"){			//sigint or server shutdow
	 		cout << getpid() << " exit" << endl;
	 		delete message;
 			break;
 		}
 
 		if(*message=="ERROR@#"){
 			FAIL++;
 		}
//Decode the message	
		int count = 0; 		
 		int pos=0;
 		int length=message->length();
 		string ** vars = new string*[6];
 		for(int j=0;j<6;j++){
 			vars[j]=new string();
 		} 
 		for(int j=0;j<length;j++){
			if(message->at(j)=='@'){
				if(++count>6)	break;
				
				vars[count-1]->assign(message->substr(pos,j-pos));
				
				pos = j+1;
			}
		}
		if(count>6) {
			//;
			cout << "Wrong input, try again" << endl;

		}
			if(*vars[0]=="/diseaseFrequency"){
				if (count==4){
					vars[4]->assign("");

				}else if(count!=5){
					cout << "Wrong input, try again" << endl;
					//continue;
				}
				char  num[20];
				//cout << HT->diseaseFrequency(vars[1],vars[4],vars[2],vars[3]) << endl;
				sprintf(num,"%d",HT->diseaseFrequency(vars[1],vars[4],vars[2],vars[3]));	// disease,country,date1,date2
				//cout << num << endl;
				message->assign("");
				message->append(num);
				// cout << *message << endl;

				// disease,country,date1,date2
			}else if(*vars[0]=="/topk-AgeRanges"){
				
				HT->TopKAgeRanges(stoi(*vars[1]),vars[3],vars[4],vars[5],vars[2],message);
				// TopKAgeRanges(int k,string* disease,string* date1,string* date2,string* country,string* &message)		//topk
				// cout << *message << endl;	

					//TopKDisease(diseasehashtable,k,country,date1,date2);					

			}else if(*vars[0]=="/searchPatientRecord"){
				// if (count!=2){
				// 	cout << "Wrong input,try again" << endl;
				// 	continue;
				// }
				//Print the information
				LLNode * node;
				if ((node = Records->Search(*vars[1]))!=NULL){
					node->ContentMessage(message);
				}else{
					message->assign("NotFound");
				}

			}else if(*vars[0]=="/numPatientAdmissions"){	
				if (count==4){
					vars[4]->assign("");
				}else if(count!=5){
					cout << "Wrong input ,try again" << endl;
				}
				string EnorEx;
//Count for admissions
				EnorEx.assign("Entry");
				HT->numPatientAorD(vars[1],vars[2],vars[3],vars[4],message,Countries,&EnorEx); // disease,country,date1,date2

			}else if(*vars[0]=="/numPatientDischarges"){
				if (count==4){
					vars[4]->assign("");
				}else if(count!=5){
					cout << "Wrong input ,try again" << endl;
				}
				string EnorEx;
//Count for discharges
				EnorEx.assign("Exit");
				HT->numPatientAorD(vars[1],vars[2],vars[3],vars[4],message,Countries,&EnorEx); // disease,date1,date2,country,...
											//exit loop and user interface
			}else if (*vars[0]=="ERROR"){
				message->assign(*vars[0]);
			}
		//cout << *message << endl;
	 	Write(fd,bufferSize,message);
	 	for (int j=0;j<6;j++){
	 		//cout << *vars[j] << endl;
	 		delete vars[j];
	 	}
	 	delete[] vars;
	 	delete message;
	}

}



char* GetHostAddress(void){

	char hostnamebuff[256]; 
    char *IPbuff; 
    struct hostent *host_entry; 
    int hostname; 
  
 
    if( (hostname=gethostname(hostnamebuff, sizeof(hostnamebuff)) ) == -1){ 
        cout << "gethostname error" << endl; 
        exit(1); 
    } 
 
     
    if( (host_entry=gethostbyname(hostnamebuff)) == NULL){ 
        cout << "host information eror" << endl;
        exit(1); 
    } 
 

    
    if ( (IPbuff=inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0])) ) == NULL){ 
        cout << "address conversion problem" << endl; 
        exit(1); 
    } 
  

  
    return IPbuff; 
} 


