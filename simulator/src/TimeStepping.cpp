#include "ParameterList.hpp"
#include "TimeStepping.hpp"
#include "NetworkEvent.hpp"
#include "NewClientEvent.hpp"
#include "GenerateBlockEvent.hpp"
#include "ChangeAddressEvent.hpp"
#include "IssueTransactionEvent.hpp"
#include "BitcoinNetwork.hpp"
#include "Sampler.hpp"

#include <ctime>
#include <queue>
#include <cassert>
#include <string>

TimeStepping::TimeStepping(): //(BitcoinNetwork* network):
	maxBlocks_(0),
	numBlocks_(0),
	numClients_(0),
	maxClients_(0),
	numTransactions_(0),
	maxTransactions_(0),
	running(true)
{
	if(ParameterList::parameterExists("startTime"))
	{
		std::string startTimeStr = ParameterList::getParameterByName<std::string>("startTime");
		startTime_ = Sampler::stringToTm(startTimeStr);
	}
	else
	{
		// start date of first block 2009-01-09 02:54:25 = 1231469665 -> http://blockexplorer.com/b/1
		// see also http://www.epochconverter.com
		time_t startTimeInSeconds = (time_t)1231469665;
		startTime_ = *(gmtime(&startTimeInSeconds));
	}

	currentTime_ = startTime_;
	nextBlockGeneration_ = startTime_;

	/*
	assert(network != NULL);
	network_ = network;
	*/
	maxClients_ = ParameterList::getParameterByName<long>("maxClients");

	if(ParameterList::parameterExists("maxBlocks"))
	{
		maxBlocks_ = ParameterList::getParameterByName<long>("maxBlocks");
	}

	if(ParameterList::parameterExists("maxTransactions"))
	{
		maxTransactions_ = ParameterList::getParameterByName<long>("maxTransactions");
	}
}

TimeStepping::~TimeStepping() {
	while(!eventQueue_.empty())
	{
		NetworkEvent* event = eventQueue_.top();
		eventQueue_.pop();
		delete event;
	}
}

NetworkEvent* TimeStepping::getNextEvent(void) {
	assert(isRunning());
	assert(!eventQueue_.empty());
	NetworkEvent* event = eventQueue_.top();
	eventQueue_.pop();

	std::string eventName = event->getName();
	currentTime_ = event->getTimeStamp();
	// add new event of same kind to queue
	// Note: this can be extended to a more sophisticated mechanism if needed
	//addNewEventToQueue(eventName, currentTime_);
	// return event for execution in main loop
	return event;
}

std::ostream& operator<<(std::ostream& o, const TimeStepping& tS)
{
	return tS.outputStatistics(o);
}
