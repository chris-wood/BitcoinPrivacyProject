#include "DisjointSets.hpp"
#include "../Address.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <signal.h>
#include <cstdlib>
#include <ctime>
#include <vector>

void printVector(const std::vector<std::string>& vec)
{
	std::cout << "[";
	for (unsigned int i = 0; i < vec.size(); ++i)
	{
		std::cout << vec[i] << ", ";
	}
	std::cout << "]" << std::endl;
}

void printVectorOfAddress(const std::vector<Address>& vec, DisjointSets<Address, AddressCompare>& sets)
{
	std::cout << "[";
	for (unsigned int i = 0; i < vec.size(); ++i)
	{
		std::cout << vec[i].toString() << ", ";
	}
	std::cout << "]" << std::endl;

	std::cout << "[";
	for (unsigned int i = 0; i < vec.size(); ++i)
	{
		std::cout << sets.getRepresentativeReference(vec[i]).toString() << ", ";
	}
	std::cout << "]" << std::endl;
}

class CPTest
{
public:
	DisjointSets<Address, AddressCompare> djs;
	CPTest():
		djs(Address(0))
	{
	}
};

int main43874387(int argc, char *argv[]) {
	std::cout << "TestCase Union Find" << std::endl;

	//DisjointSets<Address, AddressCompare> addrSets(Address(0));

	CPTest cptest;

	CPTest cptest2 = cptest;

	cptest2 = cptest;

	DisjointSets<Address, AddressCompare>& addrSets = cptest2.djs;

	Address addr1(0.0, 4);
	Address addr2(100.0, 1);
	Address addr3(20.0, 2);

	addrSets.addElement(addr1);
	addrSets.addElement(addr2);
	addrSets.addElement(addr3);

	std::cout << "Elements: " << addrSets.NumElements() << ", Sets: " << addrSets.NumSets() << std::endl;
	printVectorOfAddress(addrSets.getUnderlyingVector(), addrSets);

	addrSets.Union(addr2, addr3);

	std::cout << "Elements: " << addrSets.NumElements() << ", Sets: " << addrSets.NumSets() << std::endl;
	printVectorOfAddress(addrSets.getUnderlyingVector(), addrSets);


	addrSets.Union(addr1, addr3);

	std::cout << "Elements: " << addrSets.NumElements() << ", Sets: " << addrSets.NumSets() << std::endl;
	printVectorOfAddress(addrSets.getUnderlyingVector(), addrSets);

	/*
	DisjointSets<std::string>* strSets = new DisjointSets<std::string>();

	strSets->addElement("fdsa");
	strSets->addElement("foo");
	strSets->addElement("bar");
	//strSets->addElement("foo");

	std::cout << "Elements: " << strSets->NumElements() << ", Sets: " << strSets->NumSets() << std::endl;
	printVector(strSets->getUnderlyingVector());

	strSets->Union("fdsa", "foo");

	std::cout << strSets->getRepresentative("fdsa") << std::endl;
	std::cout << strSets->getRepresentative("foo") << std::endl;
	std::cout << strSets->getRepresentative("bar") << std::endl;

	std::cout << "Elements: " << strSets->NumElements() << ", Sets: " << strSets->NumSets() << std::endl;
	printVector(strSets->getUnderlyingVector());


	strSets->Union("foo", "bar");

	std::cout << strSets->getRepresentative("fdsa") << std::endl;
	std::cout << strSets->getRepresentative("foo") << std::endl;
	std::cout << strSets->getRepresentative("bar") << std::endl;

	std::cout << "Elements: " << strSets->NumElements() << ", Sets: " << strSets->NumSets() << std::endl;
	printVector(strSets->getUnderlyingVector());
	*/

	return 0;
}
