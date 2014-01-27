#include "main.hpp"
#include "IssueTransactionEvent.hpp"
#include "BitcoinNetwork.hpp"
#include "Sampler.hpp"

#include <ctime>
#include <vector>
#include <sstream>

IssueTransactionEvent::IssueTransactionEvent(const tm& timestamp,
							TimeStepping* timeStepping, btc amount,
							unsigned int sender, unsigned int receiver,
//							const std::vector<unsigned int>& senderAddresses,
							unsigned int receiverAddress, int eventID):
	NetworkEvent(timestamp, "IssueTransactionEvent", timeStepping),
	amount_(amount),
	sender_(sender),
	receiver_(receiver),
//	senderAddresses_(senderAddresses),
	receiverAddress_(receiverAddress),
	eventID_(eventID)
{
}

IssueTransactionEvent::IssueTransactionEvent(const tm& timestamp,
		TimeStepping* timeStepping):
		NetworkEvent(timestamp, "IssueTransactionEvent", timeStepping)
{
}

void IssueTransactionEvent::setFields(btc amount, unsigned int sender, unsigned int receiver,
//							const std::vector<unsigned int>& senderAddresses,
							unsigned int receiverAddress)
{
	amount_ = amount;
	sender_ = sender;
	receiver_ = receiver;
	receiverAddress_ = receiverAddress;
}

IssueTransactionEvent::~IssueTransactionEvent() {
}

std::ostream& IssueTransactionEvent::logOutputInternal(std::ostream& o) const
{
	o << "Issue transaction: " << "Sender=" << sender_ << ", Receiver=" << receiver_ << ", amount=" << Str(amount_)
			<< ", receiverAddress=" << receiverAddress_;
	if(!successful())
	{
		o << " -> ERROR: " << errorExplanation_;
	}
	return o;
}

bool IssueTransactionEvent::executeInternal(BitcoinNetwork* network)
{
	if(network->numClients(false) < 2)
	{
		errorExplanation_ = "Less than two clients";
		return false;
	}

	//senderAddresses_ = network->addressesClientWithAmount(sender_, amount_);
	btc clientBalance = network->getBalanceForClient(sender_);
	if(clientBalance < amount_)
	{
		errorExplanation_ = "Client " + Str(sender_) + " has not enough coins: " + Str(clientBalance) + " (" + Str(amount_) + " needed)";
		/*{
			const BitcoinNetwork* constNetwork = static_cast<const BitcoinNetwork*>(network);
			//unsigned int initCoinCount = constNetwork->clientAt(sender_).get_numInitialCoins();
			//std::cout << initCoinCount << std::endl;
			if((clientBalance + constNetwork->clientAt(sender_).get_outstandingChangeMoney()) >= amount_)
			{
				//network->outputGroundTruth(std::cout, sender_);
				std::cout << sender_ << std::endl;
				int t = 0;
			}
		}*/
		return false;
	}

	/*
	if(sender_ == receiver_ && senderAddresses_.size() == 1 && senderAddresses_[0] == receiverAddress_)
	{
		errorExplanation_ = "receiver and sender addresses are identical";
		return false;
	}
	*/

	assert(amount_ > 0);
	assert(sender_ < network->numClients(false));
	assert(receiver_ < network->numClients(false));

	assert(network->numAddressesPerClient(receiver_) > receiverAddress_);

	network->issueTransaction(getTimeStamp(), sender_, receiver_, receiverAddress_, amount_, eventID_);
	return true;
}

