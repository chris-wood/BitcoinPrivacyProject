#ifndef UNIVERSITYMODEL_HPP_
#define UNIVERSITYMODEL_HPP_

#include "../main.hpp"
#include "../Sampler.hpp"
#include "../rapidxml/rapidxml.hpp"

#include <vector>
#include <ctime>
#include <string>
#include <map>
#include <iostream>
#include <cassert>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <exception>

//forward def:
//class UniversityModelConfig;

struct IDComparable
{
	public:
		unsigned int id;
};

class IDCompare {
	public:
		bool operator()(const IDComparable& a1, const IDComparable& a2) const
		{
			return a1.id < a2.id;
		}
};

struct Store : public IDComparable
{
	Store()
	{
		for(unsigned int i = 0; i<7; ++i) availableDaysOfWeek[i] = false;
	}
	std::string name;
	unsigned int eventid;
	//btc maxPrice;
	//btc minPrice;
	bool availableDaysOfWeek[7];
};

struct Event : public IDComparable
{
	std::string name;
	unsigned int startHour;
	unsigned int startMin;
	unsigned int endHour;
	unsigned int endMin;
	std::vector<unsigned int> associatedStores;

	bool hasStore(unsigned int storeID)
	{
		for(unsigned int i = 0; i < associatedStores.size(); ++i)
			if(associatedStores[i] == storeID)
				return true;

		return false;
	}

	unsigned int durationInSecs(void) const
	{
		assert(startHour >= 0 && startHour <= 23);
		assert(endHour >= 0 && endHour <= 23);
		assert(startMin >= 0 && startMin <= 59);
		assert(endMin >= 0 && endMin <= 59);

		assert( (startHour == endHour && startMin < endMin) ||
				(startHour < endHour) );

		unsigned int durInMin = (60*endHour + endMin) - (60*startHour + startMin);

		return durInMin*60;
	}
};

struct ProfileEvent
{
	unsigned int minFreqPerWeek;
	unsigned int maxFreqPerWeek;
	unsigned int eventid;
	//unsigned int prefSum;
};

struct Supplies
{
	bool active;
	btc minAmount;
	btc maxAmount;
	unsigned int minMinutesAfterLastFailure;
	unsigned int maxMinutesAfterLastFailure;
	unsigned int failedTransactions;
};

struct PRIEX
{
public:
	bool active;

	unsigned int minFreqPerWeek;
	unsigned int maxFreqPerWeek;

	unsigned int startHour;
	unsigned int startMin;
	unsigned int endHour;
	unsigned int endMin;

	btc minPrice;
	btc maxPrice;

	std::vector<unsigned int> availableDaysOfWeek;

	unsigned int durationInSecs(void) const
	{
		assert(startHour >= 0 && startHour <= 23);
		assert(endHour >= 0 && endHour <= 23);
		assert(startMin >= 0 && startMin <= 59);
		assert(endMin >= 0 && endMin <= 59);

		assert( (startHour == endHour && startMin < endMin) ||
				(startHour < endHour) );

		unsigned int durInMin = (60*endHour + endMin) - (60*startHour + startMin);

		return durInMin*60;
	}

protected:
	virtual ~PRIEX() { };
};


struct InterExchange : public PRIEX
{
};

struct PrivacyAwareness : public PRIEX
{
};


struct Profile : IDComparable
{
	std::string name;
	double percentage;
	double miners;
	double privacyAwarePercentage;
	btc minInitialAmount;
	btc maxInitialAmount;
	unsigned int maxInitialAddressCount;
	unsigned int minInitialAddressCount;
	InterExchange interExchange;
	Supplies supplies;
	PrivacyAwareness privacyAwareness;
	std::vector<TimeSpan> nonAvailabilityTimes;
	std::map<unsigned int, ProfileEvent> profileEvents;
	std::map<std::pair<unsigned int, unsigned int>, ProbPriceRange> probRanges;

	const ProbPriceRange& getProbRange(unsigned int eventID, unsigned int storeID) const
	{
		return probRanges.at(std::make_pair(eventID, storeID));
	}
};


class UniversityModelConfig : public Singleton<UniversityModelConfig>
{
	friend class Singleton<UniversityModelConfig>;

private:
	UniversityModelConfig() { assert(false); }
	UniversityModelConfig(const std::string& fname);
	void parseNonavailabilityTimes(rapidxml::xml_node<> *topNode, std::vector<TimeSpan>& nonAvailVec) const;
	void parsePRIEX(const std::string& node_name, rapidxml::xml_node<> *proNode, Profile& pro, PRIEX& priex, bool isSeller) const;
	void parseSupplies(const std::string& node_name, rapidxml::xml_node<> *proNode, Profile& pro, bool isSeller) const;

	void readFromConfigFile(const std::string& fname);
	std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems) const;
	std::vector<std::string> split(const std::string &s, char delim) const;

	template <typename T> T readAttribute(rapidxml::xml_node<>* node, const std::string& attrName) const
	{
		assert(!attrName.empty());
		assert(node != 0 || node != NULL);
		rapidxml::xml_attribute<> *attr = node->first_attribute(attrName.c_str());
		if(attr == 0 || attr == NULL) throw std::logic_error("Attribute " + attrName + " does not exist in Node " + node->name());
		std::string inputAttrNameStr(attr->name());
		assert(inputAttrNameStr == attrName);
		std::string inputAttrValueStr = std::string(attr->value());
		if(inputAttrValueStr.empty()) throw std::logic_error("Attribute " + attrName + " from Node " + node->name() + " must not be empty.");
		T val;
		if(!(convertStringToType<T>(inputAttrValueStr, val))) throw std::logic_error("Attribute " + attrName + " from Node " + node->name() + ": could not read the value. Bad format?");
		return val;
	}

	template <typename T> T readAttribute(rapidxml::xml_node<>* node, const std::string& attrName, const T& minVal, const T& maxVal, bool includeBoundary = true) const
	{
		T val = readAttribute<T>(node, attrName);
		if((!includeBoundary) && (val <= minVal || val >= maxVal) )
			throw std::logic_error("Attribute " + attrName + " from Node " + node->name()
					+ " must be in (" + Str(minVal) + "," + Str(maxVal) + ")");
		if(val < minVal || val > maxVal)
			throw std::logic_error("Attribute " + attrName + " from Node " + node->name()
					+ " must be in [" + Str(minVal) + "," + Str(maxVal) + "]");
		return val;
	}

	btc readPrice(rapidxml::xml_node<>* node, const std::string& attrName, const btc& minVal, const btc& maxVal, bool includeBoundary = true) const
	{
		btc ret = 0;
		std::string priceAsString = readAttribute<std::string>(node, attrName);
		std::logic_error perror("In Node " + Str(node->name()) + ": could not read the price values. Bad format? Please use 'XX.YY' for BTC prices.");
		std::string::size_type pos = priceAsString.find('.');
		if(pos != std::string::npos)
		{
			unsigned int decimalDigits = priceAsString.length() - pos - 1;
			if(!(decimalDigits == 1 || decimalDigits == 2))
				throw perror; //std::logic_error("decimalDigits: " + Str(decimalDigits) + " " + priceAsString + " " + Str(priceAsString.length()) + " " + Str(pos) );
			priceAsString.erase(pos, 1); // erase the decimal point
			if(decimalDigits == 1)
				priceAsString += "0";
		}

		if(!(convertStringToType<btc>(priceAsString, ret)))
			throw perror;

		if(pos == std::string::npos)
			ret *= 100;

		if((!includeBoundary) && (ret <= minVal || ret >= maxVal) )
			throw std::logic_error("Price " + attrName + " from Node " + node->name()
					+ " must be in (" + Str(minVal) + "," + Str(maxVal) + ")");
		if(ret < minVal || ret > maxVal)
			throw std::logic_error("Price " + attrName + " from Node " + node->name()
					+ " must be in [" + Str(minVal) + "," + Str(maxVal) + "]");

		assert(ret <= maxVal && ret >= minVal);
		return ret;
	}

	void readDayTime(rapidxml::xml_node<>* node, const std::string& time, unsigned int& hourR, unsigned int& minR) const
	{
		std::logic_error lerror("In Node " + Str(node->name()) + ": could not read the startTime/endTime values. Bad format? Please use 'XX:YY' for hours and minutes.");
		std::string::size_type pos = time.find(':');
		if(pos == std::string::npos)
			throw lerror;
		std::string hours;
		std::string mins;
		try
		{
			hours = time.substr(0, pos);
			mins = time.substr(pos+1);
		}
		catch (exception &e)
		{
			throw lerror;
		}
		if(hours.empty() || mins.empty())
			throw lerror;

		if(!(convertStringToType<unsigned int>(hours, hourR))) throw lerror;
		if(!(convertStringToType<unsigned int>(mins, minR))) throw lerror;

		if(hourR < 0 || hourR > 23 || minR < 0 || minR > 59) throw lerror;
	}

	void readAttributeDayTimes(rapidxml::xml_node<>* node, Event& ev) const
	{
		assert(node != NULL);
		std::string startTime = readAttribute<std::string>(node, "startTime");
		std::string endTime = readAttribute<std::string>(node, "endTime");
		readDayTime(node, startTime, ev.startHour, ev.startMin);
		readDayTime(node, endTime, ev.endHour, ev.endMin);
	}

	void readAttributeDayTimes(rapidxml::xml_node<>* node, PRIEX& priex) const
	{
		assert(node != NULL);
		std::string startTime = readAttribute<std::string>(node, "startTime");
		std::string endTime = readAttribute<std::string>(node, "endTime");
		readDayTime(node, startTime, priex.startHour, priex.startMin);
		readDayTime(node, endTime, priex.endHour, priex.endMin);
	}

	void readAttributeDaysOfWeek(rapidxml::xml_node<>* node, bool weekDayArray[]) const
	{
		std::logic_error lerror("In Node " + Str(node->name()) + ": could not read the 'availableDayOfWeek' values. Bad format? Please use '1,2,3,4, etc.' for weekdays.");
		std::string availableDaysOfWeek = readAttribute<std::string>(node, "availableDaysOfWeek");
		std::vector<std::string> daysSeparated = split(availableDaysOfWeek, ',');
		for(unsigned int i = 0; i < daysSeparated.size(); ++i)
		{
			if(!daysSeparated[i].empty())
			{
				unsigned int dayAsNr;
				if(!(convertStringToType<unsigned int>(daysSeparated[i], dayAsNr))) throw lerror;
				if(dayAsNr >= 1 && dayAsNr <= 7)
					weekDayArray[dayAsNr-1] = true;
				else
					throw lerror;
			}
			else
			{
				throw lerror;
			}
		}
	}

	void readAttributeDaysOfWeek(rapidxml::xml_node<>* node, std::vector<unsigned int>& vecOut) const
	{
		std::logic_error lerror("In Node " + Str(node->name()) + ": could not read the 'availableDayOfWeek' values. Bad format? Please use '1,2,3,4, etc.' for weekdays.");
		std::string availableDaysOfWeek = readAttribute<std::string>(node, "availableDaysOfWeek");
		std::vector<std::string> daysSeparated = split(availableDaysOfWeek, ',');
		vecOut.clear();
		for(unsigned int i = 0; i < daysSeparated.size(); ++i)
		{
			if(!daysSeparated[i].empty())
			{
				unsigned int dayAsNr;
				if(!(convertStringToType<unsigned int>(daysSeparated[i], dayAsNr))) throw lerror;
				if(dayAsNr >= 1 && dayAsNr <= 7)
					if(std::find(vecOut.begin(), vecOut.end(), dayAsNr)==vecOut.end())
						vecOut.push_back(dayAsNr);
					else
						throw lerror;
				else
					throw lerror;
			}
			else
			{
				throw lerror;
			}
		}
	}

	void readAttributeDaysOfWeek(rapidxml::xml_node<>* node, PRIEX& priex) const
	{
		readAttributeDaysOfWeek(node, priex.availableDaysOfWeek);
	}

	void readAttributeDaysOfWeek(rapidxml::xml_node<>* node, Store& sto) const
	{
		readAttributeDaysOfWeek(node, sto.availableDaysOfWeek);
	}

	std::string fileName;
	char * buffer;
public:
	~UniversityModelConfig(){}
	std::map<unsigned int, Profile> profiles;
	std::map<unsigned int, Profile> sellerProfiles;
	std::map<unsigned int, Event> events;
	std::map<unsigned int, Store> stores;
	std::vector<TimeSpan> globalNonAvailabilityTimes;
	std::vector<std::pair<double, unsigned int> > profileProbabilities;
	std::vector<std::pair<double, unsigned int> > sellerProfileProbabilities;

	std::ostream& output(std::ostream& o) const;
};

extern std::ostream& operator<<(std::ostream& o, const UniversityModelConfig& umconf);
extern std::ostream& operator<<(std::ostream& o, const Store& store);
extern std::ostream& operator<<(std::ostream& o, const Event& ev);
extern std::ostream& operator<<(std::ostream& o, const Profile& profile);
extern std::ostream& operator<<(std::ostream& o, const PRIEX& priex);
extern std::ostream& operator<<(std::ostream& o, const Supplies& supplies);

#endif /* UNIVERSITYMODEL_HPP_ */
