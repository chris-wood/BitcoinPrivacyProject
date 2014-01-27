

#include "../main.hpp"
#include "UniversityModelIssueTransactionEvent.hpp"
#include "UniversityModelClient.hpp"
#include "UniversityModelClientSelector.hpp"
#include "../IssueTransactionEvent.hpp"
#include "../BitcoinNetwork.hpp"
#include "../Sampler.hpp"

#include <ctime>
#include <vector>
#include <sstream>


UniversityModelIssueTransactionEvent::UniversityModelIssueTransactionEvent(const tm& timestamp,
		UniversityModelTimeStepping* timeStepping, btc amount, unsigned int sender, unsigned int receiver, unsigned int umcPos, int eventID):
		IssueTransactionEvent(timestamp, timeStepping, amount, sender, receiver, 0, eventID), // set the receiver_address to zero,
		// we modify anyways before issuing the transaction
		UniversityModelClientSelector(umcPos, timeStepping),
		receiverAddressIsSet_(false),
		isNotBitcoinPurchase_(true)
{
}

UniversityModelIssueTransactionEvent::UniversityModelIssueTransactionEvent(const tm& timestamp,
		UniversityModelTimeStepping* timeStepping, btc amount, unsigned int sender, unsigned int receiver, int eventID):
		IssueTransactionEvent(timestamp, timeStepping, amount, sender, receiver, 0, eventID), // set the receiver_address to zero,
		// we modify anyways before issuing the transaction
		UniversityModelClientSelector(0, NULL),
		receiverAddressIsSet_(false),
		isNotBitcoinPurchase_(false)
{
}

UniversityModelIssueTransactionEvent::~UniversityModelIssueTransactionEvent() {
}

bool UniversityModelIssueTransactionEvent::executeInternal(BitcoinNetwork* network)
{
	if(!receiverAddressIsSet_)
	{
		unsigned int numAddrReceiver = network->numAddressesPerClient(receiver_);
		receiverAddress_ = Sampler::randRange(0, numAddrReceiver);
		receiverAddressIsSet_ = true;
	}
	bool retval = IssueTransactionEvent::executeInternal(network);
	if(!retval && isNotBitcoinPurchase())
	{
		getClientRepresentation().increaseFailedTransactions();
	}
	return retval;
}

