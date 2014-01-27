
#ifndef UNIVERSITYMODELCLIENT_HPP_
#define UNIVERSITYMODELCLIENT_HPP_

#include "../Client.hpp"
#include "../Sampler.hpp"
#include "UniversityModel.hpp"

class UniversityModelClient {
public:
	UniversityModelClient(unsigned int profileID, const tm& timestamp, bool isPrivacyAware, bool isSeller);
	virtual ~UniversityModelClient();
	inline unsigned int get_ID(void) const { assert(IDisSET_); return id_; }
	inline void set_ID(unsigned int newID) { assert(!IDisSET_); id_ = newID; IDisSET_ = true; }
	inline unsigned int get_profileID(void) const { return profileID_; }
	inline bool get_isSeller(void) const { return isSeller_; }
	inline bool get_isPrivacyAware(void) const { return isPrivacyAware_; }
	inline unsigned int get_storeID(void) const { assert(isSeller_); return storeID_; }
	inline void set_storeID(unsigned int storeID) { assert(isSeller_); storeID_ = storeID; }
	inline void increaseFailedTransactions(void) { failedTransactionCount_++; }
	inline void clearFailedTransactionCount(void) { failedTransactionCount_ = 0; }
	inline unsigned int get_failedTransactionCount(void) const { return failedTransactionCount_; }

	std::ostream& logOutput(std::ostream& o) const;
	std::ostream& plotOutput(std::ostream& o, btc maxAmount) const;

	inline double getProb(unsigned int eventID, unsigned int storeID) const
	{
		return probabilities_.at(std::make_pair(eventID, storeID));
	}

	inline const tm& getNextPeriodStartTime(const ProfileEvent& profEv) const
	{
		return eventInfos.at(profEv.eventid).nextPeriodStart;
	}
	inline void setNextPeriodStartTime(const ProfileEvent& profEv, const tm& timestamp)
	{
		eventInfos.at(profEv.eventid).nextPeriodStart = timestamp;
	}
	inline unsigned int getSecsFromEventStart(const ProfileEvent& profEv) const
	{
		return eventInfos.at(profEv.eventid).secsFromEventStart;
	}
	inline unsigned int getSecsFromEventStart(const ProfileEvent& profEv, unsigned int weekDay) const
	{
		return eventInfos.at(profEv.eventid).schedulePerWeekday.at(weekDay);
	}
	inline bool getActiveOneTimeBit(const ProfileEvent& profEv) const
	{
		//assert(ParameterList::getParameterByName<bool>("eventOneTimePad") == true);
		return eventInfos.at(profEv.eventid).oneTimePadBit;
	}
	std::vector<std::pair<double, unsigned int> > getProbabilityVector(const ProfileEvent& profEv, unsigned int weekday) const;
private:
	struct UMCEventInfo
	{
		unsigned int eventid;
		unsigned int periodCount;
		unsigned int curPeriod;
		bool oneTimePadBit;
		tm nextPeriodStart;
		unsigned int secsFromEventStart; // used for single time schedule for all days
		std::map<unsigned int, unsigned int> schedulePerWeekday; // used for a different schedule every weekday
	};

	unsigned int id_;
	unsigned int profileID_;
	bool isPrivacyAware_;
	bool isSeller_;
	unsigned int storeID_;
	bool IDisSET_;
	unsigned int failedTransactionCount_;
	std::map<unsigned int, UMCEventInfo> eventInfos;
	std::map<std::pair<unsigned int, unsigned int>, double> probabilities_;

	inline void setProb(unsigned int eventID, unsigned int storeID, double prob)
	{
		probabilities_[std::make_pair(eventID, storeID)] = prob;
	}
};

// toString method
extern std::ostream& operator<<(std::ostream& o, const UniversityModelClient& umc);

#endif /* UNIVERSITYMODELCLIENT_HPP_ */
