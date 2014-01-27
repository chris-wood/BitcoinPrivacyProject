#include "main.hpp"
#include "BitcoinNetwork.hpp"
#include "ParameterList.hpp"
#include "Client.hpp"
#include "Transaction.hpp"
#include "Sampler.hpp"
#include "UniversityModel/UniversityModel.hpp"
#include "UniversityModel/UniversityModelTimeStepping.hpp"

#include "hist/StatCalc.h"

#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cassert>
#include <limits>
#include <map>

const unsigned int GENERATION = std::numeric_limits<unsigned int>::max();
const unsigned int GAID_NOTSET = std::numeric_limits<unsigned int>::max()-1;

const btc ONECENT = 1;

BitcoinNetwork::BitcoinNetwork():
	//testDBG(5478954, 1),
	numTransactions_(0),
	numClients_(0),
	lastNotAcceptedIndex_(0),
	appearedTransactionCount_(0),
	acceptedTransactionCount_(0),
	countOfNoChangeTransactions_(0),
	countOfSingleGenerationGAs_(0),
	countOfSingleTransactionGAs_(0),
	minerReward_(ParameterList::getParameterByName<btc>("minerReward")),
	probForInternalMiner_(1.0)
	//numClients_(0),
	//numMiners_(0)
{
	if(ParameterList::parameterExists("probForInternalMiner"))
	{
		probForInternalMiner_ = ParameterList::getParameterByName<double>("probForInternalMiner");
	}

	transactionInclusionProb_ = 0.9;
	if(ParameterList::parameterExists("transactionInclusionProb"))
	{
		ParameterList::getParameterByName("transactionInclusionProb", transactionInclusionProb_);
		//transactionInclusionProb_ = ParameterList::getParameterByName<int>("transactionInclusionProb");
	}
}

BitcoinNetwork::~BitcoinNetwork() {
	for(unsigned int i = 0; i < listOfNormalClients_.size(); ++i) delete listOfNormalClients_[i];
}

// returns id of newly created client
unsigned int BitcoinNetwork::generateNewClient(bool isMiner, unsigned int profileID, int numInitialAddresses,
		 const std::vector<unsigned int>& friends, btc initialCoins)
{
	numClients_++;
	unsigned int curClientNum = numClients(false);
	Client new_client(isMiner, curClientNum, profileID, numInitialAddresses, friends, initialCoins);
	addClient<Client>(new_client);

	if(isMiner) listOfMinerIDs_.push_back(curClientNum);

	return curClientNum;
}

unsigned int BitcoinNetwork::generateNewBitcoinSeller(unsigned int profileID, int numInitialAddresses)
{
	numClients_++;
	unsigned int curClientNum = numClients(false);
	BitcoinSeller new_bitcoinSeller(curClientNum, profileID, numInitialAddresses);
	addClient<BitcoinSeller>(new_bitcoinSeller);

	return curClientNum;
}


void BitcoinNetwork::outputGroundTruth(std::ostream& outputStream, unsigned int clientNr) const
{
	const Client& client = clientAt(clientNr);
	outputStream << "Client " << clientNr << std::endl;
	outputStream << "\tid:" << client.id_ << ", ";
	outputStream << "miner:" << client.miner_ << ", ";
	outputStream << "profileID:" << client.profileID_ << ", ";
	outputStream << "initialAmount:" << Str(client.initialAmount_) << ", ";
	outputStream << "initialAddressesCount:" << client.initialAddressesCount_ << ", ";
	outputStream << "initialCoinCount:" << client.initialCoinCount_ << ", ";
	outputStream << "numGAs:" << client.numGAs() << ", ";
	outputStream << "numAddresses:" << client.setOfAddresses_.NumElements() << ", ";
	outputStream << "numAddressesAppeared:" << client.numAddressesSeen() << std::endl;
	outputStream << "\tcurrentBalance: " << Str(client.get_totalBalance()) << ", ";
	outputStream << "outstandingChangeMoney: " << Str(client.outstandingChangeMoney_) << std::endl;
	outputStream << "\tAddresses:" << std::endl;
	for(int p = 0; p < client.setOfAddresses_.size(); ++p)
	{
		const Address& clientAddr = client.setOfAddresses_.getElement(p);
		outputStream << "\t\t" << clientAddr.toString() << " -> ";
		outputStream << client.setOfAddresses_.getRepresentativeReference(clientAddr).get_hashString();
		outputStream << " -> ";
		unsigned int gaid = client.setOfAddresses_.getRepresentativeReference(clientAddr).get_gaid();
		if(GAID_NOTSET == gaid) outputStream << "not_set";
		else outputStream << gaid;
		outputStream << std::endl;
	}
	outputStream << "\tFriends:" << std::endl << "\t\t";
	for(unsigned int p = 0; p < client.listOfFriends_.size(); ++p)
	{
		outputStream << client.listOfFriends_[p] << ", ";
	}
	outputStream << std::endl;
	outputStream << "\tTransactions as Sender:" << std::endl;
	for(unsigned int p = 0; p < client.listOfTransactionsAsSender_.size(); ++p)
	{
		unsigned int transactionNum = client.listOfTransactionsAsSender_[p];
		outputStream << listOfTransactions_[transactionNum].toString("\t\t") << std::endl;
	}
	outputStream << "\tTransactions as Receiver:" << std::endl;
	for(unsigned int p = 0; p < client.listOfTransactionsAsReceiver_.size(); ++p)
	{
		unsigned int transactionNum = client.listOfTransactionsAsReceiver_[p];
		outputStream << listOfTransactions_[transactionNum].toString("\t\t") << std::endl;
	}
	outputStream << std::endl;

	//DEBUG:
	//sanity check:
	for(unsigned int i = 0; i < listOfTransactions_.size(); ++i) assert(listOfTransactions_[i].get_id() == i);
	for(unsigned int i = 0; i < listOfNormalClients_.size(); ++i) assert(listOfNormalClients_[i]->get_id() == i);
	assert(listOfTransactions_.size() == numTransactions_);
	assert(listOfNormalClients_.size() == numClients_);
	//////////
}

void BitcoinNetwork::outputGroundTruth(std::ostream& outputStream) const
{
	for(unsigned int i = 0; i < numClients(false); ++i)
	{
		outputGroundTruth(outputStream, i);
	}
}

void BitcoinNetwork::outputGroundTruth(const std::string& fileName) const
{
	std::ofstream outputStream;
	outputStream.open(fileName.c_str());
	outputGroundTruth(outputStream);
	outputStream.close();
}


std::map<unsigned int, unsigned int> BitcoinNetwork::outputTransactionTracesForClient(std::ofstream& outputStream,
		unsigned int clientID,
		btc& maxAmount,
		bool printOnlyTransactionsFromSenders) const
{
	std::map<unsigned int, unsigned int> res;

	maxAmount = 0;

	const Client& client = clientAt(clientID);

	assert(client.id_ == clientID);

	unsigned int transactionsFromClient = client.listOfTransactionsAsSender_.size();
	for(unsigned int p = 0; p < transactionsFromClient; ++p)
	{
		unsigned int transactionNum = client.listOfTransactionsAsSender_.at(p);
		const Transaction& curTransaction = listOfTransactions_.at(transactionNum);

		assert(curTransaction.get_senderID() == clientID);

		// only print transactions that have appeared in the network
		if(!curTransaction.get_appeared()) continue;

		unsigned int receiverID = curTransaction.get_receiverID();

		if(res.count(receiverID) == 0) res[receiverID] = 0;
		res[receiverID] += 1;

		const Address& firstSenderAddress = curTransaction.get_senders()[0];
		const Address& representativeAddress = client.setOfAddresses_.getRepresentativeReference(firstSenderAddress);
		unsigned int GAID = representativeAddress.get_gaid();
		assert(GAID != GAID_NOTSET);
		btc transactionAmount = curTransaction.get_recipient().get_totalAmount();
		outputStream << GAID << ", " << client.id_ << ", " << client.profileID_ << ", "
				     << transactionsFromClient << ", "
				     << Str(transactionAmount) << ", "
				     << Sampler::tmToString(curTransaction.get_appearedTimestamp(), true) << ", "
				     << Sampler::secondsFromMidnight(curTransaction.get_appearedTimestamp()) << ", "
				     << curTransaction.get_eventID() << ", "
				     << "S"
				     << std::endl;

		if(transactionAmount >= maxAmount) maxAmount = transactionAmount;
	}

	if(!printOnlyTransactionsFromSenders) // also include the transactions which the users receives
	{
		unsigned int transactionsToClient = client.listOfTransactionsAsReceiver_.size();
		for(unsigned int p = 0; p < transactionsToClient; ++p)
		{
			unsigned int transactionNum = client.listOfTransactionsAsReceiver_.at(p);
			const Transaction& curTransaction = listOfTransactions_.at(transactionNum);

			assert(curTransaction.get_receiverID() == clientID);

			// only print transactions that have appeared in the network
			if(!curTransaction.get_appeared()) continue;

			const Address& receiverAddress = curTransaction.get_recipient();
			const Address& representativeAddress = client.setOfAddresses_.getRepresentativeReference(receiverAddress);
			unsigned int GAID = representativeAddress.get_gaid();
			assert(GAID != GAID_NOTSET);
			btc transactionAmount = curTransaction.get_recipient().get_totalAmount();
			outputStream << GAID << ", " << client.id_ << ", " << client.profileID_ << ", "
						 << transactionsFromClient << ", "
						 << Str(transactionAmount) << ", "
						 << Sampler::tmToString(curTransaction.get_appearedTimestamp(), true) << ", "
						 << Sampler::secondsFromMidnight(curTransaction.get_appearedTimestamp()) << ", "
						 << curTransaction.get_eventID() << ", "
						 << "R"
						 << std::endl;
		}
	}

	return res;
}

void BitcoinNetwork::outputTransactionTraces(const std::string& fileName, bool printOnlyTransactionsFromSenders) const
{
	std::ofstream outputStream;
	outputStream.open(fileName.c_str());

	outputStream << "# ga_id, client_id, profile_id, trans_count, trans_amount, trans_daytime, trans_secsFromMidnight, event_id, senderOrReceiver" << std::endl;

	btc maxAmount = 0;

	for(unsigned int i = 0; i < numClients(false); ++i)
	{
		/*
		// make sure we're dealing with a "normal" client, not a Bitcoin seller or a store.
		if(UniversityModelConfig::getInstance().profiles.count(clientAt(i).profileID_) <= 0)
		{
			continue;
		}
		*/
		outputTransactionTracesForClient(outputStream, i, maxAmount, printOnlyTransactionsFromSenders);
	}
	outputStream << std::endl;
	outputStream.close();
}


void BitcoinNetwork::outputTransactionTracesRandomClients(const std::string& fileName,
		const UniversityModelTimeStepping* univTimeStepping, unsigned int numRandomClients) const
{
	// this only works if we are using the "University model"
	if(!ParameterList::parameterExists("model") ||
			ParameterList::getParameterByName<std::string>("model") != "UniversityModel")
	{
		std::cout << "User Traces are not printed to file because the current simulation is not based on the University model." << std::endl;
		return;
	}

	const std::map<unsigned int, std::vector<unsigned int> >& profileToClientLinks = univTimeStepping->getProfileToClientsMapping();

	for(std::map<unsigned int, std::vector<unsigned int> >::const_iterator it = profileToClientLinks.begin();
			it != profileToClientLinks.end(); it++)
	{
		// make sure we're dealing with a "normal" client, not a Bitcoin seller or a store.
		/*
		if(UniversityModelConfig::getInstance().profiles.count(it->first) <= 0)
		{
			continue;
		}
		*/

		std::vector<unsigned int> randPositions = Sampler::randVectPermutation(0, it->second.size()-1, numRandomClients);
		string fileNameProfile = fileName + "_profile_" + Str(it->first) + ".txt";
		std::ofstream outputStreamProfile;
		outputStreamProfile.open(fileNameProfile.c_str());

		for(unsigned int z = 0; z < randPositions.size(); ++z)
		{
			// output first block
			unsigned int randPos = randPositions[z];
			unsigned int clientPos = it->second[randPos];
			btc maxAmount = 0;
			std::map<unsigned int, unsigned int> transCount =
					outputTransactionTracesForClient(outputStreamProfile, clientPos, maxAmount, true);
			outputStreamProfile << std::endl << std::endl;


			// output second block
			const Client& client = clientAt(clientPos);
			const UniversityModelClient& umc = univTimeStepping->getClientRepresentation(client.id_);
			assert(client.id_ == umc.get_ID());
			assert(client.id_ == clientPos);

			umc.plotOutput(outputStreamProfile, maxAmount);
			outputStreamProfile << std::endl << std::endl;

			// output third block
			outputTransactionCollaborators(outputStreamProfile, clientPos, transCount, univTimeStepping);
			outputStreamProfile << std::endl << std::endl;
		}
		outputStreamProfile.close();
	}
}

void BitcoinNetwork::outputTransactionCollaborators(std::ofstream& outputStream, unsigned int clientID,
		const std::map<unsigned int, unsigned int>& transCounts,
		const UniversityModelTimeStepping* univTimeStepping) const
{
	unsigned int totalNrDescr = UniversityModelConfig::getInstance().stores.size() + UniversityModelConfig::getInstance().profiles.size() + 1;
	std::vector<bool> done(totalNrDescr, false);

	for(std::map<unsigned int, unsigned int>::const_iterator trit = transCounts.begin(); trit != transCounts.end(); ++trit)
	{
		unsigned int receiverID = trit->first;
		unsigned int transactionCount = trit->second;
		const UniversityModelClient& umcReceiver = univTimeStepping->getClientRepresentation(receiverID);
		string receiverName;
		int receiverDescr = 0;
		unsigned int color = 0;
		if(receiverID == clientID)
		{
			receiverName = "self";
			receiverDescr = totalNrDescr - 1;
			color = 0;
		}
		else if(umcReceiver.get_isSeller())
		{
			receiverName = UniversityModelConfig::getInstance().stores.at(umcReceiver.get_storeID()).name;
			receiverDescr = umcReceiver.get_storeID();
			color = UniversityModelConfig::getInstance().stores.at(umcReceiver.get_storeID()).eventid + 1;
		}
		else
		{
			receiverName = UniversityModelConfig::getInstance().profiles.at(umcReceiver.get_profileID()).name;
			receiverDescr = UniversityModelConfig::getInstance().stores.size() + umcReceiver.get_profileID();
			color = 0;
		}

		outputStream << receiverDescr << ", " << receiverID << ", " << receiverName << ", " << transactionCount << ", " << color << std::endl;
		done[receiverDescr] = true;
	}

	for(unsigned int k = 0; k < done.size(); ++k)
	{
		if(!done[k])
		{
			if(k == (totalNrDescr-1))
			{
				outputStream << k << ", " << -1 << ", " << "self" << ", " << 0 << ", " << 0 << std::endl;
			}
			else if(k < UniversityModelConfig::getInstance().stores.size())
			{
				string receiverName = UniversityModelConfig::getInstance().stores.at(k).name;
				unsigned int color = UniversityModelConfig::getInstance().stores.at(k).eventid + 1;
				outputStream << k << ", " << -1 << ", " << receiverName << ", " << 0 << ", " << color << std::endl;
			}
			else
			{
				unsigned int z = k - UniversityModelConfig::getInstance().stores.size();
				string receiverName = UniversityModelConfig::getInstance().profiles.at(z).name;
				outputStream << k << ", " << -1 << ", " << receiverName << ", " << 0 << ", " << 0 << std::endl;
			}
		}
	}
}

void BitcoinNetwork::outputFeatureOverview(const std::string& fileName) const
{
	unsigned int binSizeSecs = 15*60;
	btc binSizeBTC = 1000; // 10.00 BTC

	std::vector<unsigned int> timeCounts;
	std::vector<unsigned int> amountCounts;

	std::ofstream outputStream;
	outputStream.open(fileName.c_str());

	for(unsigned int i = 0; i < numClients(false); ++i)
	{
		std::vector<bool> addedTimeCount(timeCounts.size(), false);
		std::vector<bool> addedAmountCount(amountCounts.size(), false);

		const Client& client = clientAt(i);
		unsigned int transactionsFromClient = client.listOfTransactionsAsSender_.size();
		for(unsigned int p = 0; p < transactionsFromClient; ++p)
		{
			unsigned int transactionNum = client.listOfTransactionsAsSender_[p];
			const Transaction& curTransaction = listOfTransactions_[transactionNum];
			if(curTransaction.get_appeared())
			{
				unsigned int secsFromMidnight = Sampler::secondsFromMidnight(curTransaction.get_appearedTimestamp());
				unsigned int transactionAmount = curTransaction.get_recipient().get_totalAmount();

				unsigned int binSecs = secsFromMidnight/binSizeSecs;
				unsigned int binAmount = transactionAmount/binSizeBTC;


				if(binSecs >= addedTimeCount.size())
				{
					addedTimeCount.resize(binSecs+1, false);
					timeCounts.resize(binSecs+1, 0);
				}
				if(binAmount >= addedAmountCount.size())
				{
					addedAmountCount.resize(binAmount+1, false);
					amountCounts.resize(binAmount+1, 0);
				}


				if(!addedTimeCount[binSecs])
				{
					addedTimeCount[binSecs] = true;
					timeCounts[binSecs] += 1;
				}
				if(!addedAmountCount[binAmount])
				{
					addedAmountCount[binAmount] = true;
					amountCounts[binAmount] += 1;
				}
			}
		}
	}

	for(unsigned p = 0; p < amountCounts.size(); ++p)
	{
		outputStream << p << ", " << Str(binSizeBTC*p) << ", " << Str(binSizeBTC*(p+1)) << ", " << amountCounts[p] << std::endl;
	}
	outputStream << std::endl << std::endl;


	for(unsigned p = 0; p < timeCounts.size(); ++p)
	{
		outputStream << p << ", " << binSizeSecs*p << ", " << binSizeSecs*(p+1) << ", " << timeCounts[p] << std::endl;
	}


	outputStream << std::endl;
	outputStream.close();
}

void BitcoinNetwork::outputGrundTruthMapping(const std::string& fileName, const UniversityModelTimeStepping* univTimeStepping) const
{
	bool writeStoreID = false;
	if(univTimeStepping != NULL)
	{
		writeStoreID = true;
	}

	std::ofstream outputStream;
	outputStream.open(fileName.c_str());

	Histogram1D numTransactionPerClientHist("numTransactionPerClientHist", 50, false, true, 1, 601);

	outputStream << "# client_id, ga_id, address, profile_id, ga_addressCount, ga_transactionCount";
	if(writeStoreID) outputStream << ", storeID";
	outputStream << std::endl;

	unsigned int clientSeenCount = 0;
	for(unsigned int i = 0; i < numClients(false); ++i)
	{
		const Client& client = clientAt(i);
		numTransactionPerClientHist.addNewValue(client.listOfTransactionsAsSender_.size());
		assert(i == client.id_);
		int storeID = -1;
		if(writeStoreID && UniversityModelConfig::getInstance().sellerProfiles.count(client.profileID_) > 0) // check if indeed is a store
		{
			storeID = univTimeStepping->getClientRepresentation(client.id_).get_storeID();
		}
		bool clientSeen = false;
		for(int p = 0; p < client.setOfAddresses_.size(); ++p) // loop through all addresses
		{
			const Address& curAddr = client.setOfAddresses_.getElement(p);
			if(curAddr.hasAppeared())
			{
				clientSeen = true;
				const Address& reprAddr = client.setOfAddresses_.getRepresentativeReference(curAddr);
				unsigned int gaid = reprAddr.get_gaid();
				assert(gaid != GAID_NOTSET);
				const GA& assocGA = generalAddresses.at(gaid);
				outputStream << client.id_ << "," << gaid << ","
						     << curAddr.get_hashString() << "," << client.profileID_ << ","
						     << assocGA.associatedAddresses.size() << "," << assocGA.associatedTransactions.size();
							 if(writeStoreID) outputStream << "," << storeID;
				outputStream << std::endl;

			}
		}
		if(clientSeen) clientSeenCount++;
	}

	assert(clientSeenCount == numClients(true));

	(fileName+"clientTransHist.txt") << numTransactionPerClientHist;

	outputStream << std::endl;
	outputStream.close();
}

void BitcoinNetwork::addClientTransactionsToGA(const Client& client,
		std::map<unsigned int, GA>& generalAddresses)
{
	for(unsigned int p = 0; p < client.listOfTransactionsAsSender_.size(); ++p)
	{
		unsigned int transactionNum = client.listOfTransactionsAsSender_[p];
		const Transaction& curTransaction = listOfTransactions_[transactionNum];
		assert(curTransaction.get_senderID() != GENERATION);

		if(curTransaction.get_appeared())
		//if(curTransaction.get_accepted())
		{
			//for the sender address
			const Address& firstSenderAddr = curTransaction.get_senders()[0];
			const Address senderRepr = client.setOfAddresses_.getRepresentative(firstSenderAddr);
			assert(client.setOfAddresses_.Find(firstSenderAddr).hasAppeared());
			assert(client.setOfAddresses_.Find(senderRepr).hasAppeared());
			unsigned int gaIDforSender = senderRepr.get_gaid();
			GA& ga = generalAddresses.at(gaIDforSender);
			ga.associatedTransactions.insert(transactionNum);

			//for the shadow addresss
			if(curTransaction.get_shadowAddrNeeded())
			{
				const Address& shadowAddr = curTransaction.get_shadow();

				/*// DEBUG
				if(!client.setOfAddresses_.elementExists(shadowAddr))
				{
					std::cout << std::endl << "=========================" << std::endl << std::endl;
					std::cout << curTransaction.toString("\t") << std::endl;
					std::cout << client.setOfAddresses_.getUnderlyingVector() << std::endl;
					std::cout << std::endl << "=========================" << std::endl << std::endl;
				}
				*/

				const Address shadowRepr = client.setOfAddresses_.getRepresentative(shadowAddr);
				assert(client.setOfAddresses_.Find(shadowAddr).hasAppeared());
				assert(client.setOfAddresses_.Find(shadowRepr).hasAppeared());
				unsigned int gaIDforShadow = shadowRepr.get_gaid();
				GA& gaShadow = generalAddresses.at(gaIDforShadow);
				gaShadow.associatedTransactions.insert(transactionNum);
			}
		}
	}

	// note: generation reward transactions can only occur in listOfTransactionsAsReceiver_
	for(unsigned int p = 0; p < client.listOfTransactionsAsReceiver_.size(); ++p)
	{
		unsigned int transactionNum = client.listOfTransactionsAsReceiver_[p];
		Transaction& curTransaction = listOfTransactions_[transactionNum];

		if(curTransaction.get_appeared())
		{
			const Address& receiverAddr = curTransaction.get_recipient();
			Address receiverRepr = client.setOfAddresses_.getRepresentative(receiverAddr);
			assert(client.setOfAddresses_.Find(receiverAddr).hasAppeared());
			assert(client.setOfAddresses_.Find(receiverRepr).hasAppeared());
			unsigned int gaIDforRepr = receiverRepr.get_gaid();
			GA& ga = generalAddresses.at(gaIDforRepr);
			ga.associatedTransactions.insert(transactionNum);
		}
	}
}

void BitcoinNetwork::outputNetworkLog(const std::string& outputStr)
{
	std::ofstream outputStream;
	outputStream.open(outputStr.c_str());

	//std::map<unsigned int, GA> generalAddresses; // TODO: make vector instead of map
	//std::vector<long> fdsa(48945895);

	unsigned int GAcounter = 0;

	// consolidation for each client
	for(unsigned int i = 0; i < numClients(false); ++i)
	{
		Client& client = clientAt(i);
		assert(client.id_ == i);

		unsigned int nrGAsForCurClient = 0;
		// create all the GAs belonging to client i
		for(int z = 0; z < client.setOfAddresses_.size(); ++z)
		{
			const Address& curAddr = client.setOfAddresses_.getElement(z);

			if(curAddr.hasAppeared())
			{
				Address& reprAddr = client.setOfAddresses_.getRepresentativeReference(curAddr);

				if(generalAddresses.count(reprAddr.get_gaid()) > 0)
				{
					GA& existingGA = generalAddresses[reprAddr.get_gaid()];
					if(existingGA.associatedAddresses.count(curAddr.get_hashString()) <= 0)
					{
						existingGA.associatedAddresses.insert(curAddr.get_hashString());
						//existingGA.numAddresses++;
					}
				}
				else // create new GA
				{
					nrGAsForCurClient++;
					GA ga;
					reprAddr.set_gaid(GAcounter); // set the GA ID for this address -> this address represents the GA just created.
					//ga.id = GAcounter;
					ga.representativeAddress = reprAddr;
					//ga.distinctLineIDcount = 0;
					//ga.numAddresses = 1; // one address at the beginning
					//ga.reprAddress = reprAddr.get_hashString();
					ga.associatedAddresses.insert(curAddr.get_hashString());
					//store
					generalAddresses[reprAddr.get_gaid()] = ga;
					GAcounter++;
				}
			}
		}

		//outputGroundTruth(std::cout, client.id_);
		//std::cout << std::endl <<  "fdsafdsaf";
		// DEBUG //
		unsigned int clientGAs = client.numGAs();
		assert(nrGAsForCurClient == clientGAs);
		///////////

		addClientTransactionsToGA(client, generalAddresses);
	}



	// output step for each GA
	for(std::map<unsigned int, GA>::const_iterator it = generalAddresses.begin(); it != generalAddresses.end(); ++it)
	{
		const GA& curGA = it->second;
		//assert(it->first == curGA.id);
		assert(it->first == curGA.representativeAddress.get_gaid());

		outputStream << "GA " << curGA.representativeAddress.get_gaid() << std::endl; //" (" <<  curGA.reprAddress  << ")" << std::endl;

		Client& client = clientAt(curGA.representativeAddress.get_owner());
		//////////DEBUG
		int size = (int)curGA.associatedAddresses.size();
		int setSize = client.setOfAddresses_.NumSetElements(curGA.representativeAddress);

		/*
		if(size != setSize)
		{
			std::cout << curGA.associatedAddresses.size() << " " << curGA.associatedAddresses << std::endl;
			std::cout << client.setOfAddresses_.size() << " " << client.setOfAddresses_.getUnderlyingVector() << std::endl;
			outputGroundTruth(std::cout, client.id_);
		}
		*/
		assert(size == setSize);
		//////////////

		outputStream << "Distinct ID " << curGA.associatedAddresses.size();
		for(std::set<std::string>::const_iterator setIt = curGA.associatedAddresses.begin(); setIt != curGA.associatedAddresses.end(); ++setIt)
		{
			outputStream << " " << *setIt;
		}
		outputStream << std::endl;
		outputStream << "DistinctLineID " << curGA.associatedTransactions.size() << std::endl;

		assert(curGA.associatedTransactions.size() > 0); // no empty GAs!

		outputStream << NetworkLogTransactions(curGA.associatedTransactions);

		if(curGA.associatedTransactions.size() == 1)
		{
			countOfSingleTransactionGAs_++;
			unsigned int transactionNr = *(curGA.associatedTransactions.begin());
			const Transaction& singleTransaction = listOfTransactions_[transactionNr];
			if(singleTransaction.get_senderID() == GENERATION)
			{
				countOfSingleGenerationGAs_++;
			}
		}
	}



	outputStream << std::endl;
	outputStream.close();
	
	// DEBUG //
	unsigned int DBGtotNrGAs = 0;
	for(unsigned int i = 0; i < numClients(false); ++i)
		DBGtotNrGAs += clientAt(i).numGAs();
	
	assert(generalAddresses.size() == DBGtotNrGAs);
	//////////
}

std::string BitcoinNetwork::NetworkLogTransactions(const std::set<unsigned int>& transactionNumbersSet) const
{
	std::stringstream out;
	for(std::set<unsigned int>::const_iterator setIt = transactionNumbersSet.begin(); setIt != transactionNumbersSet.end(); ++setIt)
	{
		const Transaction& curTransaction = listOfTransactions_[*setIt];
		out << NetworkLogSingleTransaction(curTransaction) << std::endl;
	}
	return out.str();
}

std::string BitcoinNetwork::NetworkLogSingleTransaction(const Transaction & transaction) const
{
	std::stringstream out;

	unsigned int receiverID = transaction.get_receiverID();
	unsigned int senderID = transaction.get_senderID();
	const Client& receiverC = clientAt(receiverID);

	unsigned int receiverGAID = receiverC.setOfAddresses_.getRepresentativeReference(transaction.get_recipient()).get_gaid();
	assert(receiverGAID != GAID_NOTSET);

	if(senderID == GENERATION)
	{
			out  << "{ " << Sampler::tmToLogFormat(transaction.get_appearedTimestamp())
				 << " 1 1  " << Str(transaction.get_recipient().get_totalAmount()) << "  -1  "
				 << receiverGAID
				 << " }";
	}
	else
	{
		const Client& senderC = clientAt(senderID);

		unsigned int senderGAID = senderC.setOfAddresses_.getRepresentativeReference(transaction.get_senders()[0]).get_gaid();
		assert(senderGAID != GAID_NOTSET);

		out << "{ " << Sampler::tmToLogFormat(transaction.get_appearedTimestamp());
		out << " 1 "; // we do not need to know how many addresses the client used for sending the BTCs
		//out << " " << transaction.get_senders().size() << " ";
		if(transaction.get_shadowAddrNeeded())
		{
			unsigned int shadowGAID = senderC.setOfAddresses_.getRepresentativeReference(transaction.get_shadow()).get_gaid();
			assert(shadowGAID != GAID_NOTSET);

			out  << "2 " << Str(transaction.get_recipient().get_totalAmount())
				 << " " << Str(transaction.get_shadow().get_totalAmount())
				 << " " << senderGAID
				 << " " << receiverGAID
				 << " " << shadowGAID;
		}
		else
		{
			out  << "1 " << Str(transaction.get_recipient().get_totalAmount())
				 << " " << senderGAID
				 << " " << receiverGAID;
		}
		out << " }";
	}

	return out.str();
}

// choses which addresses are sampled from
std::vector<unsigned int> BitcoinNetwork::addressesClientWithAmount(unsigned int client_r, btc amount_r)
{
	std::vector<unsigned int> addresses;
	Client& client = clientAt(client_r);
	btc sum = 0;
	for(int i = 0; i < client.setOfAddresses_.size(); ++i)
	{
		btc amountFromAddress = client.setOfAddresses_[i].get_totalAmount();
		sum += amountFromAddress;
		if(amountFromAddress > 0) addresses.push_back(i);
		if (sum >= amount_r) break;
	}
	if (sum < amount_r) addresses.clear();

	return addresses;
}

std::vector<unsigned int> BitcoinNetwork::friendsOfClient(unsigned int client_nr)
{
	Client& client = clientAt(client_nr);
	return client.listOfFriends_;
}

unsigned int BitcoinNetwork::addAddressForClient(unsigned int client_nr)
{
	Client& client = clientAt(client_nr);
	Address newAddr(0, client.id_);
	int addrPos = client.setOfAddresses_.addElement(newAddr);
	return addrPos;
}

unsigned int BitcoinNetwork::generateBlock(const tm &timestamp, unsigned int miner) {
	blockTimeStamps_.push_back(timestamp);
	unsigned long i;
	unsigned int includedTransactionCount = 0;
	bool firstEncounter = true;
	for(i = lastNotAcceptedIndex_; i < listOfTransactions_.size(); i++)
	{
		Transaction& currentTransaction = listOfTransactions_[i];
		if(!currentTransaction.get_accepted())
		{
			if(!currentTransaction.get_appeared() && Sampler::rand01() < transactionInclusionProb_)
			{
				// the transaction is included in the log for the first time
				currentTransaction.set_transactionOrderPos(appearedTransactionCount_);
				currentTransaction.increaseBlockInclusionCount(timestamp);
				appearedTransactionCount_++;
				includedTransactionCount++;
				// update the GAs
				consolidateAddressesForTransaction(currentTransaction);
			}
			else if(currentTransaction.get_appeared()) // transaction has been seen before
			{
				currentTransaction.increaseBlockInclusionCount(timestamp);
			}

			if(firstEncounter && !currentTransaction.get_accepted()) // still not accepted
			{
				firstEncounter = false;
				lastNotAcceptedIndex_ = i;
			}
			else if(currentTransaction.get_accepted())
			// transaction is now accepted. we now 'transfer' the money
			{
				acceptedTransactionCount_++;
				transferCoins(currentTransaction);
			}
		}
	}

	if((probForInternalMiner_ == 1.0) || (probForInternalMiner_ >= Sampler::rand01()))
	{
		// issue transaction of [reward] btc to miner
		//unsigned int numAddrMiner = numAddressesPerClient(miner);
		//unsigned int minerReceiverAddr = Sampler::randRange(numAddrMiner);
		// -> we create a new address for the coins!
		Client& minerC = clientAt(miner);
		assert(minerC.miner_ == true);
		Address rewardAddr(0, minerC.id_);
		int rewardAddrPos = minerC.setOfAddresses_.addElement(rewardAddr);

		//std::vector<unsigned int> generationAddresses(1, 0);
		unsigned int minerRewardTransationID =
				issueTransaction(timestamp, GENERATION, miner, rewardAddrPos, minerReward_, -1);
		// accept automatically
		listOfTransactions_[minerRewardTransationID].accept(timestamp, appearedTransactionCount_);
		appearedTransactionCount_++;
		consolidateAddressesForTransaction(listOfTransactions_[minerRewardTransationID]);
		acceptedTransactionCount_++;
		transferCoins(listOfTransactions_[minerRewardTransationID]);
	}

	return includedTransactionCount; // plus one for the generation reward transaction?
}

// processes a freshly accepted transaction
void BitcoinNetwork::transferCoins(Transaction& transaction)
{
	if(transaction.get_senderID() != GENERATION && transaction.get_shadowAddrNeeded()) // we do not need to transfer something back to GENERATION
	{
		Client& senderC = clientAt(transaction.get_senderID());

		unsigned int numCoinsBefore = senderC.get_numCoins();
		btc balanceBefore = senderC.get_totalBalance();

		const Address& shadowAddress = transaction.get_shadow();
		assert(shadowAddress.get_coins().size() == 1);

		Address& realSenderShadowAddr = senderC.setOfAddresses_.Find(shadowAddress);
		btc changeCoin = shadowAddress.get_totalAmount();
		assert(changeCoin > 0);
		assert(shadowAddress.get_coins().size() == 1);
		realSenderShadowAddr.addCoin(changeCoin);

		assert(senderC.outstandingChangeMoney_ >= changeCoin);
		senderC.outstandingChangeMoney_ -= changeCoin;

		unsigned int numCoinsAfter = senderC.get_numCoins();
		btc balanceAfter = senderC.get_totalBalance();

		assert(numCoinsAfter == numCoinsBefore + 1);
		assert(balanceAfter == (balanceBefore + changeCoin));
	}

	Client& receiverC = clientAt(transaction.get_receiverID());

	unsigned int numCoinsBefore = receiverC.get_numCoins();
	btc balanceBefore = receiverC.get_totalBalance();

	const Address& recipientAddress = transaction.get_recipient();
	//std::cout << transaction.toString("\t") << std::endl;
	//std::cout << transaction.get_recipient().toString() << std::endl;
	//std::cout << transaction.get_shadow().toString() << std::endl;
	Address& realReceiverAddress = receiverC.setOfAddresses_.Find(recipientAddress);

	assert(recipientAddress.get_coins().size() == 1);
	btc payment = recipientAddress.get_totalAmount();
	assert(payment > 0);
	realReceiverAddress.addCoin(payment);

	unsigned int numCoinsAfter = receiverC.get_numCoins();
	btc balanceAfter = receiverC.get_totalBalance();

	assert(numCoinsAfter == numCoinsBefore + 1);
	assert(balanceAfter == (balanceBefore + payment));
}

void BitcoinNetwork::consolidateAddressesForTransaction(Transaction& transaction)
{
	Client& receiverC = clientAt(transaction.get_receiverID());

	if(transaction.get_senderID() == GENERATION)
	{
		assert(!transaction.get_shadowAddrNeeded());
		const Address& recipientAddress = transaction.get_recipient();
		receiverC.setOfAddresses_.Find(recipientAddress).apprearsNow();
		return; // no further actions for GENERATION transactions
	}

	Client& senderC = clientAt(transaction.get_senderID());

	assert(transaction.get_senders().size() > 0);

	const Address& firstAddrFromTransaction = transaction.get_senders()[0];
	//GAaddresses_.addElement(firstAddrFromTransaction.get_hashString());

	unsigned int sizeTr = transaction.get_senders().size();
	for(unsigned int i = 0; i < sizeTr; ++i)
	{
		const Address& addrFromTransaction = transaction.get_senders()[i];

		//GAaddresses_.addElementIfNotExists(addrFromTransaction.get_hashString());
		//GAaddresses_.Union(firstAddrFromTransaction.get_hashString(), addrFromTransaction.get_hashString());

		senderC.setOfAddresses_.Union(firstAddrFromTransaction, addrFromTransaction);
		// we have seen the addresses now in the log
		senderC.setOfAddresses_.Find(addrFromTransaction).apprearsNow();
	}

	const Address& recipientAddress = transaction.get_recipient();
	//GAaddresses_.addElementIfNotExists(recipientAddress.get_hashString());

	if(transaction.get_shadowAddrNeeded())
	{
		const Address& shadowAddress = transaction.get_shadow();
		//GAaddresses_.addElementIfNotExists(shadowAddress.get_hashString());

		//std::cout << transaction.toString("") << std::endl;

		//if(!shadowAddress.hasAppeared() && recipientAddress.hasAppeared()) -> wrong
		if(!senderC.setOfAddresses_.Find(shadowAddress).hasAppeared() &&
				receiverC.setOfAddresses_.Find(recipientAddress).hasAppeared())
		{
			senderC.setOfAddresses_.Union(firstAddrFromTransaction, shadowAddress);
			//GAaddresses_.Union(firstAddrFromTransaction.get_hashString(), shadowAddress.get_hashString());
		}
		// the following case should not happen since the Bitcoin client
		// always creates a completely new address for change (shadow address)
		else if(senderC.setOfAddresses_.Find(shadowAddress).hasAppeared() &&
				!receiverC.setOfAddresses_.Find(recipientAddress).hasAppeared())
		{
			// in this case the heuristic fails -> TODO
			//senderC.setOfAddresses_.Union(firstAddrFromTransaction, recipientAddress); -> not possible we use GAaddress_ instead
			//GAaddresses_.Union(firstAddrFromTransaction.get_hashString(), recipientAddress.get_hashString());
		}
		//we have seen the address in the log
		senderC.setOfAddresses_.Find(shadowAddress).apprearsNow();
	}
	//we have seen the address in the log
	receiverC.setOfAddresses_.Find(recipientAddress).apprearsNow();
}

/*
void BitcoinNetwork::changeAddress(tm &timestamp, unsigned int client, unsigned int affectedAddress) {
}
*/


/// here we have to define an extra case for GENERATION
unsigned int BitcoinNetwork::issueTransaction(const tm &timestamp, unsigned int sender, unsigned int receiver,
		/*const std::vector<unsigned int>& senderAddresses,*/ unsigned int receiverAddress, btc amount, int eventID) {
	//Transaction transct;
	listOfTransactions_.push_back(Transaction(timestamp, numTransactions_, eventID));
	Transaction& new_transaction = listOfTransactions_.back();

	btc changeMoney = 0;

	// sender side
	if(sender == GENERATION)
	{
		Address generationAddress(amount, GENERATION);
		//std::cout << generationAddress.toString() << std::endl;
		new_transaction.addSender(generationAddress);
		new_transaction.set_senderID(GENERATION);
	}
	else
	{
		Client& senderC = clientAt(sender);
		assert(senderC.get_totalBalance() >= amount); // client must be able to afford this transaction
		new_transaction.set_senderID(senderC.id_);
		/*
		btc sum = 0;
		unsigned int i;
		unsigned int maxP = 0;
		for (i = 0; i < senderAddresses.size(); ++i)
		{
			unsigned int p = senderAddresses[i];
			if(p > maxP) maxP = p;

			sum += senderC.setOfAddresses_[p].get_amount();
			Address senderA(senderC.setOfAddresses_[p]);

			senderC.setOfAddresses_[p].set_amount(0.0);
			new_transaction.addSender(senderA);
		}
		*/
		btc sumCoins = 0; // how much the sender spends
		std::map<unsigned int, std::vector<Coin> > coinsToSpendAddress = senderC.selectCoins(amount, sumCoins);
		assert(sumCoins > 0);
		assert(sumCoins >= amount);

		changeMoney = sumCoins - amount;

		for(std::map<unsigned int, std::vector<Coin> > ::const_iterator it = coinsToSpendAddress.begin();
				it != coinsToSpendAddress.end(); ++it)
		{
			Address senderA(senderC.setOfAddresses_[it->first], false);
			senderA.addCoins(it->second);
			new_transaction.addSender(senderA);
		}

		/*
		Address shadowAddr(0.0, senderC.id_);
		unsigned int shadowAddrPos = senderC.setOfAddresses_.addElement(shadowAddr);
		//senderC.shadowAddressesLink_[lastInvolvedAddress] = shadowAddrPos; // link to new shadow address

		shadowAddr.set_amount(changeMoney);
		new_transaction.addChangeReceiver(shadowAddr);
		*/

		if(changeMoney > 0)
		{
			new_transaction.set_shadowAddrNeeded(true);
			/*
			Address& lastInvolvedAddress = senderC.setOfAddresses_[maxP]; // last involved address
			//std::cout << "lastInvolvedAddress:" << lastInvolvedAddress.toString() << std::endl;
			if(senderC.shadowAddressesLink_.count(lastInvolvedAddress) > 0) // shadow address exists
			{
				int existingShadowAddrPos = senderC.shadowAddressesLink_[lastInvolvedAddress];
				Address& existingShadowAddr = senderC.setOfAddresses_[existingShadowAddrPos];
				Address shadowAddr(existingShadowAddr);

				shadowAddr.set_amount(changeMoney);
				new_transaction.addChangeReceiver(shadowAddr);
			}
			else // shadow address does not exist yet
			{
				//std::cout << "fdsa" << std::endl;
				Address shadowAddr(0.0, senderC.id_);
				//std::cout << shadowAddr.toString() << std::endl;
				int shadowAddrPos = senderC.setOfAddresses_.addElement(shadowAddr); // here setOfAddresses is modified!

				//std::cout << senderC.setOfAddresses_.Find(shadowAddr).toString() << std::endl;
				//std::cout << senderC.setOfAddresses_.getElement(shadowAddrPos).toString() << std::endl;
				//std::cout << "here it happens" << std::endl;
				Address& lastInvolvedAddress = senderC.setOfAddresses_[maxP];
				//std::cout << "lastInvolvedAddress:" << lastInvolvedAddress.toString() << std::endl;
				senderC.shadowAddressesLink_[lastInvolvedAddress] = shadowAddrPos; // link to new shadow address

				shadowAddr.set_amount(changeMoney);
				new_transaction.addChangeReceiver(shadowAddr);
			}
			*/

			// we always create a new shadow address!! (referring to the newest source code of the Bitcoin client)
			Address shadowAddr(0, senderC.id_);
			senderC.setOfAddresses_.addElement(shadowAddr);

			shadowAddr.addCoin(changeMoney);
			new_transaction.addChangeReceiver(shadowAddr);

			senderC.outstandingChangeMoney_ += changeMoney;
		}
		else
		{
			new_transaction.set_shadowAddrNeeded(false);
			countOfNoChangeTransactions_++;
		}

		senderC.listOfTransactionsAsSender_.push_back(numTransactions_);
	}


	// receiver side
	Client& receiverC = clientAt(receiver);
	new_transaction.set_receiverID(receiverC.id_);
	Address receiverA(receiverC.setOfAddresses_[receiverAddress], false); // make a copy of the receiver address, dont copy the coins
	receiverA.addCoin(amount);
	new_transaction.addReceiver(receiverA); // add it to the receiver addr of the transaction
	receiverC.listOfTransactionsAsReceiver_.push_back(numTransactions_);


	new_transaction.calculateTotalAmounts();

	btc transaction_IN = new_transaction.get_totalINamount();
	btc transaction_OUT = new_transaction.get_totalOUTamount();

	assert(transaction_OUT == amount + changeMoney);
	assert(transaction_IN == amount + changeMoney);

	numTransactions_++;

	assert((numTransactions_ -1) == new_transaction.get_id());
	return numTransactions_-1;
}

/*
void BitcoinNetwork::testUnionDBG(unsigned int transactionID, tm& tmstp)
{
	Transaction& tr = listOfTransactions_[transactionID];
	tr.accept(tmstp, acceptedTransactionCount_);
	transferCoins(tr);
	consolidateAddressesForTransaction(tr);
}
*/

std::ostream& BitcoinNetwork::outputStatistics(std::ostream& o) const
{
	assert(numTransactions_ == listOfTransactions_.size());
	o << "Number of broadcasted Transactions: " << numTransactions_ << std::endl;
	o << "Number of Transactions appeared in log: " << appearedTransactionCount_ << std::endl;
	o << "Number of Transactions accepted: " << acceptedTransactionCount_ << std::endl;
	o << "Number of generated blocks: " << blockTimeStamps_.size() << std::endl;
	o << "Last block was generated at " << Sampler::tmToString(blockTimeStamps_.back()) << std::endl;
	o << "Last transaction was broadcasted at: " << Sampler::tmToString(listOfTransactions_.back().get_broadcastTimestamp()) << std::endl;
	o << "Number of clients total: " << numClients(false) << std::endl;
	o << "Number of miners total: " << numMiners(false) << std::endl;
	o << "Number of clients appeared in network: " << numClients(true) << std::endl; // leave this: it's used int the clustering!
	o << "Number of miners appeared in network: " << numMiners(true) << std::endl;
	o << "Number of addresses appeared in the network: " << numAddresses(true) << std::endl;
	o << "Number of total addresses in the network (not necessarily appeared ones): " << numAddresses(false) << std::endl;
	o << "Number of broadcasted transactions that did not need change coins: " << countOfNoChangeTransactions_ << std::endl;
	o << "Number of GAs: " << generalAddresses.size() << std::endl;
	o << "Number of GAs consisting of only one single generation transaction: " << countOfSingleGenerationGAs_ << std::endl;
	o << "Number of GAs consisting of exactly one transaction: " << countOfSingleTransactionGAs_ << std::endl;
	return o;
}

std::ostream& operator<<(std::ostream& o, const BitcoinNetwork& network)
{
	return network.outputStatistics(o);
}
