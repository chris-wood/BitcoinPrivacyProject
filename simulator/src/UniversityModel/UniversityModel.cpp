#include "../main.hpp"
#include "UniversityModel.hpp"
#include "../Sampler.hpp"

#include <vector>
#include <ctime>
#include <string>
#include <cstring>
#include <map>
#include <iostream>
#include <cassert>
#include <fstream>
#include <stdexcept>
#include <exception>


UniversityModelConfig::UniversityModelConfig(const std::string& fname):
	fileName(fname),
	buffer(NULL)
{
	assert(!fileName.empty());
	try
	{
		readFromConfigFile(fname);
	}
	catch (exception& e)
	{
		if(buffer != NULL) delete[] buffer;
		//buffer = NULL;
		//std::cout << e.what() << std::endl;
		throw; // rethrow original exception
	}
	//if(buffer != NULL) delete[] buffer;
	//buffer = NULL;
}

std::vector<std::string>& UniversityModelConfig::split(const std::string &s, char delim, std::vector<std::string> &elems) const
{
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> UniversityModelConfig::split(const std::string &s, char delim) const
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

void UniversityModelConfig::parseNonavailabilityTimes(rapidxml::xml_node<> *topNode, std::vector<TimeSpan>& nonAvailVec) const
{
	for (rapidxml::xml_node<> *node = topNode->first_node("Nonavailability");
			node != NULL && std::string(node->name()) == "Nonavailability"; node = node->next_sibling())
	{
		TimeSpan tS;
		for (rapidxml::xml_attribute<> *attr = node->first_attribute();
			 attr; attr = attr->next_attribute())
		{
			std::string inputStr(attr->name());
			if(inputStr == "startTime")
			{
				std::string startTime = std::string(attr->value());
				tS.startTime = Sampler::stringToTm(startTime);
			}
			else if(inputStr == "endTime")
			{
				std::string endTime = std::string(attr->value());
				tS.endTime = Sampler::stringToTm(endTime);
			}
		}
		nonAvailVec.push_back(tS);
	}
}


void UniversityModelConfig::parseSupplies(const std::string& node_name, rapidxml::xml_node<> *proNode, Profile& pro, bool isSeller) const
{
	assert(proNode != NULL);

	rapidxml::xml_node<> *node = proNode->first_node(node_name.c_str());
	if(node == NULL)
	{
		pro.supplies.active = false;
		return;
	}

	if(isSeller)
		throw std::logic_error("Profile id " + Str(pro.id) + ": " + node_name + " can not be defined for seller profiles");

	pro.supplies.active = true;

	pro.supplies.minMinutesAfterLastFailure = readAttribute<unsigned int>(node, "minMinutesAfterLastFailure", 1, 1000);
	pro.supplies.maxMinutesAfterLastFailure = readAttribute<unsigned int>(node, "maxMinutesAfterLastFailure", 1, 1000);

	if(pro.supplies.minMinutesAfterLastFailure > pro.supplies.maxMinutesAfterLastFailure)
		throw std::logic_error("Profile id " + Str(pro.id) + " " + node_name + ": minMinutesAfterLastFailure > maxMinutesAfterLastFailure!!!");


	pro.supplies.minAmount = readPrice(node, "minAmount" , ONECENT, 500000, true);
	pro.supplies.maxAmount = readPrice(node, "maxAmount" , ONECENT, 500000, true);
	if(pro.supplies.minAmount > pro.supplies.maxAmount)
		throw std::logic_error("Profile id " + Str(pro.id) + " " + node_name + ": minAmount > maxAmount!!!");

	pro.supplies.failedTransactions = readAttribute<unsigned int>(node, "FailedTransactions", 1, 20);
}


void UniversityModelConfig::parsePRIEX(const std::string& node_name, rapidxml::xml_node<> *proNode, Profile& pro, PRIEX& priex, bool isSeller) const
{
	assert(proNode != NULL);

	rapidxml::xml_node<> *node = proNode->first_node(node_name.c_str());
	if(node == NULL)
	{
		priex.active = false;
		return;
	}

	if(isSeller)
		throw std::logic_error("Profile id " + Str(pro.id) + ": " + node_name + " can not be defined for seller profiles");

	priex.active = true;
	priex.maxFreqPerWeek = readAttribute<unsigned int>(node, "maxFreqPerWeek" , 0, 100);
	priex.minFreqPerWeek = readAttribute<unsigned int>(node, "minFreqPerWeek" , 0, 100);

	if(priex.minFreqPerWeek > priex.maxFreqPerWeek)
		throw std::logic_error("Profile id " + Str(pro.id) + " " + node_name + ": minFreqPerWeek > maxFreqPerWeek!!!");
	if(priex.minFreqPerWeek == 0 && priex.maxFreqPerWeek == 0)
		throw std::logic_error("Profile id " + Str(pro.id) + " " + node_name + ": minFreqPerWeek = maxFreqPerWeek = 0!!!");

	priex.minPrice = readPrice(node, "minPrice" , ONECENT, 500000, true);
	priex.maxPrice = readPrice(node, "maxPrice" , ONECENT, 500000, true);
	if(priex.minPrice > priex.maxPrice)
		throw std::logic_error("Profile id " + Str(pro.id) + " " + node_name + ": minPrice > maxPrice!!!");

	readAttributeDayTimes(node, priex);
	readAttributeDaysOfWeek(node, priex);

	if(node->next_sibling(node_name.c_str()) != NULL)
	{
		throw std::logic_error ("Two or more '" + node_name + "' sections defined in Profile id " + Str(pro.id));
	}
}

void UniversityModelConfig::readFromConfigFile(const std::string& fname)
{
	std::ifstream infile(fname.c_str());

	// get length of file:
	infile.seekg (0, ios::end);
	unsigned long length = infile.tellg();
	infile.seekg (0, ios::beg);

	//if(buffer != NULL) delete[] buffer;
	buffer = new char [length+1];
	// read data as a block:
	unsigned long charsReadCount = infile.readsome(buffer,length);
	assert(charsReadCount == length);
	buffer[length] = 0; // '\0';
	assert(length == strlen(buffer));
	infile.close();
	//std::cout.write(buffer, length);
	rapidxml::xml_document<> doc;    // character type defaults to char
	doc.parse<0>(buffer);   		// 0 means default parse flags
	rapidxml::xml_node<> *rootNode = doc.first_node();

	rootNode = rootNode->first_node("UniversityModel");
	if(rootNode == NULL || rootNode == 0)
		throw std::logic_error ("No 'UniversityModel' node in the XML file!");

	parseNonavailabilityTimes(rootNode, globalNonAvailabilityTimes);

	unsigned int maxEventID = 0;
	//read events
	for (rapidxml::xml_node<> *evNode = rootNode->first_node("Event");
			evNode != NULL && std::string(evNode->name()) == "Event"; evNode = evNode->next_sibling())
	{
		Event ev;
		ev.name = readAttribute<std::string>(evNode, "name");
		ev.id = readAttribute<unsigned int>(evNode, "id", 0, 1000);
		if(events.count(ev.id) > 0)
			throw std::logic_error("Event ev " + Str(ev.id) + " appears at least twice.");
		readAttributeDayTimes(evNode, ev);

		// stores
		for (rapidxml::xml_node<> *selNode = evNode->first_node("Store");
				selNode != NULL && std::string(selNode->name()) == "Store"; selNode = selNode->next_sibling())
		{
			Store sel;
			sel.name = readAttribute<std::string>(selNode, "name");
			sel.id = readAttribute<unsigned int>(selNode, "id" , 0, 1000);
			sel.eventid = ev.id;
			if(stores.count(sel.id) > 0)
				throw std::logic_error("Store id " + Str(sel.id) + " appears at least twice.");
			if(sel.id > maxEventID) maxEventID = sel.id;
			/*
			sel.maxPrice = readPrice(selNode, "maxPrice", ONECENT, 500000, true);
			sel.minPrice = readPrice(selNode, "minPrice", ONECENT, 500000, true);

			if(sel.minPrice > sel.maxPrice)
				throw std::logic_error("Store id " + Str(sel.id) + ": minPrice (" + Str(sel.minPrice) + ") larger than maxPrice (" + Str(sel.maxPrice) + ")");
			*/
			readAttributeDaysOfWeek(selNode, sel);

			ev.associatedStores.push_back(sel.id);
			stores[sel.id] = sel;
		}
		if(ev.associatedStores.size() == 0)
			throw std::logic_error("No Stores defined for Event id " + Str(ev.id));
		events[ev.id] = ev;
	}
	if(events.size() == 0)
		throw std::logic_error("No Events defined");

	//read profiles
	for (rapidxml::xml_node<> *proNode = rootNode->first_node("Profile");
			proNode != NULL && std::string(proNode->name()) == "Profile"; proNode = proNode->next_sibling())
	{
		Profile pro;
		pro.name = readAttribute<std::string>(proNode, "name");
		pro.id = readAttribute<unsigned int>(proNode, "id" , 0, 1000);
		if(profiles.count(pro.id) > 0 || sellerProfiles.count(pro.id) > 0)
			throw std::logic_error("Profile id " + Str(pro.id) + " appears at least twice.");
		pro.percentage = readAttribute<double>(proNode, "percentage", 0.0, 1.0);
		//pro.privacyAwareness = readAttribute<double>(proNode, "privacyAwareness", 0.0, 1.0);
		pro.miners = readAttribute<double>(proNode, "miners", 0.0, 1.0);
		pro.minInitialAmount = readPrice(proNode, "minInitialAmount", ONECENT, 5000000, true);
		pro.maxInitialAmount = readPrice(proNode, "maxInitialAmount", ONECENT, 5000000, true);
		pro.privacyAwarePercentage = readAttribute<double>(proNode, "privacyAwareness", 0.0, 1.0);
		pro.minInitialAddressCount = readAttribute<unsigned int>(proNode, "minInitialAddressCount" , 1, 100);
		pro.maxInitialAddressCount = readAttribute<unsigned int>(proNode, "maxInitialAddressCount" , 1, 100);

		if(pro.minInitialAmount > pro.maxInitialAmount)
			throw std::logic_error("Profile id " + Str(pro.id) + ": minInitialAmount > maxInitialAmount!!!");
		if(pro.minInitialAddressCount > pro.maxInitialAddressCount)
			throw std::logic_error("Profile id " + Str(pro.id) + ": minInitialAddressCount > maxInitialAddressCount!!!");

		unsigned int sellerAsUInt = readAttribute<unsigned int>(proNode, "isSeller" , 0, 1);
		bool isNewProfileAseller = false;
		if(sellerAsUInt > 0)
			isNewProfileAseller = true;

		parseNonavailabilityTimes(proNode, pro.nonAvailabilityTimes);

		parsePRIEX("PrivacyAwareness", proNode, pro, pro.privacyAwareness, isNewProfileAseller);
		parsePRIEX("InterExchange", proNode, pro, pro.interExchange, isNewProfileAseller);
		parseSupplies("Supplies", proNode, pro, isNewProfileAseller);

		for (rapidxml::xml_node<> *profileEvNode = proNode->first_node("ProfileEvent");
				profileEvNode != NULL && std::string(profileEvNode->name()) == "ProfileEvent"; profileEvNode = profileEvNode->next_sibling())
		{
			ProfileEvent prEvent;
			prEvent.eventid = readAttribute<unsigned int>(profileEvNode, "eventid" , 0, maxEventID);
			if(events.count(prEvent.eventid) == 0)
				throw std::logic_error("Defining ProfileEvent eventid " + Str(prEvent.eventid) + " but Event id " + Str(prEvent.eventid) + " does not exist.");
			prEvent.minFreqPerWeek = readAttribute<unsigned int>(profileEvNode, "minFreqPerWeek" , 0, 100);
			prEvent.maxFreqPerWeek = readAttribute<unsigned int>(profileEvNode, "maxFreqPerWeek" , 0, 100);

			if(prEvent.minFreqPerWeek > prEvent.maxFreqPerWeek)
				throw std::logic_error("ProfileEvent eventid " + Str(prEvent.eventid) + ": minFreqPerWeek > maxFreqPerWeek!!!");

			if(prEvent.minFreqPerWeek == 0 && prEvent.maxFreqPerWeek == 0)
				throw std::logic_error("ProfileEvent eventid " + Str(prEvent.eventid) + ": minFreqPerWeek = maxFreqPerWeek = 0!!!");

			double prefSum = 0.0;
			double maxPrefSum = 0.0;

			for (rapidxml::xml_node<> *stNode = profileEvNode->first_node("Store");
					stNode != NULL && std::string(stNode->name()) == "Store"; stNode = stNode->next_sibling())
			{
				ProbPriceRange probR;
				unsigned int storeid = readAttribute<unsigned int>(stNode, "storeid" , 0, 100);
				if(stores.count(storeid) <= 0)
					throw std::logic_error("Trying to define profile realization for Store id " + Str(storeid) + ". But Store id " + Str(storeid) + " is not present.");
				if(!(events.at(prEvent.eventid).hasStore(storeid)))
					throw std::logic_error("Trying to define profile realization for Store id " + Str(storeid) + ". But Store id " + Str(storeid) + " is does not belong to Event id " + Str(prEvent.eventid));
				if(pro.probRanges.count(std::make_pair(prEvent.eventid, storeid)) > 0)
					throw std::logic_error("Trying to define profile realization for Store id " + Str(storeid) + " and Event id " + Str(prEvent.eventid) + ". But this combination does already exist.");
				probR.minPref = readAttribute<double>(stNode, "minProb", 0.0, 100.0);
				probR.maxPref = readAttribute<double>(stNode, "maxProb", 0.0, 100.0);
				if(probR.minPref > probR.maxPref)
					throw std::logic_error("Store storeid " + Str(storeid) + ": minPref > maxPref!!!");
				probR.minPrice = readPrice(stNode, "minPrice" , ONECENT, 500000, true);
				probR.maxPrice = readPrice(stNode, "maxPrice" , ONECENT, 500000, true);
				if(probR.minPrice > probR.maxPrice)
					throw std::logic_error("Store storeid " + Str(storeid) + ": minPrice > maxPrice!!!");

				maxPrefSum += probR.maxPref;
				prefSum += probR.maxPref + probR.minPref;
				pro.probRanges[std::make_pair(prEvent.eventid, storeid)] = probR;
			}

			pro.profileEvents[prEvent.eventid] = prEvent;

			/*
			prEvent.prefSum = prefSum;
			double scalingFactor = maxPrefSum / prEvent.prefSum;

			// calculate probabilities
			for(std::map<std::pair<unsigned int, unsigned int>, ProbPriceRange>::iterator it = pro.probRanges.begin();
					it != pro.probRanges.end(); ++it)
			{
				if(it->first.first == prEvent.eventid)
				{
					it->second.minProb = ((double) it->second.minPref / (double) prEvent.prefSum) / scalingFactor;
					it->second.maxProb = ((double) it->second.maxPref / (double) prEvent.prefSum) / scalingFactor;
				}
			}
			*/

		}
		if(!isNewProfileAseller)
		{
			if(pro.profileEvents.size() == 0)
				throw std::logic_error("No profileEvent defined for Profile id " + Str(pro.id));

			for(std::map<unsigned int, Event>::const_iterator it = events.begin(); it != events.end(); ++it)
			{
				if(pro.profileEvents.count(it->first) > 0) // profile must have this event (plus, must have at least one event)
				{
					const std::vector<unsigned int>& storeIDsVec = it->second.associatedStores;
					for(unsigned int i = 0; i < storeIDsVec.size(); ++i)
					{
						if(pro.probRanges.count(std::make_pair(it->first, storeIDsVec[i])) <= 0)
						{
							throw std::logic_error("Profile id " + Str(pro.id) + " has not the correct stores defined for profileEvent eventid "
									+ Str(it->first));
						}
					}
				}
			}
		}
		else
		{
			if(pro.profileEvents.size() > 0)
							throw std::logic_error("Seller profile id " + Str(pro.id) + " should have no profileEvents defined.");
		}

		if(isNewProfileAseller)
			sellerProfiles[pro.id] = pro;
		else
			profiles[pro.id] = pro;
	}
	if(profiles.size() == 0)
		std::logic_error lerror("No Profiles (buyers) defined");
	if(sellerProfiles.size() == 0)
		std::logic_error lerror("No Profiles (sellers) defined");

	double sum = 0.0;
	//profile probabilities
	for(std::map<unsigned int, Profile>::iterator it = profiles.begin(); it != profiles.end(); ++it)
	{
		assert(it->first == it->second.id);
		profileProbabilities.push_back(std::make_pair(it->second.percentage, it->second.id));
		sum += it->second.percentage;
	}
	if(sum != 1.0) throw std::logic_error("probabilities for profiles must sum up to 1");

	sum = 0.0;
	//seller profile probabilities
	for(std::map<unsigned int, Profile>::iterator it = sellerProfiles.begin(); it != sellerProfiles.end(); ++it)
	{
		assert(it->first == it->second.id);
		sellerProfileProbabilities.push_back(std::make_pair(it->second.percentage, it->second.id));
		sum +=  it->second.percentage;
	}
	if(sum != 1.0) throw std::logic_error("probabilities for seller profiles must sum up to 1");
}

std::ostream& UniversityModelConfig::output(std::ostream& o) const
{
	o << "Source Filename: " << fileName << std::endl << std::endl;

	o << "Events:" << std::endl;
	o << events << std::endl;
	o << "Stores:" << std::endl;
	o << stores << std::endl;
	o << "Profiles: " << std::endl;
	o << profiles << std::endl;
	o << "seller Profiles: " << std::endl;
	o << sellerProfiles << std::endl;
	o << "Global non-availability times: " << std::endl;
	o << globalNonAvailabilityTimes << std::endl;
	o << "Profile prob Vector:" << std::endl;
	o << profileProbabilities << std::endl;
	o << "seller profile prob Vector:" << std::endl;
	o << sellerProfileProbabilities << std::endl;
	return o;
}

std::ostream& operator<<(std::ostream& o, const UniversityModelConfig& umconfig)
{
	return umconfig.output(o);
}

std::ostream& operator<<(std::ostream& o, const Store& store)
{
	//assert(store.minPrice > 0.0);
	//assert(store.minPrice <= store.maxPrice);
	assert(!store.name.empty());
	o << "Store[";
	o << store.id << " : ";
	o << store.name << " : ";
	//o << store.minPrice << " : ";
	//o << store.maxPrice << " : ";
	for(unsigned int i = 0; i < 7; ++i) o << store.availableDaysOfWeek[i] << ", ";
	o << "]";
	return o;
}

std::ostream& operator<<(std::ostream& o, const Event& ev)
{
	assert( (ev.startHour == ev.endHour && ev.startMin < ev.endMin) ||
			(ev.startHour < ev.endHour) );
	o << "Event[";
	o << ev.id << " : ";
	o << ev.name << " : ";
	o << ev.startHour << ":" << ev.startMin << " : ";
	o << ev.endHour << ":" << ev.endMin << " : ";
	for(unsigned int i = 0; i < ev.associatedStores.size(); ++i)
	{
		o << ev.associatedStores[i] << ", ";
	}
	o << "]";
	return o;
}

std::ostream& operator<<(std::ostream& o, const Profile& profile)
{
	o << "Profile[";
	o << profile.id << " : ";
	o << profile.name << " : ";
	o << profile.percentage << " : ";
	o << profile.miners << " : ";
	o << profile.privacyAwarePercentage << " : ";
	o << profile.minInitialAmount << " : ";
	o << profile.maxInitialAmount << " : ";
	o << profile.minInitialAddressCount << " : ";
	o << profile.maxInitialAddressCount << std::endl;
	o << "Interexchange:" << profile.interExchange << std::endl;
	o << "PrivacyAwareness: " << profile.privacyAwareness << std::endl;
	o << "Supplies: " << profile.supplies << std::endl;
	o << profile.nonAvailabilityTimes << std::endl;
	o << profile.profileEvents << std::endl;
	o << profile.probRanges;
	o << "]";
	return o;
}

std::ostream& operator<<(std::ostream& o, const Supplies& suppl)
{
	o << "[";
	o << "Active: " << suppl.active;
	if(suppl.active)
	{
		o << " : ";
		o << "Amount=[" << suppl.minAmount << ", " << suppl.maxAmount << "]";
		o << " : ";
		o << "minutes=[" << suppl.minMinutesAfterLastFailure << ", " << suppl.maxMinutesAfterLastFailure << "]";
		o << " : " << "allowedTransactionFailures=" << suppl.failedTransactions;
	}
	o << "]";
	return o;
}

std::ostream& operator<<(std::ostream& o, const PRIEX& priex)
{
	o << "PRIEX[";
	o << "Active:" << priex.active;
	if(priex.active)
	{
		o << " : ";
		o << "Freq=[" << priex.minFreqPerWeek << "," << priex.maxFreqPerWeek << "]" << " : ";
		o << priex.startHour << ":" << priex.startMin << " : ";
		o << priex.endHour << ":" << priex.endMin << " : ";
		o << "Price=";
		o << "[" << priex.minPrice << "," << priex.maxPrice << "]" << " : ";
		o << priex.availableDaysOfWeek;
	}
	o << "]";

	return o;
}


std::ostream& operator<<(std::ostream& o, const ProfileEvent& profileevent)
{
	assert(profileevent.minFreqPerWeek <= profileevent.maxFreqPerWeek);
	o << "ProfileEvent[" << profileevent.eventid << " : ";
	o << profileevent.minFreqPerWeek << " : " << profileevent.maxFreqPerWeek << "";
	//o << profileevent.prefSum;
	o << "]";
	return o;
}
