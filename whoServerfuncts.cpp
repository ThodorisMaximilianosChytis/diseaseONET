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
#include <pthread.h>
#include <netdb.h>
#include <sstream>

#include "whoServerfuncts.h"

using namespace std;


int DateCompareEarlier(string* date1,string* date2){			//1 -> date1 is earlier || 2->date2 os earlier
		if(*date1=="--")					//-- is always larger
			return 2;
		else if(*date2=="--")
			return 1;
		//sum of dd-mm-yyyy , giving the correct weights
		unsigned int sum1= atoi(date1->substr(0,2).c_str())*1 + atoi(date1->substr(3.2).c_str())*31 + atoi(date1->substr(6.4).c_str())*31*12;		
		unsigned int sum2= atoi(date2->substr(0,2).c_str())*1 + atoi(date2->substr(3.2).c_str())*31 + atoi(date2->substr(6.4).c_str())*31*12;
	
		if (sum1<sum2)
			return 1;
		else if(sum2<sum1)
			return 2;
		else
			return 0; 

}




void UserQueries(string Request,IList* sockfdL,int bufferSize,int numwork,string& answer){
	 string disease,country,date1,date2,record_id;
 	int error,k,w_s_q;
 		
 		error=0;
 		w_s_q=-1;		//flag from previous project ot used here
 						// /Start the user interface
			
			stringstream wordcount(Request);
			stringstream srequest(Request);

			int count = 0;
			string temp;
			while(wordcount>>temp)			//count number of words given
				count++;
			
			 string app;
			 srequest>>app;				//get app name
			// cout << app << endl;
				
				if(app=="/diseaseFrequency"){
				srequest >> disease;
				srequest >> date1;
				srequest >> date2;
				
				if (count==5){
					srequest >> country;
				}else if (count==4){
					country.assign("");
				}else{
					cout << "Wrong input, try again" << endl;
					error=1;
				}
				if (error!=1 && DateCompareEarlier(&date2,&date1)==1){
					cout << "Error: exit date given earlier than entry date" << endl;
					error=1;
				}
				if(error!=1)
					diseaseFrequency(sockfdL,app,disease,country,date1,date2,bufferSize,numwork,answer,w_s_q);
				
				

			}else if(app=="/topk-AgeRanges"){
				if (count!=6){
					cout << "Wrong input, try again" << endl;
					error=1;
				}
					srequest >> k;
					if(k>4) k=4;
					if (k<1) k=1;
					
					srequest >>country;
					srequest >> disease;
					srequest >>date1;
					srequest >> date2;

				if (error!=1 && DateCompareEarlier(&date2,&date1)==1 ){
					cout << "Error: exit date given earlier than entry date" << endl;
					error=1;
				}						
			 
				if(error!=1)
			 		TopKAgeRanges(sockfdL,k,disease, country, date1, date2, bufferSize, numwork,answer,w_s_q);

			

			}else if(app=="/searchPatientRecord"){

				if(count!=2){
					cout << "Wrong input,try again" << endl;
					error=1;
				}
				srequest >> record_id;

				if(error!=1)
					searchPatientRecord(sockfdL,record_id,bufferSize,numwork,answer,w_s_q);
				


			}else if(app=="/numPatientAdmissions"){
				srequest >> disease;
				srequest >> date1;
				srequest >> date2;
				
				if (count==5){
					srequest >> country;
				}else if (count==4){
					country.assign("");
				}else{
					cout << "Wrong input, try again" << endl;
					error=1;
				}
				if (error!=1 && DateCompareEarlier(&date2,&date1)==1){
					cout << "Error: exit date given earlier than entry date" << endl;
					error=1;
				}
				if(error!=1)
					diseaseFrequency(sockfdL,app,disease,country,date1,date2,bufferSize,numwork,answer,w_s_q);
				

			}else if(app=="/numPatientDischarges"){
				srequest >> disease;
				srequest >> date1;
				srequest >> date2;
				if (DateCompareEarlier(&date2,&date1)==1){
					cout << "Error: exit date given earlier than entry date" << endl;
					error=1;
				}
				if (count==5){
					srequest >> country;
				}else if (count==4){
					country.assign("");
				}else{
					cout << "Wrong input, try again" << endl;
					error=1;
				}
				if(error!=1)
					diseaseFrequency(sockfdL,app,disease,country,date1,date2,bufferSize,numwork,answer,w_s_q);
				

			// }else if (app=="/exit"){
			// 	// exit(sockfdL,bufferSize,numwork,answer,w_s_q);
			// 	// break;							//exit loop and user interface
			}else{
				error=1;
			}
			if(error==1){
				SendError(sockfdL,bufferSize,numwork,answer,w_s_q);
			}
 
}


void SendError(IList* sockfdL, int bufferSize, int numwork,string &answer,int& w_s_q){
	string * message = new string("ERROR@");
	string Query=*message;
	// cout << "ERROR" << endl;
	answer.assign("ERROR");

	ILNode* temp=sockfdL->ILHead;
	while(temp!=NULL){					//sockfdl: sockets to all workers
		Write(temp->data,bufferSize,&Query);
		temp=temp->Next;
	}
	temp=sockfdL->ILHead;
	while(temp!=NULL){
		Read(temp->data,bufferSize,message);
		
		temp=temp->Next;
	}


	delete message;
}









void searchPatientRecord(IList* sockfdL,string record_id,int bufferSize,int numwork,string& answer,int& w_s_q){
	string* message = new string("/searchPatientRecord");
	message->append("@");
	message->append(record_id);
	message->append("@");
	string Query = *message;
	
	answer.assign("NotFound");

	ILNode* temp=sockfdL->ILHead;
	while(temp!=NULL){
		Write(temp->data,bufferSize,&Query);
		temp=temp->Next;
	}

	temp=sockfdL->ILHead;
	while(temp!=NULL){
		Read(temp->data,bufferSize,message);
		//cout << *message << endl;
		if(*message!="NotFound#"){
			answer.assign(*message);
			// cout << *message << endl;
		}

		temp=temp->Next;
	}



	//cout << *wholemess << endl;
	delete message;
	//delete wholemess;

}

void diseaseFrequency(IList* sockfdL,string app,string virusName,string country,string date1,string date2,int bufferSize,int numwork,string& answer,int& w_s_q){
	int process=-1;
	int freq=0;

	string* message = new string(app);
	string* wholemess = new string();
	message->append("@");
	message->append(virusName);
	message->append("@");
	message->append(date1);
	message->append("@");
	message->append(date2);
	message->append("@");

//with country
	if (country!=""){

			message->append(country);
			message->append("@");
			
			wholemess->assign(country);
			wholemess->append(" 0");	
	}
		//no country given go through all of them
		string Query = *message;
		ILNode* temp=sockfdL->ILHead;
		while(temp!=NULL){
			Write(temp->data,bufferSize,&Query);
			temp=temp->Next;
		}

		temp=sockfdL->ILHead;
		while(temp!=NULL){
			int optimize=1;

			Read(temp->data,bufferSize,message);

//count freq			
			if (app=="/diseaseFrequency"){ 
				freq+=stoi(message->substr(0,message->length()-optimize));
			}
///list for numPatient Queries			
			else{
				if (country!=""){
					stringstream s(message->substr(0,message->length()-optimize));
					string t;
					s >> t;
					s >> t;
					if (stoi(t)!=0){
						wholemess->assign(message->substr(0,message->length()-optimize));	
					}

				}else
					wholemess->append(message->substr(0,message->length()-optimize));			
			}
			temp=temp->Next;
		}

	if (app=="/diseaseFrequency"){
		// cout << "freq is " <<freq << endl;
		answer.assign("freq is ");
		answer.append(to_string(freq)); 
	}
	else{
		// cout << *wholemess << endl;
		answer.assign(*wholemess);
	}


	delete message;
	delete wholemess;

	return;
}






void TopKAgeRanges(IList* sockfdL,int k,string disease,string country,string date1,string date2,int bufferSize,int numwork,string& answer,int& w_s_q){
	int process=-1;
	int freq=0;
	char numk[2];
	string* message = new string("/topk-AgeRanges");
	// string* wholemess = new string();
	message->append("@");
	sprintf(numk,"%d",k);
	message->append(numk);
	message->append("@");
	message->append(country);
	message->append("@");
	message->append(disease);
	message->append("@");
	message->append(date1);
	message->append("@");
	message->append(date2);
	message->append("@");
	


		ILNode* temp=sockfdL->ILHead;
		while(temp!=NULL){
			Write(temp->data,bufferSize,message);
			temp=temp->Next;
		}
		// Write(Table[process]->pathw,Table[process]->wfd,bufferSize,message);
		int found=0;
		temp=sockfdL->ILHead;
		while(temp!=NULL){
			Read(temp->data, bufferSize, message);
			temp=temp->Next;

			if (*message!="#"){
				// cout << *message << endl;
				answer.assign(*message);
				found=1;
			}
		}
		if (found==0){
			// cout << "0-20: 00.00%\n41-60: 00.00%\n21-40: 0.00%\n60+: 0.00%" << endl;
			answer.assign("0-20: 00.00%\n41-60: 00.00%\n21-40: 0.00%\n60+: 0.00%");	//not found
		}


	delete message;
}

//create log file


void sig_handler(int signo)
{
  if (signo == SIGINT){
		flag_int=1;
		pthread_cond_broadcast(&cond_nonempty);
		write(0,"whoServershutdown\n\0",19);
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