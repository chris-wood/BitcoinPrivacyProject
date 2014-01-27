#ifndef STATISTICSCONTAINER_HPP_
#define STATISTICSCONTAINER_HPP_

#include "StatisticsCalculator.hpp"

#include <sstream>
#include <vector>
#include <string>
#include <cassert>

class StatisticsContainer {
public:
	StatisticsContainer(const std::string& containerName);
	virtual ~StatisticsContainer();
	void addValue(double newVal);

	template <class STATCALC_TYPE >
	void addCalculator();

	std::ostream& output(std::ostream& o) const;
private:
	std::vector<StatisticsCalculator*> statCalcs_;
	std::string containerName_;
	bool running_;
};

template <class STATCALC_TYPE>
void StatisticsContainer::addCalculator()
{
	assert(!running_);
	STATCALC_TYPE* statCalc = new STATCALC_TYPE(containerName_);
	statCalcs_.push_back(statCalc);
}

extern std::ostream& operator<<(std::ostream& o, const StatisticsContainer& statContainer);

#endif /* STATISTICSCONTAINER_HPP_ */
