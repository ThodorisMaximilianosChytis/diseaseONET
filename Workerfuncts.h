
#include "WorkerDS.h"
#include "masterClasses.h"
#include "customio.h"


extern volatile sig_atomic_t wflag_int_q;


extern volatile sig_atomic_t flag_usr1;

// extern int wflag_int_q;

// void LoadNewFile(Hashtable*,LList*,SSList*,char*);


void QueriesAnswer(LList* ,Hashtable *,SSList *,int &,int,int&,int&,char*);


void CreateLogfile(SSList*,int ,int );


void InsertChildFileRecords(LList*, Hashtable *, SSLNode* ,SSList* ,char*);


char* GetHostAddress(void);


void sig_handler_worker(int signo);




// void Write(int &,int ,string* );

// void Read(int &,int ,string* );
