
#include "UniversityModelPlanningEvent.hpp"
#include "../main.hpp"
#include "../BitcoinNetwork.hpp"
#include "../Sampler.hpp"

#include <ctime>
#include <vector>
#include <sstream>

UniversityModelPlanningEvent::UniversityModelPlanningEvent(const tm& timestamp,
		TimeStepping* timeStepping):
		NetworkEvent(timestamp, "PlanningEvent", timeStepping)
{
}

UniversityModelPlanningEvent::~UniversityModelPlanningEvent() {
}

std::ostream& UniversityModelPlanningEvent::logOutputInternal(std::ostream& o) const
{
	o << "PlanningEvent executed";
	return o;
}

bool UniversityModelPlanningEvent::executeInternal(BitcoinNetwork* network)
{
	// do nothing here
	return true;
}
