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
#include <pthread.h>
#include <netdb.h>
#include <sstream>
// #include<signal.h>

#include "cbuffer.h"

using namespace std;









void Cbuff::CreateDataArray(int _bufferSize){
	bufferSize=_bufferSize;
	fdsA=new int[bufferSize];			//array of fds
	fdsAorigin=new int[bufferSize];		//array of fdsorigin
}

Cbuff::Cbuff(void) {
	fdsA=NULL;
	start = 0;
	end = -1;
	count = 0;
}


void Cbuff::place(int data,int origin) {				//0:worker || 1:client
	pthread_mutex_lock(&mtx);
	while (count >= bufferSize) {
		// cout << "Found Buffer Full" << endl;
		pthread_cond_wait(&cond_nonfull, &mtx);		//wait til there is room in buffer
	}
	end = (end + 1) % bufferSize;
	fdsA[end] = data;
	fdsAorigin[end] = origin;
	count++;
	pthread_mutex_unlock(&mtx);
}


void Cbuff::PrintData(void){
	for(int i=0;i<bufferSize;i++){
		cout<< fdsA[i] << endl;
	}
}

void Cbuff::optain(int & data, int & origin) {
	data = 0;
	pthread_mutex_lock(&mtx);
	while (count <= 0) {
		// cout << " Found Buffer Empty" <<endl;
		pthread_cond_wait(&cond_nonempty, &mtx);
		if (flag_int==1){
			pthread_mutex_unlock(&mtx);
			return;
		} 
	}
	data = fdsA[start];
	origin = fdsAorigin[start];
	start = (start + 1) % bufferSize;
	count--;
	pthread_mutex_unlock(&mtx);
	return ;
}

Cbuff::~Cbuff(void){
	delete[] fdsA;
	delete[] fdsAorigin;
}


/////////////////////////////////////////////////////////////////////

void ILNode::Print(void){
	cout << data << endl;
}



ILNode::ILNode(int _data){
	//cout << "Listnode constructor" << endl;
	data = _data;
 
	Next = NULL;
};

ILNode::~ILNode(void){}


///////////////////////////////////////////////LList//////////////////////////////////////////////////////////////////////

//list save fds


void IList::Print(void){
	ILNode * temp = ILHead;
	while (temp!=NULL){
		temp->Print();				//print listnode
		temp=temp->Next;
	}
}

void IList::Insert(int _data){
	ILNode*  newNode= new ILNode(_data);	//listonde memory allocation
	newNode->Next = ILHead;			//and list insertion
	ILHead = newNode;

}

ILNode * IList::Search(int _data){					//search for a node with specific id . return that node or NULL
	ILNode * temp = ILHead;
	while(temp!=NULL){
		if (temp->data == _data){
			return temp;
		}
		temp=temp->Next;
	}
	return NULL;
}






IList::IList(void):ILHead(NULL){
	//cout << "calling LList constructor" << endl;
}







IList::~IList(void){				//list constructor
	//cout << "deleting List" << endl; 
	ILNode* temp;
	while (ILHead!=NULL){
		temp=ILHead;
		ILHead=ILHead->Next;
		delete temp;			//call listonode destructor and free listnode
	}

}

 
