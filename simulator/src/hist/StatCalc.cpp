
#include "StatCalc.h"


#include <cassert>
#include <limits>
#include <sstream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <string>
#include <iostream>


//===========================================================

std::ostream& operator<<(std::ostream& o, const PrintableStatistic& sC)
{
	return sC.output(o);
}

void operator<<(const std::string& st, const PrintableStatistic& sC)
{
	std::ofstream myfile;
	myfile.open(st.c_str());
	myfile.setf(std::ios::fixed, std:: ios::floatfield);
	myfile.precision(8);
	myfile << sC;
	myfile.close();
}

//===========================================================

std::ostream& MeanStdMinMax::outputInternal(std::ostream& o) const
{
	o << "mean=" << getMean() << " std=" << getStd() << " min=" << smallest_ << " max=" << largest_ << " count=" << getCount();
	return o;
}

//===========================================================

std::ostream& Histogram1D::outputInternal(std::ostream& o) const
{	
	o.precision(10);
	o << "# total stats: ";
	totStats_.outputInternal(o);
	o << std::endl;
	o << "#left\tright\tcount\tfraction\tmean\tstd\tvar\tmin\tmax" << std::endl;
	long counter = 0;
	if(exceedingLeft_)
	{
		o << "-inf" << "\t" << boundaryLeft_ << "\t";
		MeanStdMinMax::toHistogramFormat(o, leftBucket_, getCount());
	}
	for(std::vector<MeanStdMinMax>::const_iterator it = buckets_.begin(); it != buckets_.end(); ++it)
	{
		o << boundaryLeft_+(step_*counter) << "\t" << boundaryLeft_+(step_*(1+counter)) << "\t";
		counter++;
		MeanStdMinMax::toHistogramFormat(o, *it, getCount());
	}
	if(exceedingRight_)
	{
		o << boundaryRight_ << "\t" << "inf" << "\t";
		MeanStdMinMax::toHistogramFormat(o, rightBucket_, getCount());
	}
	return o;
}

//============================================================



//============================================================

std::ostream& TimeSeries::outputInternal(std::ostream& o) const
{
	const size_t bufSiz = 30;
	char timeBuf[bufSiz];

	//o.precision(10);
	o << "# total stats: ";
	totStats_.outputInternal(o);
	o << std::endl;
	o << "# step=" << step_ << std::endl;

	o << "#left\tright\tdateleft\tdateright\tval\tcount\tfraction\tmean\tstd\tvar\tmin\tmax" << std::endl;
	long counter = 0;
	if(exceedingLeft_)
	{
		o << "-inf" << "\t" << boundaryLeft_ << "\t"
                  << "-inf" << "\t" << gmTime(boundaryLeft_, timeBuf, bufSiz) << "\t";
		MeanStdMinMaxVal::toHistogramFormat(o, leftBucket_, getCount());
	}
	for(std::vector<MeanStdMinMaxVal>::const_iterator it = buckets_.begin(); it != buckets_.end(); ++it)
	{
		double left = boundaryLeft_+(step_*counter);
		double right = boundaryLeft_+(step_*(1.0+counter));
		o << left << "\t" << right << "\t";
		o << gmTime(left, timeBuf, bufSiz) << "\t";
		o << gmTime(right, timeBuf, bufSiz) << "\t";

		counter++;
		MeanStdMinMaxVal::toHistogramFormat(o, *it, getCount());
	}
	if(exceedingRight_)
	{
		o << boundaryRight_ << "\t" << "inf" << "\t"
                  << gmTime(boundaryRight_, timeBuf, bufSiz) << "\t" << "inf" << "\t";
		MeanStdMinMaxVal::toHistogramFormat(o, rightBucket_, getCount());
	}
	return o;
}

