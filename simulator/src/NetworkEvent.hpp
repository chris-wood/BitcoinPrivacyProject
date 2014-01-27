#ifndef NETWORKEVENT_HPP_
#define NETWORKEVENT_HPP_

#include "BitcoinNetwork.hpp"
#include "Sampler.hpp"

#include <string>
#include <ctime>
#include <sstream>
#include <ostream>

// forward definition
class TimeStepping;

// abstract class
class NetworkEvent {
public:
	virtual ~NetworkEvent();
	bool execute(BitcoinNetwork* network);
	std::ostream& logOutput(std::ostream& o) const;

	inline const tm& getTimeStamp() const { return timestamp_; }
	inline const std::string& getName(void) const { return name_; }
	inline bool successful(void) const { return success_; }
	inline bool executed(void) const { return executed_; }
protected:
	NetworkEvent(const tm& timestamp, const std::string &name, TimeStepping* timeStepping);
	virtual bool executeInternal(BitcoinNetwork* network) = 0;
	virtual std::ostream& logOutputInternal(std::ostream& o) const = 0;
	//BitcoinNetwork* network_;
private:
	tm timestamp_;
	std::string name_;
	bool executed_;
	bool success_;
	TimeStepping* timeStepping_;

	//bool aborted_;
	//bool needsLogging_;
	//inline void setTimeStamp(const tm &new_timestamp) { timestamp_ = new_timestamp; }
	//inline void setLogging(bool new_l) { needsLogging_ = new_l; }
	//inline void setName(const std::string& new_str) { name_ = new_str; }
	//inline void setNetwork(BitcoinNetwork* new_network) { network_ = new_network; }
	//inline void setAborted(bool ab) { aborted_ = ab; }
	// this is called from the constructor of deriving classes
	//void initialize(tm& timestamp, bool needsLogging, const std::string &name, BitcoinNetwork* network);
};


class NetworkEventCompare {
    public:
    bool operator()(NetworkEvent* t1, NetworkEvent* t2) // Returns true if t1 is earlier than t2
    {
       tm tm1 = t1->getTimeStamp();
       tm tm2 = t2->getTimeStamp();
       return !Sampler::tm1BeforeTm2(tm1, tm2);
    }
};

// toString method
extern std::ostream& operator<<(std::ostream& o, const NetworkEvent& nEv);

#endif /* NETWORKEVENT_HPP_ */
