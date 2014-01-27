
#include "NetworkEvent.hpp"
#include "BitcoinNetwork.hpp"
#include "TimeStepping.hpp"
#include "Sampler.hpp"

#include <cassert>
#include <ctime>
#include <string>



NetworkEvent::NetworkEvent(const tm& timestamp, const std::string &name,
							TimeStepping* timeStepping):
	//network_(network),
	timestamp_(timestamp),
	name_(name),
	executed_(false),
	success_(false),
	timeStepping_(timeStepping)
{
	//assert(network != NULL);
	assert(timeStepping != NULL);
	assert(!name_.empty());
}

bool NetworkEvent::execute(BitcoinNetwork* network)
{
	 assert(network != NULL);
	 executed_ = true;
	 success_ = executeInternal(network);
	 timeStepping_->postProcessing(this);
	 return success_;
}

std::ostream& NetworkEvent::logOutput(std::ostream& o) const
{
	assert(executed_);
	if(success_) o << "+ ";
	else o << "- ";
	o << Sampler::tmToString(timestamp_);
	o << " " << name_;
	o << ": ";
	logOutputInternal(o);
	o << std::endl;
	return o;
}

NetworkEvent::~NetworkEvent() {
}


std::ostream& operator<<(std::ostream& o, const NetworkEvent& nEv)
{
	return nEv.logOutput(o);
}

/*
void NetworkEvent::initialize(tm& timestamp, bool needsLogging, const std::string &name, BitcoinNetwork *network) {
	tm newTimestamp = sampleNextOccurrence(timestamp);
	setNetwork(network);
	setTimeStamp(newTimestamp);
	setLogging(needsLogging);
	setName(name);
	setAborted(false);
}
*/
