

#include "UniversityModelClient.hpp"
#include "UniversityModel.hpp"
#include "../Client.hpp"
#include "../Sampler.hpp"
#include "../main.hpp"

#include <vector>

UniversityModelClient::UniversityModelClient(unsigned int profileID, const tm& startTimeStamp, bool isPrivacyAware, bool isSeller):
	//id_(id),
	profileID_(profileID),
	isPrivacyAware_(isPrivacyAware),
	isSeller_(isSeller),
	IDisSET_(false),
	failedTransactionCount_(0)
{
	if(!isSeller)
	{
		const Profile& profile = UniversityModelConfig::getInstance().profiles.at(profileID);
		for(std::map<unsigned int, ProfileEvent>::const_iterator it = profile.profileEvents.begin();
				it != profile.profileEvents.end(); ++it)
		{
			const ProfileEvent& profileEvent = it->second;
			unsigned int eventID = profileEvent.eventid;
			assert(it->first == eventID);
			const Event& event = UniversityModelConfig::getInstance().events.at(eventID);

			std::vector<ProbPriceRange> rangesForEvent;
			const std::vector<unsigned int>& storeVec = event.associatedStores;
			for(unsigned int i = 0; i < storeVec.size(); ++i)
			{
				rangesForEvent.push_back(profile.getProbRange(eventID, storeVec[i]));
			}

			std::vector<double> probs = Sampler::sampleProbabilityDistrFromPrefs(rangesForEvent);
			assert(rangesForEvent.size() == probs.size());
			assert(rangesForEvent.size() == storeVec.size());

			for(unsigned int i = 0; i < storeVec.size(); ++i)
			{
				setProb(eventID, storeVec[i], probs[i]);
			}

			UMCEventInfo umcEvInfo;
			umcEvInfo.curPeriod = 0;
			umcEvInfo.periodCount = 0;
			umcEvInfo.eventid = eventID;
			umcEvInfo.nextPeriodStart = startTimeStamp;
			umcEvInfo.secsFromEventStart = Sampler::randRange(event.durationInSecs()+1);
			umcEvInfo.oneTimePadBit = true;
			if(ParameterList::getParameterByName<bool>("eventOneTimePad"))
				umcEvInfo.oneTimePadBit = (Sampler::rand01() > 0.5)? true : false;
			if(ParameterList::getParameterByName<bool>("weekSchedules") == true)
			{
				unsigned int numberOfDaysPerWeek = Sampler::randRange(profileEvent.minFreqPerWeek, profileEvent.maxFreqPerWeek +1);
				if(numberOfDaysPerWeek > 0)
				{
					std::vector<unsigned int> randomWeekdays = Sampler::randVectPermutation(1, 7, numberOfDaysPerWeek);
					assert(randomWeekdays.size() == numberOfDaysPerWeek);
					for(unsigned int i = 0; i < randomWeekdays.size(); ++i)
					{
						unsigned int randomDay = randomWeekdays[i];
						umcEvInfo.schedulePerWeekday.insert( std::make_pair(randomDay, Sampler::randRange(event.durationInSecs()+1)) );
					}
				}
			}
			eventInfos[eventID] = umcEvInfo;
		}
	}
}

UniversityModelClient::~UniversityModelClient() {
}

std::vector<std::pair<double, unsigned int> > UniversityModelClient::getProbabilityVector(const ProfileEvent& profEv,
																						unsigned int weekday) const
{
	assert(weekday >= 1 && weekday <= 7);
	std::vector<std::pair<double, unsigned int> > v;
	double sum = 0.0;
	assert(probabilities_.size() > 0);
	for(std::map<std::pair<unsigned int, unsigned int>, double>::const_iterator it = probabilities_.begin();
			it != probabilities_.end(); ++it)
	{
		if(it->first.first == profEv.eventid)
		{
			const Store& store = UniversityModelConfig::getInstance().stores.at(it->first.second);
			assert(store.id == it->first.second);
			if(store.availableDaysOfWeek[weekday-1])
			{
				v.push_back(std::make_pair(it->second, store.id));
				sum += it->second;
			}
		}
	}
	//normalization
	for(std::vector<std::pair<double, unsigned int> >::iterator it = v.begin(); it != v.end(); ++it)
	{
		it->first /= sum;
	}
	//assert(v.size() > 0);
	return v;
}

std::ostream& UniversityModelClient::logOutput(std::ostream& o) const
{
	string name;
	string storeStr = "no_store";
	if(isSeller_)
	{
		name = UniversityModelConfig::getInstance().sellerProfiles.at(profileID_).name;
		storeStr = Str(storeID_) + ":" + UniversityModelConfig::getInstance().stores.at(storeID_).name;
	}
	else
	{
		name = UniversityModelConfig::getInstance().profiles.at(profileID_).name;
	}
	o << "UmC: [" << id_ << " : " << isSeller_ << " : " << profileID_ << ":" << isPrivacyAware_ << ":" << name << " : " << storeStr <<"]" << std::endl;
	o << probabilities_;
	return o;
}

std::ostream& UniversityModelClient::plotOutput(std::ostream& o, btc maxAmount) const
{
	unsigned int count = 1;
	btc step = maxAmount / (UniversityModelConfig::getInstance().events.size()+2);

	for(std::map<unsigned int, UMCEventInfo>::const_iterator it = eventInfos.begin(); it != eventInfos.end(); ++it)
	{
		btc yPos = maxAmount - step*count -5;
		const Event& ev = UniversityModelConfig::getInstance().events.at(it->first);

		if(it->second.oneTimePadBit)
		{
			unsigned int timeInSecsFromMidnight = it->second.secsFromEventStart + ev.startHour*3600 + ev.startMin*60;
			unsigned int z = timeInSecsFromMidnight;
			unsigned int additionalSecs = z % 60;
			z -= additionalSecs;
			z /= 60;
			unsigned int additionalMins = z % 60;
			z -= additionalMins;
			z /= 60;
			unsigned int additionalHours = z;
			o << timeInSecsFromMidnight << ", " << Str(maxAmount) << ", " << Str(yPos) << ", "
			  << ev.name << ": " << Sampler::dayTimeStr(additionalHours, additionalMins, additionalSecs) << ", " << ev.id << ", 1";
		}
		else
		{
			o << "10000, " << Str(maxAmount) << ", " << Str(yPos) << ", " <<ev.name << ": off, " << ev.id << ", 0";
		}
		o << std::endl;
		count++;
	}
	return o;
}

std::ostream& operator<<(std::ostream& o, const UniversityModelClient& umc)
{
	return umc.logOutput(o);
}


