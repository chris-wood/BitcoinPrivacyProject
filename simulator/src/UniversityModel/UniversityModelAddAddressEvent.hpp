

#ifndef UNIVERSITYMODELADDADDRESSEVENT_HPP_
#define UNIVERSITYMODELADDADDRESSEVENT_HPP_

#include "../NetworkEvent.hpp"
#include "../BitcoinNetwork.hpp"
#include "UniversityModelClient.hpp"
#include "UniversityModelIssueTransactionEvent.hpp"


#include <ctime>
#include <vector>

class UniversityModelTimeStepping;

class UniversityModelAddAddressEvent : public UniversityModelIssueTransactionEvent {
public:
	UniversityModelAddAddressEvent(const tm& timestamp,
			UniversityModelTimeStepping* timeStepping, btc amount, unsigned int client, unsigned int umcPos);
	virtual ~UniversityModelAddAddressEvent();
	bool executeInternal(BitcoinNetwork* network);
	std::ostream& logOutputInternal(std::ostream& o) const;
};

#endif /* UNIVERSITYMODELADDADDRESSEVENT_HPP_ */
