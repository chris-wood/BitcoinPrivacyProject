#include "ChangeAddressEvent.hpp"
#include "BitcoinNetwork.hpp"
#include "Sampler.hpp"

#include <ctime>

AddAddressEvent::AddAddressEvent(const tm& timestamp,	TimeStepping* timeStepping):
	NetworkEvent(timestamp, "AddAddressEvent", timeStepping)
{
}

AddAddressEvent::~AddAddressEvent() {
}

std::ostream& AddAddressEvent::logOutputInternal(std::ostream& o) const
{
	o <<  "Change address: " << "Client=" << client_ << ", address=" << affectedAddress_;
	return o;
}


bool AddAddressEvent::executeInternal(BitcoinNetwork* network)
{
	if(network->numClients(false) == 0) return false;
	client_ = Sampler::randRange(0, network->numClients(false));
	unsigned int numAddr = network->numAddressesPerClient(client_);
	affectedAddress_ = Sampler::randRange(0, numAddr);
	//network_->changeAddress(timestamp_, client_, affectedAddress_);
	return true;
}

