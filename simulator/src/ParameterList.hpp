#ifndef PARAMETERLIST_HPP_
#define PARAMETERLIST_HPP_


#include "main.hpp"
#include "Client.hpp"
#include "Sampler.hpp"
#include "rapidxml/rapidxml.hpp"

#include <vector>
#include <ctime>
#include <string>
#include <cstring>
#include <map>
#include <iostream>
#include <cassert>
#include <fstream>
#include <sstream>


class ParameterList
{

private:
	ParameterList()
	{
	}

	~ParameterList()
	{
	}

public:
	static options_t parameters;

	static void setDefaults(options_t &params)
	{
		if(ParameterList::parameters.size() == 0)
		{
			ParameterList::parameters = params; // copy them
			for(options_t::iterator it = ParameterList::parameters.begin(); it != ParameterList::parameters.end(); ++it)
			{
				std::string key = it->first;
				std::string value = it->second;
				std::cout << "DEFAULT PARAMETER: '" << key << "' = '" << value << "'" << std::endl;
			}
		}
	}

	static void writeParametersToFile(std::ofstream& outputFile)
	{
		for(options_t::iterator it = ParameterList::parameters.begin(); it != ParameterList::parameters.end(); ++it)
		{
			std::string key = it->first;
			std::string value = it->second;
			outputFile << "PARAMETER: '" << key << "' = '" << value << "'" << std::endl;
		}
	}

	// Populate the 'parameters' map based on 'key=value' pairs from an input xml file (input stream)
	static void readParameters(ifstream& infile)
	{
		// get length of file:
		infile.seekg (0, ios::end);
		unsigned long length = infile.tellg();
		infile.seekg (0, ios::beg);

		char * buffer = new char [length+1];
		// read data as a block:
		unsigned long readCount = infile.readsome(buffer, length);
		assert(readCount == length);
		buffer[length] = 0; //'\0'; // zero terminate
		assert(length == strlen(buffer));
		//std::cout << buffer << std::endl;
		//std::cout.write(buffer, length);
		rapidxml::xml_document<> doc;    // character type defaults to char
		doc.parse<0>(buffer);   		// 0 means default parse flags
		rapidxml::xml_node<> *rootNode = doc.first_node();

		// read parameters
		for (rapidxml::xml_node<> *paramNode = rootNode->first_node("Parameter");
		     paramNode != NULL && std::string(paramNode->name()) == "Parameter"; paramNode = paramNode->next_sibling())
		{
			//std::cout << paramNode->name() << "\n";
			std::string key;
			std::string value;
			for (rapidxml::xml_attribute<> *attr = paramNode->first_attribute();
			     attr; attr = attr->next_attribute())
			{
			    //std::cout << "attribute " << attr->name() << " ";
			    //std::cout << "with value " << attr->value() << "\n";
				std::string inputStr(attr->name());
				if(inputStr == "name")
				{
					key = std::string(attr->value());
				}
				else if(inputStr == "value")
				{
					value = std::string(attr->value());
				}
			}
			std::string overriding = "";
			if(ParameterList::parameters.count(key) > 0) overriding = " (overrides previous definition)";
			ParameterList::parameters[key] = value;
			std::cout << "PARAMETER: '" << key << "' = '" << value << "'" << overriding << std::endl;
		}

		// free memory
		delete[] buffer;
		buffer = NULL;

        /*
		std::string line;
		size_t pos;
	
		for (int lineno = 1; in.good(); lineno++) {
			std::getline(in, line);

			// Ignore # comments
			if ((pos = line.find('#')) != std::string::npos) {
				line = line.substr(0, pos);
			}

			// Remove whitespace
			while ((pos = line.find(" ")) != std::string::npos)
				line.erase(line.begin() + pos);
			while ((pos = line.find("\t")) != std::string::npos)
				line.erase(line.begin() + pos);

			if(line.empty()) break;

			// Split into key/value pair at the first '='
			pos = line.find('=');
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);

			// Check that we have parsed a valid key/value pair. Warn on failure or
			// set the appropriate option on success.
			if (pos == std::string::npos) {
				std::cerr << "WARNING: skipping line " << lineno << " (no '=')" << std::endl;
			}
			else if (key.size() == 0) {
				std::cerr << "WARNING: skipping line " << lineno << " (no key)" << std::endl;
			}
			else if (value.size() == 0) {
				std::cerr << "WARNING: skipping line " << lineno << " (no value)" << std::endl;
			}
			else {
				std::string overriding = "";
				if(ParameterList::parameters.count(key) > 0) overriding = " (overrides previous definition)";
				ParameterList::parameters[key] = value; // Success!
				std::cout << "PARAMETER: '" << key << "' = '" << value << "'" << overriding << std::endl;
			}
		}
		*/
	}

	static bool parameterExists(const std::string& paramName)
	{
		return (ParameterList::parameters.count(paramName) > 0);
	}

	template <typename T>
	static void getParameterByName(const std::string &str, T &val) {
		if(!parameterExists(str))
		{
			cerr << "Parameter " << str << " does not exist";
		}
		assert(parameterExists(str));
		std::istringstream iss(ParameterList::parameters.at(str));
		iss >> val;
	}

	template <typename T>
	static T getParameterByName(const std::string &str) {
		T val;
		getParameterByName(str, val);
		return val;
	}

};

#endif /* PARAMETERLIST_HPP_ */
