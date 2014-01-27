
#include "StatisticsContainer.hpp"
#include "StatisticsCalculator.hpp"

#include <sstream>
#include <vector>
#include <string>
#include <cassert>

StatisticsContainer::StatisticsContainer(const std::string& containerName):
	containerName_(containerName),
	running_(false)
{
}

StatisticsContainer::~StatisticsContainer() {
	for(unsigned int i = 0; i < statCalcs_.size(); ++i)
	{
		delete statCalcs_[i];
	}
}

std::ostream& StatisticsContainer::output(std::ostream& o) const
{
	assert(running_);
	for(unsigned int i = 0; i < statCalcs_.size(); ++i)
	{
		statCalcs_[i]->output(o);
		o << std::endl;
	}
	return o;
}

void StatisticsContainer::addValue(double newVal)
{
	assert(statCalcs_.size() > 0);
	for(unsigned int i = 0; i < statCalcs_.size(); ++i)
			statCalcs_[i]->addNewValue(newVal);
	running_ = true;
}

std::ostream& operator<<(std::ostream& o, const StatisticsContainer& statContainer)
{
	statContainer.output(o);
	return o;
}
