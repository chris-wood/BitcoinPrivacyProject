#ifndef UNIVERSITYMODELCLIENTSELECTOR_HPP_
#define UNIVERSITYMODELCLIENTSELECTOR_HPP_

#include "UniversityModelTimeStepping.hpp"
#include "UniversityModelClient.hpp"

class UniversityModelClientSelector {
public:
	explicit UniversityModelClientSelector(const UniversityModelClient& client):
		umcCopy_(new UniversityModelClient(client)),
		storedAcopy_(true),
		umcPosition_(0),
		timestepping_(NULL)
	{
	}

	UniversityModelClientSelector(unsigned int umcPos, UniversityModelTimeStepping* timestepping):
		umcCopy_(NULL),
		storedAcopy_(false),
		umcPosition_(umcPos),
		timestepping_(timestepping)
	{
	}

	UniversityModelClient& getClientRepresentation(void)
	{
		if(storedAcopy_)
			return *umcCopy_;

		return timestepping_->getClientRepresentation(umcPosition_);
	}

	const UniversityModelClient& getClientRepresentation(void) const
	{
		if(storedAcopy_)
			return *umcCopy_;

		return timestepping_->getClientRepresentation(umcPosition_);
	}

protected:
	virtual ~UniversityModelClientSelector()
	{
		if(umcCopy_ != NULL) delete umcCopy_;
	}

private:
	explicit UniversityModelClientSelector(const UniversityModelClientSelector& selector);
	UniversityModelClientSelector();

	UniversityModelClient* umcCopy_;
	bool storedAcopy_;
	unsigned int umcPosition_;
	UniversityModelTimeStepping* timestepping_;
};

#endif /* UNIVERSITYMODELCLIENTSELECTOR_HPP_ */
