
#include <string>
#include <iostream>

#include "cbuffer.h"

using namespace std;


void SendError(IList*,int ,int ,string& ,int&);



void diseaseFrequency(IList*,string,string,string,string,string,int,int,string&,int&);

void searchPatientRecord(IList*,string ,int,int,string&,int&);

// void listCountries(Child**,int,int,int&);

void UserQueries(string,IList*,int,int,string&);

int DateCompareEarlier(string*,string*);


// void exit(IList*,int,int,int&);

void TopKAgeRanges(IList*,int,string,string,string,string,int,int,string&,int&);

void sig_handler(int signo);


char* GetHostAddress(void);