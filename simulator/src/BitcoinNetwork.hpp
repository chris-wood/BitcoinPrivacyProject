#ifndef BITCOINNETWORK_HPP_
#define BITCOINNETWORK_HPP_


#include "ParameterList.hpp"
#include "Client.hpp"
#include "BitcoinSeller.hpp"
#include "Transaction.hpp"
#include "UnionFind/DisjointSets.hpp"
#include "Address.hpp"
//#include "UniversityModel/UniversityModelTimeStepping.hpp"

#include <queue>
#include <ctime>
#include <string>
#include <iostream>
#include <set>

class UniversityModelTimeStepping;

class BitcoinNetwork {
private:

	struct GA
	{
		GA():
			representativeAddress(0)
		{}
		Address representativeAddress;
		//unsigned int distinctLineIDcount;
		std::set<unsigned int> associatedTransactions;
		std::set<std::string> associatedAddresses;
	};

	std::map<unsigned int, GA> generalAddresses;

	//std::vector<char> testDBG;

	// network state
	//const unsigned int GENERATION;
	unsigned int numTransactions_;
	unsigned int numClients_;
	unsigned int lastNotAcceptedIndex_;
	unsigned long appearedTransactionCount_;
	unsigned long acceptedTransactionCount_;
	unsigned long countOfNoChangeTransactions_;
	unsigned long countOfSingleGenerationGAs_;
	unsigned long countOfSingleTransactionGAs_;
	btc minerReward_;
	double probForInternalMiner_;
	//unsigned int numClients_;
	//unsigned int numMiners_;
	double transactionInclusionProb_;
	std::vector<tm> blockTimeStamps_;
	std::vector<Client*> listOfNormalClients_;
	//std::vector<Client> listOfMiners_;
	std::vector<unsigned int> listOfMinerIDs_;
	std::vector<Transaction> listOfTransactions_;

	//DisjointSets<std::string> GAaddresses_;

	//void addBlockTimeStamp(tm &timestamp);
	//tm getTimeStampOf6thBlockInPast(void);
	//void acceptTransactions(void);

	void consolidateAddressesForTransaction(Transaction& transaction);
	void transferCoins(Transaction& transaction);

	void addClientTransactionsToGA(const Client& client, std::map<unsigned int, GA>& generalAddresses);
	std::string NetworkLogTransactions(const std::set<unsigned int>& transactionNumbersSet) const;
	std::string NetworkLogSingleTransaction(const Transaction & transaction) const;

	std::map<unsigned int, unsigned int> outputTransactionTracesForClient(std::ofstream& outputStream,
			unsigned int clientID,
			btc& maxAmount, bool printOnlyTransactionsFromSenders) const;
	void outputTransactionCollaborators(std::ofstream& outputStream, unsigned int clientID,
			const std::map<unsigned int, unsigned int>& transCount,
			const UniversityModelTimeStepping* univTimeStepping) const;

	Client& clientAt(unsigned int nr) {
		//unsigned int nr2 = nr-listOfNormalClients_.size();
		//return (nr < listOfNormalClients_.size()) ? listOfNormalClients_[nr] : listOfMiners_[nr2];
		return *(listOfNormalClients_.at(nr));
	}

	template <typename CLIENTTYPE>
	void addClient(const CLIENTTYPE& client)
	{
		listOfNormalClients_.push_back(new CLIENTTYPE(client));
	}

public:
	BitcoinNetwork();
	~BitcoinNetwork();
	void outputGroundTruth(std::ostream& outputStream, unsigned int clientNr) const;
	void outputGroundTruth(std::ostream& outputStream) const;
	void outputGroundTruth(const std::string& outputStr) const;

	void outputNetworkLog(const std::string& outputStr);
	void outputGrundTruthMapping(const std::string& outputStr, const UniversityModelTimeStepping* univTimeStepping = NULL) const;
	void outputTransactionTracesRandomClients(const std::string& fileName,
			const UniversityModelTimeStepping* univTimeStepping,  unsigned int numRandomClients) const;
	void outputTransactionTraces(const std::string& fileName, bool printOnlyTransactionsFromSenders) const;
	void outputFeatureOverview(const std::string& fileName) const;

	const Client& clientAt(unsigned int nr) const {
		//unsigned int nr2 = nr-listOfNormalClients_.size();
		//return (nr < listOfNormalClients_.size()) ? listOfNormalClients_[nr] : listOfMiners_[nr2];
		return *(listOfNormalClients_.at(nr));
	}

	unsigned int numClients(bool onlyCountAppearedOnes) const
	{
		if(!onlyCountAppearedOnes)
		{
			unsigned int size =  listOfNormalClients_.size();
			return size;
		}
		else
		{
			unsigned int count = 0;
			for(unsigned int i = 0; i < listOfNormalClients_.size(); i++)
			{
				if(listOfNormalClients_[i]->numAddressesSeen() > 0)
					count++;
			}
			return count;
		}

		//return listOfNormalClients_.size() + listOfMiners_.size();
	}
	unsigned int numMiners(bool onlyCountAppearedOnes) const
	{
		if(!onlyCountAppearedOnes)
			return listOfMinerIDs_.size();
		else
		{
			unsigned int count = 0;
			for(unsigned int i = 0; i < listOfMinerIDs_.size(); i++)
			{
				if(listOfNormalClients_[listOfMinerIDs_[i]]->numAddressesSeen() > 0)
					count++;
			}
			return count;
		}
		//return listOfMiners_.size();
	}
	unsigned int getMinerIDFromIndex(unsigned int posIndex) const
	{
		return listOfMinerIDs_[posIndex];
	}

	unsigned long numAddresses(bool onlyCountAppearedOnes) const
	{
		unsigned long ret = 0;
		for(unsigned int i = 0; i < listOfNormalClients_.size(); i++)
		{
			if(onlyCountAppearedOnes)
				ret += listOfNormalClients_[i]->numAddressesSeen();
			else
				ret += listOfNormalClients_[i]->numAddresses();
		}
		return ret;
	}

	unsigned int addAddressForClient(unsigned int client_nr);

	unsigned int numAddressesPerClient(unsigned int client_nr) const { return clientAt(client_nr).numAddresses(); }
	btc getBalanceForClient(unsigned int client_nr) const { return clientAt(client_nr).get_totalBalance(); }

	std::vector<unsigned int> addressesClientWithAmount(unsigned int client_r, btc amount_r);
	std::vector<unsigned int> friendsOfClient(unsigned int client_nr);

	// Bitcoin Network functionalities as described in the diagrams
	unsigned int generateBlock(const tm &timestamp, unsigned int miner); //miner receives 50 coins

	unsigned int generateNewClient(bool isMiner, unsigned int profileID, int numInitialAddresses, const std::vector<unsigned int>& friends, btc initialCoins);
	// sample addresses for new client, timestamp not needed, since this does not appear in logs
	unsigned int generateNewBitcoinSeller(unsigned int profileID, int numInitialAddresses);

	//void changeAddress(tm &timestamp, unsigned int client, unsigned int affectedAddress); // sample a new address for the affected one
	unsigned int issueTransaction(const tm &timestamp, unsigned int sender, unsigned int receiver,
			/*const std::vector<unsigned int>& senderAddresses*/ unsigned int receiverAddress, btc amount, int eventID);

	//void testUnionDBG(unsigned int transactionID, tm& tmstp);

	std::ostream& outputStatistics(std::ostream& o) const;

};

extern std::ostream& operator<<(std::ostream& o, const BitcoinNetwork& network);

#endif /* BITCOINNETWORK_HPP_ */
