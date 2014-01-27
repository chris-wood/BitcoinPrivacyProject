#include <vector>
#include <time.h>
#include <string>
using namespace std;

struct Transaction{
	vector<string>sender;
	tm timestamp;
	vector<double> amountIN;
	vector<double> amountOUT;
	vector<string> recipient;
	vector<int> collaboratorID;
	vector<int> flagCollaborator;
};

struct GA{
vector<string> ListAddresses;
vector<Transaction> ListTransactions;
vector<int> Friends;
vector<double> balance;
int realID;
};


