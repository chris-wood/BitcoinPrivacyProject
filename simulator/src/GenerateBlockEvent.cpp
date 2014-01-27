#include "GenerateBlockEvent.hpp"
#include "BitcoinNetwork.hpp"
#include "Client.hpp"
#include "Sampler.hpp"

#include <ctime>


GenerateBlockEvent::GenerateBlockEvent(const tm& timestamp,
					TimeStepping* timeStepping):
	NetworkEvent(timestamp, "GenerateBlockEvent", timeStepping)
{
}

GenerateBlockEvent::~GenerateBlockEvent() {
}

std::ostream& GenerateBlockEvent::logOutputInternal(std::ostream& o) const
{
	if(successful())
	{
		o << "New block created: " << "Miner=" << luckyMiner_ << " transactionsIncluded=" << transactionsIncluded_;
	}
	else
	{
		o << "ERROR: No Block generation possible: there are no miners in the network";
	}
	return o;
}

bool GenerateBlockEvent::executeInternal(BitcoinNetwork* network)
{
	// TODO: sample the affected miner
	if(network->numMiners(false) == 0) return false;
	unsigned int luckyMinerIndex = Sampler::randRange(0, network->numMiners(false));
	luckyMiner_ = network->getMinerIDFromIndex(luckyMinerIndex);
	transactionsIncluded_ = network->generateBlock(getTimeStamp(), luckyMiner_);
	return true;
}

