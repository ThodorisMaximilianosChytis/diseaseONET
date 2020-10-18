#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>


#include "customio.h"



using namespace std;




void Read(int &rfd,int bufferSize,string* message){


	char buff[bufferSize];
	int n;

//message ends at #
	
		message->assign("");
		do{
			if ( (n=read(rfd,buff,bufferSize))<0){
				cout << "rproblem" << endl;
				// exit(1);
			}
			buff[n]='\0';
			message->append(buff);
			
		}while(message->back()!='#');



}


void Write(int &wfd,int bufferSize,string* message){
	

	char buff[bufferSize];
	int n;


//wite #biffersize bytes at a time unti message is sent
	if (message->back()!='#')
		message->append("#");
	int length = message->length();
	for(int j=0;1;j++){
		strcpy(buff,message->substr(j*bufferSize,bufferSize).c_str());

		n=strlen(buff);	
		
		if(write(wfd,buff,n) !=n){
			//exit(1);
			cout << "wproblem" << endl;
		}
		
		length-=bufferSize;
		if (length<=0){
			break;
		}
	}

}




