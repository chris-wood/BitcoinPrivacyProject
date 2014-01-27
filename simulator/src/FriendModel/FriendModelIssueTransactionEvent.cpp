
#include "../main.hpp"
#include "FriendModelIssueTransactionEvent.hpp"
#include "../IssueTransactionEvent.hpp"
#include "../BitcoinNetwork.hpp"
#include "../Sampler.hpp"

#include <ctime>
#include <vector>
#include <sstream>


FriendModelIssueTransactionEvent::FriendModelIssueTransactionEvent(const tm& timestamp, TimeStepping* timeStepping):
		IssueTransactionEvent(timestamp, timeStepping)
{
}


bool FriendModelIssueTransactionEvent::executeInternal(BitcoinNetwork* network)
{
	if(network->numClients(false) < 2)
	{
		errorExplanation_ = "Less than two clients";
		return false;
	}

	amount_ = Sampler::rand01() * Sampler::randRange(1, 51);
	sender_ = Sampler::randRange(0, network->numClients(false));

	std::vector<unsigned int> friends = network->friendsOfClient(sender_);

	if(friends.empty())
	{
		errorExplanation_ = "Client " + Str(sender_) + " has no associated counter parties / friends";
		return false;
	}

	int maxTries = 30;
	do {
		receiver_ = friends[Sampler::randRange(0, friends.size())];
		maxTries--;
	} while(sender_ == receiver_ && maxTries > 0);
	if(maxTries <= 0)
	{
		errorExplanation_ = "Client " + Str(sender_) + ": No counter party / friend found";
		return false;
	}

	unsigned int numAddrReceiver = network->numAddressesPerClient(receiver_);
	receiverAddress_ = Sampler::randRange(0, numAddrReceiver);

	return IssueTransactionEvent::executeInternal(network);
}

FriendModelIssueTransactionEvent::~FriendModelIssueTransactionEvent() {
}

