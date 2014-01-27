
#include "FriendModelTimeStepping.hpp"
#include "../ParameterList.hpp"
#include "../NetworkEvent.hpp"
#include "../IssueTransactionEvent.hpp"
#include "FriendModelIssueTransactionEvent.hpp"
#include "FriendModelNewClientEvent.hpp"
#include "../BitcoinNetwork.hpp"
#include "../TimeStepping.hpp"


#include <ctime>
#include <queue>
#include <vector>

FriendModelTimeStepping::FriendModelTimeStepping():
	TimeStepping()
{
		// initial setting
		tm nextTimestamp = currentTime_;

		NewClientEvent* nce = new FriendModelNewClientEvent(nextTimestamp, this);
		eventQueue_.push(nce);
		numClients_++;

		int seconds1to2days = Sampler::randRange(60*60*24, 60*60*48);
		Sampler::addTimeInterval(nextTimestamp, 0, 0, 0, 0, 0, seconds1to2days);
		GenerateBlockEvent* gbe = new GenerateBlockEvent(nextTimestamp, this);
		eventQueue_.push(gbe);
		numBlocks_++;

		/*
		seconds1to2days = Sampler::randRange(60*60*24, 60*60*48);
		Sampler::addTimeInterval(nextTimestamp, 0, 0, 0, 0, 0, seconds1to2days);
		ChangeAddressEvent* cae = new ChangeAddressEvent(nextTimestamp, network_);
		eventQueue_.push(cae);
		*/

		seconds1to2days = Sampler::randRange(60*60*24, 60*60*48);
		Sampler::addTimeInterval(nextTimestamp, 0, 0, 0, 0, 0, seconds1to2days);
		IssueTransactionEvent* ite = new FriendModelIssueTransactionEvent(nextTimestamp, this);
		eventQueue_.push(ite);
		numTransactions_++;
}

FriendModelTimeStepping::~FriendModelTimeStepping() {
}


void FriendModelTimeStepping::postProcessing(NetworkEvent* networkEvent)
{
	assert(networkEvent != NULL);

	std::string eventName = networkEvent->getName();
	tm old_EventTime = networkEvent->getTimeStamp();

	NetworkEvent* nee = NULL;
	if(eventName == "NewClientEvent") // newClientEvent
	{
		if(maxClients_ <= numClients_)
		{
			std::cout << "NewClientEvent: Reached " << maxClients_ << ", no more new clients created" << std::endl;
		}
		else
		{
			int seconds1to2days = Sampler::randRange(60*60*24, 60*60*48);
			tm nextOccurence = old_EventTime;
			Sampler::addTimeInterval(nextOccurence, 0, 0, 0, 0, 0, seconds1to2days);

			nee = new FriendModelNewClientEvent(nextOccurence, this);
			numClients_++;
		}
	}
	else if(eventName == "GenerateBlockEvent") // generateBlockEvent
	{
		if(maxBlocks_ > 0 && maxBlocks_ <= numBlocks_)
		{
			std::cout << "GenerateBlockEvent: Reached " << maxBlocks_ << ", no more new blocks" << std::endl;
		}
		else
		{
			// TODO: when is the next block generated?
			int seconds1to2days = Sampler::randRange(60*60*24, 60*60*48);
			tm nextOccurence = old_EventTime;
			Sampler::addTimeInterval(nextOccurence, 0, 0, 0, 0, 0, seconds1to2days);

			nee = new GenerateBlockEvent(nextOccurence, this);
			nextBlockGeneration_ = nee->getTimeStamp();
			numBlocks_++;
		}
	}
	//else if(eventName == "ChangeAddressEvent") // addressChangeEvent
	//{
	//int seconds1to2days = Sampler::randRange(60*60*24, 60*60*48);
	//tm nextOccurence = previousOccurence;
	//Sampler::addTimeInterval(nextOccurence, 0, 0, 0, 0, 0, seconds1to2days);
	//return nextOccurence;
	//	nee = new ChangeAddressEvent(old_EventTime, network_);
	//}
	else if(eventName == "IssueTransactionEvent") // issueTransactionEvent
	{
		if(maxTransactions_ > 0 && maxTransactions_ <= numTransactions_)
		{
			std::cout << "IssueTransactionEvent: Reached " << maxTransactions_ << ", no more new transactions" << std::endl;
		}
		else
		{
			// TODO: when does the next new client arise
			int seconds1to2days = Sampler::randRange(60*60*24, 60*60*48);
			tm nextOccurence = old_EventTime;
			Sampler::addTimeInterval(nextOccurence, 0, 0, 0, 0, 0, seconds1to2days);

			nee = new FriendModelIssueTransactionEvent(nextOccurence, this);
			numTransactions_++;
		}
	}

	if(nee != NULL) eventQueue_.push(nee);
}

std::ostream& FriendModelTimeStepping::outputStatistics(std::ostream& o) const
{
	o << "TODO" << std::endl;
	return o;
}
