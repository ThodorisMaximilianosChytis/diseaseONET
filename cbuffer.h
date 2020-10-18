#include <string>
#include <iostream>

#include "customio.h"


using namespace std;


extern volatile sig_atomic_t flag_int;
extern pthread_mutex_t mtx;
extern pthread_cond_t cond_nonempty;
extern pthread_cond_t cond_nonfull;




class Cbuff{
	
	public:
		int bufferSize;
		int* fdsA;
		int* fdsAorigin;
		int start;
		int end;
		int count;
		Cbuff();
		~Cbuff();
		void CreateDataArray(int);
		void place(int,int);
		void optain(int& , int& );
		void PrintData(void);

};


class ILNode{

	public:
		int data;
		ILNode* Next;
		ILNode(int);
		~ILNode(void);
		void Print(void);

};

class IList{
	
	public:
		ILNode* ILHead;
		IList();
		~IList();
		void Insert(int);
		//LList* Remove();
		ILNode* Search(int);
		void Print();
		// void Sort(int);
};




// void Write(int &,int ,string* );

// void Read(int &,int ,string* );

// void SendError(IList*,int ,int ,string& ,int&);

// // void ReConstructNewWorker();

// void diseaseFrequency(IList*,string,string,string,string,string,int,int,string&,int&);

// void searchPatientRecord(IList*,string ,int,int,string&,int&);

// // void listCountries(Child**,int,int,int&);

// void UserQueries(string,IList*,int,int,string&);

// int DateCompareEarlier(string*,string*);


// // void exit(IList*,int,int,int&);

// void TopKAgeRanges(IList*,int,string,string,string,string,int,int,string&,int&);

// void sig_handler(int signo);


// char* GetHostAddress(void);
