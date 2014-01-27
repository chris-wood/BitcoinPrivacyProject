#ifndef STATISTICSCALCULATOR_HPP_
#define STATISTICSCALCULATOR_HPP_

#include <sstream>
#include <vector>

class StatisticsCalculator {
public:
	virtual ~StatisticsCalculator();
	void addNewValue(double newVal);
	virtual std::ostream& output(std::ostream& o) const = 0;
protected:
	virtual void addValue(double newVal) = 0;
	StatisticsCalculator(std::string& name);
	std::string name_;
	unsigned int count_;
};

class MeanStd : public StatisticsCalculator {
public:
	MeanStd(std::string& name);
	virtual ~MeanStd();
	void addValue(double newVal);
	std::ostream& output(std::ostream& o) const;
private:
	double mean_;
	double M2_;
};

class MinAndMax : public StatisticsCalculator {
public:
	MinAndMax(std::string& name);
	virtual ~MinAndMax();
	void addValue(double newVal);
	std::ostream& output(std::ostream& o) const;
protected:
	double smallest_;
	double largest_;
};

class HistogramAndMedian : public StatisticsCalculator {
public:
	HistogramAndMedian(std::string& name, unsigned int buckets);
	virtual ~HistogramAndMedian();
	void addValue(double newVal);
	std::ostream& output(std::ostream& o) const;
private:
	mutable std::vector<double> elements_;
	unsigned int buckets_;
};






#endif /* STATISTICSCALCULATOR_HPP_ */
