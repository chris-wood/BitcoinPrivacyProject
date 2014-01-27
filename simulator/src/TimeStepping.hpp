
#ifndef TIMESTEPPING_HPP_
#define TIMESTEPPING_HPP_

#include "ParameterList.hpp"
#include "BitcoinNetwork.hpp"
#include "NetworkEvent.hpp"
#include "GenerateBlockEvent.hpp"

#include <ctime>
#include <queue>
#include <vector>


class TimeStepping { // abstract class
public:
	virtual ~TimeStepping();
	NetworkEvent* getNextEvent(void);
	//void postProcessing(GenerateBlockEvent* blockGenerationEvent);
	virtual void postProcessing(NetworkEvent* networkEvent) = 0; // pure virtual
	virtual std::ostream& outputStatistics(std::ostream& o) const = 0;
	//void addNewEventToQueue(std::string& eventName, tm& old_EventTime);
	inline bool isRunning() const { return running; };
	inline void abort() { running = false; }
	inline bool empty(void) const { return eventQueue_.empty(); }
	inline const tm& get_startTime(void) const { return startTime_; }
	inline unsigned int get_remainingCount(void) const { return eventQueue_.size(); }
protected:
	explicit TimeStepping(); //(BitcoinNetwork* network);
	std::priority_queue<NetworkEvent*, std::vector<NetworkEvent*> , NetworkEventCompare> eventQueue_;
	tm startTime_;
	tm currentTime_;
	tm nextBlockGeneration_;
	//BitcoinNetwork* network_;
	long maxBlocks_;
	long numBlocks_;
	long numClients_;
	long maxClients_;
	long numTransactions_;
	long maxTransactions_;
private:
	bool running;
};


extern std::ostream& operator<<(std::ostream& o, const TimeStepping& tS);

#endif /* TIMESTEPPING_HPP_ */
