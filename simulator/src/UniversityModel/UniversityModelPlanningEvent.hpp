
#ifndef UNIVERSITYMODELPLANNINGEVENT_HPP_
#define UNIVERSITYMODELPLANNINGEVENT_HPP_

#include "../NetworkEvent.hpp"
#include "../BitcoinNetwork.hpp"

#include <ctime>
#include <vector>


class UniversityModelPlanningEvent : public NetworkEvent {
public:
	UniversityModelPlanningEvent(const tm& timestamp, TimeStepping* timeStepping);
	virtual ~UniversityModelPlanningEvent();
protected:
	bool executeInternal(BitcoinNetwork* network);
	std::ostream& logOutputInternal(std::ostream& o) const;
};


#endif /* UNIVERSITYMODELPLANNINGEVENT_HPP_ */
