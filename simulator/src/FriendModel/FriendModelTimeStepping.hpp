
#ifndef FRIENDMODELTIMESTEPPING_HPP_
#define FRIENDMODELTIMESTEPPING_HPP_

#include "../ParameterList.hpp"
#include "../NetworkEvent.hpp"
#include "../BitcoinNetwork.hpp"
#include "../TimeStepping.hpp"
#include "../NewClientEvent.hpp"
#include "../GenerateBlockEvent.hpp"
#include "../ChangeAddressEvent.hpp"
#include "../IssueTransactionEvent.hpp"
#include "../Sampler.hpp"

#include <ctime>
#include <queue>
#include <vector>

class FriendModelTimeStepping : public TimeStepping {
public:
	explicit FriendModelTimeStepping();
	virtual ~FriendModelTimeStepping();
	void postProcessing(NetworkEvent* networkEvent); // implement abstract class
	std::ostream& outputStatistics(std::ostream& o) const;
private:

};

#endif /* FRIENDMODELTIMESTEPPING_HPP_ */
