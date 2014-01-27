

#include "../main.hpp"
#include "UniversityModelAddAddressEvent.hpp"
#include "UniversityModelClientSelector.hpp"
#include "../IssueTransactionEvent.hpp"
#include "../BitcoinNetwork.hpp"
#include "../Sampler.hpp"

#include <ctime>
#include <vector>
#include <sstream>


UniversityModelAddAddressEvent::UniversityModelAddAddressEvent(const tm& timestamp,
		UniversityModelTimeStepping* timeStepping, btc amount, unsigned int client, unsigned int umcPos):
		UniversityModelIssueTransactionEvent(timestamp, timeStepping, amount, client, client, umcPos, 7)
{
}

UniversityModelAddAddressEvent::~UniversityModelAddAddressEvent() {
}

bool UniversityModelAddAddressEvent::executeInternal(BitcoinNetwork* network)
{
	receiverAddress_ = network->addAddressForClient(sender_);
	receiverAddressIsSet_ = true; // make sure it does not get changed anymore
	return UniversityModelIssueTransactionEvent::executeInternal(network);
}

std::ostream& UniversityModelAddAddressEvent::logOutputInternal(std::ostream& o) const
{
	o << "New address added: " << "Client=" << sender_ << ", amount=" << Str(amount_)
			<< ", newAddress=receiverAddress=" << receiverAddress_;

	return o;
}

