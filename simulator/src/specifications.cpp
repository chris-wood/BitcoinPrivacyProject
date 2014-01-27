#include "main.hpp"
#include <iostream>
#include <sstream>


template <>
std::string Str<Coin>( const Coin & t ) {
   std::ostringstream os;
   t.outputBTC(os);
   return os.str();
}

template <>
std::string Str<btc>( const btc & t ) {
	std::ostringstream os;
	btc decimals = (t%100);
	os << (t/100) << '.';
	if(decimals<10) os << '0';
	os << decimals;
	return os.str();
}

std::ostream& operator<<(std::ostream& o, const Coin& coin)
{
	coin.outputBTC(o);
	return o;
}

template <>
std::ostream& operator<<(std::ostream& o, const std::vector<btc>& vec)
{
	o << "[";
	for(unsigned int i = 0; i < vec.size(); ++i)
	{
		o << Str(vec[i]);
		if(i < (vec.size()-1)) o << ", ";
	}
	o << "]";
	return o;
}
