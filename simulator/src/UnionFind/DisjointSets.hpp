// Generic Union Find Data structure by Marc Roeschlin, ETH Zurich, Spring 2012
// made generic and improved
// based on:
// Disjoint Set Data Structure
// Author: Emil Stefanov
// Date: 03/28/06
// Implementaton is as described in http://en.wikipedia.org/wiki/Disjoint-set_data_structure

#ifndef DISJOINTSETS_HPP_
#define DISJOINTSETS_HPP_ 1

#include <vector>
#include <map>
#include <iostream>
#include <cassert>
#include <functional>
#include <algorithm>
#include <limits>

// Internal Node data structure used for representing an element
template <class TYPE>
struct UNode
{
	int rank; // This roughly represents the max height of the node in its subtree
	int index; // The index of the element the node represents
	int count; // The number of children the nodes has plus one (the node itself)
	int setListPos; // position in the set list
	UNode<TYPE>* parent; // The parent node of the node
};

// forward definition
template <class TYPE, typename _Compare>
class DisjointSets;

// compare object for std::map
template <class TYPE, typename _Compare>
class PtrCompare {
public:
	PtrCompare() {};
	PtrCompare(DisjointSets< TYPE, _Compare > const* djs);
    bool operator()(const int t1, const int t2) const;
private:
    DisjointSets< TYPE, _Compare > const* m_disjointSets;
};

template <class TYPE, typename _Compare = std::less<TYPE> >
class DisjointSets
{
public:
	// Create an empty DisjointSets data structure
	explicit DisjointSets(const TYPE& defaultElement = TYPE());

    // Copy constructor
    DisjointSets(const DisjointSets<TYPE, _Compare> & s); //implemented below

	// Destructor
	~DisjointSets()
	{
		for(typename std::map<int, UNode<TYPE>*, PtrCompare<TYPE, _Compare> >::iterator it = m_storedPointers.begin();
				it != m_storedPointers.end(); ++it)
		{
			delete it->second;
		}
		/*
		for(int i = 0; i < m_numElements; ++i)
			delete m_nodes[i];
		*/
		//m_nodes.clear();
		//m_storedPointers.clear();
		//m_storedElements.clear();
	}

	//assignment operator
	const DisjointSets<TYPE, _Compare>& operator=(const DisjointSets<TYPE, _Compare>& s);

	// Returns the number of elements currently in the DisjointSets data structure.
	int NumElements() const
	{
		return m_numElements;
	}

	// Returns the number of elements currently in the DisjointSets data structure.
	int size() const
	{
		assert(m_numElements == (int) m_storedElements.size());
		assert(m_numElements == (int) m_storedPointers.size());
		return m_numElements;
	}

	// Returns the number of sets currently in the DisjointSets data structure.
	int NumSets() const
	{
		return m_numSets;
	}

	// returns the representative of the set "element" belongs to (return value is a reference!)
	TYPE& getRepresentativeReference(const TYPE& element);

	// returns the representative of the set "element" belongs to (return value is a reference!)
	const TYPE& getRepresentativeReference(const TYPE& element) const;

	// returns the representative of the set "element" belongs to (return value is a copy of the representative)
	TYPE getRepresentative(const TYPE& element) const;

	// unions the sets of two elements, arguments remain untouched
	void Union(const TYPE& element1, const TYPE& element2);

	// adds a copy of "element" to the internal map and union find structure
	int addElement(const TYPE& element);

	// adds a copy of "element" to the internal map and union find structure
	int addElementIfNotExists(const TYPE& element);

	// adds a copy of the elements in "elementVector" to the internal map and union find structure
	void addElements(const std::vector<TYPE >& elementVector);

	//return the vector for easy access. e.g. looping through etc.
	const std::vector<TYPE>& getUnderlyingVector(void) const;

	// retrieves the element from the internal storage. if index is equal to a special identifier a local element is returned
	TYPE& getElement(int pos);

	// retrieves the element from the internal storage. if index is equal to a special identifier a local element is returned
	const TYPE& getElement(int pos) const;

	// retrieves the element from the internal storage. if index is equal to a special identifier a local element is returned
	TYPE& operator[](int pos);

	// tests if element exists
	bool elementExists(const TYPE& element) const;

	// returns a reference to the element in the data structure. the returned element represents the same (in terms of the comparer)
	TYPE& Find(const TYPE& element);

	const TYPE& Find(const TYPE& element) const;

	// returns the index/position of the given element (which must exist)
	int IndexOf(const TYPE& element) const;

	// returns the number of elements belonging to the set of 'element'
	int NumSetElements(const TYPE& element) const;

private:
	// internal functions
	//===================
	// Find the set identifier that an element currently belongs to.
	// Note: some internal data is modified for optimization even though this method is constant.
	UNode<TYPE>* FindSet(UNode<TYPE>* node) const;
	// Combine two sets into one. All elements in those two sets will share the same set id that can be retrieved using FindSet.
	void Union(UNode<TYPE>* root1, UNode<TYPE>* root2);
	// Add a specified number of elements to the DisjointSets data structure. The element id's of the new elements are numbered
	// consequitively starting with the first never-before-used elementId.
	//void AddElements(int numToAdd);
	UNode<TYPE>* getRepresentativeNode(const TYPE& element) const;
	//===================
	// internal reconstruction after copy / assignment
	//===================
	void reconstructDataStructureFromReference(const DisjointSets<TYPE, _Compare>& s);
	//===================

	int m_numElements; // the number of elements currently in the DisjointSets data structure.
	int m_numSets; // the number of sets currently in the DisjointSets data structure.
	int m_lookupConstant; //identifier for comparison of elements not added to the vector
	PtrCompare<TYPE, _Compare> m_comparer;
	mutable TYPE const* m_elementForComparison;
	std::vector<TYPE> m_storedElements;
	//std::vector<UNode<TYPE>*> m_nodes; // the list of nodes representing the elements (TODO: not needed actually)
	std::map<int, UNode<TYPE>*, PtrCompare<TYPE, _Compare> > m_storedPointers; // this is a map storing all the data pointers and linking to the union-find structure
	//std::vector<int> m_setList;
};

template <class TYPE, typename _Compare>
PtrCompare<TYPE, _Compare>::PtrCompare(DisjointSets< TYPE, _Compare > const* djs):
	m_disjointSets(djs)
{
}

template <class TYPE, typename _Compare>
bool PtrCompare<TYPE, _Compare>::operator()(const int t1, const int t2) const
{
	const TYPE& elemt1 = m_disjointSets->getElement(t1);
	const TYPE& elemt2 = m_disjointSets->getElement(t2);

	_Compare C;
   return C(elemt1, elemt2);
}

template <class TYPE, typename _Compare >
DisjointSets< TYPE, _Compare >::DisjointSets(const TYPE& defaultElement):
	m_numElements(0),
	m_numSets(0),
	m_lookupConstant(std::numeric_limits<int>::max()),
	m_comparer(PtrCompare<TYPE, _Compare>(this)),
	//m_elementForComparison(defaultElement),
	m_elementForComparison(NULL),
	m_storedPointers(std::map<int, UNode<TYPE>*, PtrCompare<TYPE, _Compare> >(m_comparer))
{
}

template <class TYPE, typename _Compare >
DisjointSets< TYPE, _Compare >::DisjointSets(const DisjointSets<TYPE, _Compare> & s):
	m_numElements(s.m_numElements),
	m_numSets(s.m_numSets),
	m_lookupConstant(s.m_lookupConstant),
	m_comparer(PtrCompare<TYPE, _Compare>(this)),
	//m_elementForComparison(s.m_elementForComparison),
	m_elementForComparison(NULL),
	m_storedElements(s.m_storedElements),
	m_storedPointers(std::map<int, UNode<TYPE>*, PtrCompare<TYPE, _Compare> >(m_comparer))
{
	//std::cout << "copy constructor" << std::endl;
	reconstructDataStructureFromReference(s);
}

template <class TYPE, typename _Compare >
const DisjointSets<TYPE, _Compare>& DisjointSets< TYPE, _Compare >::operator=(const DisjointSets<TYPE, _Compare>& s)
{
	//std::cout << "assignment operator" << std::endl;
	if(this != &s)
	{
		for(typename std::map<int, UNode<TYPE>*, PtrCompare<TYPE, _Compare> >::iterator it = m_storedPointers.begin();
				it != m_storedPointers.end(); ++it)
		{
			delete it->second;
		}
		m_storedPointers.clear();
		/*
		for(int i = 0; i < m_numElements; ++i)
			delete m_nodes[i];
		*/
		//m_nodes.clear();
		m_storedPointers.clear();
		m_storedElements.clear();

		m_numElements = s.m_numElements;
		m_numSets = s.m_numSets;
		m_lookupConstant = s.m_lookupConstant;
		m_comparer = PtrCompare<TYPE, _Compare>(this);
		//m_elementForComparison = s.m_elementForComparison;
		m_elementForComparison = NULL;
		m_storedElements = std::vector<TYPE>(s.m_storedElements);
		m_storedPointers = std::map<int, UNode<TYPE>*, PtrCompare<TYPE, _Compare> >(m_comparer);

		reconstructDataStructureFromReference(s);
	}
	return *this;
}

template <class TYPE, typename _Compare >
void DisjointSets< TYPE, _Compare >::reconstructDataStructureFromReference(const DisjointSets<TYPE, _Compare>& s)
{
	// Copy nodes
	for(typename std::map<int, UNode<TYPE>*, PtrCompare<TYPE, _Compare> >::const_iterator it = s.m_storedPointers.begin();
			it != s.m_storedPointers.end(); ++it)
	{
		m_storedPointers[it->first] = new UNode<TYPE>(*(it->second));
	}
	/*
	m_nodes.resize(m_numElements);
	for(int i = 0; i < m_numElements; ++i)
	{
		m_nodes[i] = new UNode<TYPE>(*s.m_nodes[i]);
		m_storedPointers[i] = m_nodes[i];
	}
	*/
	// Update parent pointers to point to newly created nodes rather than the old ones
	for(typename std::map<int, UNode<TYPE>*, PtrCompare<TYPE, _Compare> >::const_iterator it = s.m_storedPointers.begin();
			it != s.m_storedPointers.end(); ++it)
	{
		if(it->second->parent != NULL)
			m_storedPointers[it->first]->parent = m_storedPointers[it->second->parent->index];
	}
	/*
	for(int i = 0; i < m_numElements; ++i)
		if(m_nodes[i]->parent != NULL)
			m_nodes[i]->parent = m_nodes[s.m_nodes[i]->parent->index];
	*/
}

template <class TYPE, typename _Compare >
const std::vector<TYPE>& DisjointSets< TYPE, _Compare >::getUnderlyingVector(void) const
{
	return m_storedElements;
}

template <class TYPE, typename _Compare >
TYPE& DisjointSets< TYPE, _Compare >::getElement(int pos)
{
	assert((pos >= 0 && pos < (int)m_storedElements.size()));
	/*if(pos == m_lookupConstant)
	{
		assert(m_elementForComparison != NULL);
		return *m_elementForComparison;
	}*/
	return m_storedElements.at(pos);
}

template <class TYPE, typename _Compare >
const TYPE& DisjointSets< TYPE, _Compare >::getElement(int pos) const
{
	assert(pos == m_lookupConstant || (pos >= 0 && pos < (int)m_storedElements.size()));
	if(pos == m_lookupConstant)
	{
		assert(m_elementForComparison != NULL);
		return *m_elementForComparison;
	}
	return m_storedElements.at(pos);
}

template <class TYPE, typename _Compare >
TYPE& DisjointSets< TYPE, _Compare >::operator[](int pos)
{
	return getElement(pos);
}

template <class TYPE, typename _Compare >
bool DisjointSets< TYPE, _Compare >::elementExists(const TYPE& element) const
{
	m_elementForComparison = &element;
	bool ret = m_storedPointers.count(m_lookupConstant) > 0;
	m_elementForComparison = NULL;
	return ret;
}

template <class TYPE, typename _Compare >
TYPE& DisjointSets< TYPE, _Compare >::Find(const TYPE& element)
{
	assert(elementExists(element));
	m_elementForComparison = &element;
	int indexOfElement = m_storedPointers.at(m_lookupConstant)->index;
	m_elementForComparison = NULL;
	return m_storedElements[indexOfElement];
}

template <class TYPE, typename _Compare >
const TYPE& DisjointSets< TYPE, _Compare >::Find(const TYPE& element) const
{
	assert(elementExists(element));
	m_elementForComparison = &element;
	int indexOfElement = m_storedPointers.at(m_lookupConstant)->index;
	m_elementForComparison = NULL;
	return m_storedElements[indexOfElement];
}

template <class TYPE, typename _Compare >
int DisjointSets< TYPE, _Compare >::IndexOf(const TYPE& element) const
{
	assert(elementExists(element));
	m_elementForComparison = &element;
	int indexOfElement = m_storedPointers.at(m_lookupConstant)->index;
	m_elementForComparison = NULL;
	return indexOfElement;
}

template <class TYPE, typename _Compare >
int DisjointSets< TYPE, _Compare >::NumSetElements(const TYPE& element) const
{
	assert(elementExists(element));
	UNode<TYPE>* representativeNode = getRepresentativeNode(element);
	return representativeNode->count;
}

// returns the representative of the set "element" belongs to (return value is a reference!)
template <class TYPE, typename _Compare >
TYPE& DisjointSets< TYPE, _Compare >::getRepresentativeReference(const TYPE& element)
{
	assert(elementExists(element));
	UNode<TYPE>* representativeNode = getRepresentativeNode(element);
	return m_storedElements.at(representativeNode->index);
}

// returns the representative of the set "element" belongs to (return value is a reference!)
template <class TYPE, typename _Compare >
const TYPE& DisjointSets< TYPE, _Compare >::getRepresentativeReference(const TYPE& element) const
{
	assert(elementExists(element));
	UNode<TYPE>* representativeNode = getRepresentativeNode(element);
	return m_storedElements.at(representativeNode->index);
}

// returns the representative of the set "element" belongs to (return value is a copy!)
template <class TYPE, typename _Compare >
TYPE DisjointSets< TYPE, _Compare >::getRepresentative(const TYPE& element) const
{
	assert(elementExists(element));
	UNode<TYPE>* representativeNode = getRepresentativeNode(element);
	return m_storedElements.at(representativeNode->index);
}

// unions the sets of two elements, arguments remain untouched
template <class TYPE, typename _Compare >
void DisjointSets< TYPE, _Compare >::Union(const TYPE& element1, const TYPE& element2)
{
	assert(elementExists(element1));
	assert(elementExists(element2));

	UNode<TYPE>* repr1 = getRepresentativeNode(element1);
	UNode<TYPE>* repr2 = getRepresentativeNode(element2);
	Union(repr1, repr2);

	// check if they are fused now
	TYPE& DBGref1 = getRepresentativeReference(element1);
	TYPE& DBGref2 = getRepresentativeReference(element2);
	assert(&DBGref1 == &DBGref2);
	// double check
	UNode<TYPE>* DBGrepr1 = getRepresentativeNode(element1);
	UNode<TYPE>* DBGrepr2 = getRepresentativeNode(element2);
	assert(DBGrepr1 == DBGrepr2);
}

// adds a copy of "element" to the internal map and union find structure iff it is not already there
template <class TYPE, typename _Compare >
int DisjointSets< TYPE, _Compare >::addElementIfNotExists(const TYPE& element)
{
	if(!elementExists(element))
	{
		return addElement(element);
	}
	else
	{
		return IndexOf(element);
	}
}

// adds a copy of "element" to the internal map and union find structure
template <class TYPE, typename _Compare >
int DisjointSets< TYPE, _Compare >::addElement(const TYPE& element)
{
	int returnedPosition;

	assert(!elementExists(element));

	m_storedElements.push_back(element);

	UNode<TYPE>* node = new UNode<TYPE>;
	node->count = 1;
	node->parent = NULL;
	node->rank = 0;
	node->index = m_numElements;
	//m_nodes.push_back(node);

	m_storedPointers[m_numElements] = node;

	returnedPosition = m_numElements;

	m_numElements++;
	m_numSets++;

	return returnedPosition;
}

// adds a copy of the elements in "elementVector" to the internal map and union find structure
template <class TYPE, typename _Compare >
void DisjointSets< TYPE, _Compare >::addElements(const std::vector<TYPE >& elementVector)
{
	for(unsigned int i = 0; i < elementVector.size(); ++i)
	{
		addElement(elementVector[i]);
	}
}






// internal functions from here on
// ================================================================================================================

// finds the node representing an element
template <class TYPE, typename _Compare >
UNode<TYPE>* DisjointSets< TYPE, _Compare >::getRepresentativeNode(const TYPE& elementArg) const
{
	// -> we have to use .at(), because [] creates the object if it is not found! we do not want that
	m_elementForComparison = &elementArg;
	UNode<TYPE>* assocNode = m_storedPointers.at(m_lookupConstant);
	m_elementForComparison = NULL;
	UNode<TYPE>* representativeNode = FindSet(assocNode);
	return representativeNode;
}

// Note: some internal data is modified for optimization nevertheless this method remains constant.
template <class TYPE, typename _Compare >
UNode<TYPE>* DisjointSets< TYPE, _Compare >::FindSet(UNode<TYPE>* node) const
{
	UNode<TYPE>* curNode = node;
	while(curNode->parent != NULL)
		curNode = curNode->parent;
	UNode<TYPE>* root = curNode;

	// Walk to the root, updating the parents of `elementId`. Make those elements the direct
	// children of `root`. This optimizes the tree for future FindSet invocations.
	curNode = node;
	while(curNode != root)
	{
		UNode<TYPE>* next = curNode->parent;
		curNode->parent = root;
		curNode = next;
	}
	return root;
}

// union two sets
template <class TYPE, typename _Compare >
void DisjointSets< TYPE, _Compare >::Union(UNode<TYPE>* root1, UNode<TYPE>* root2)
{
	assert(root1->parent == NULL && root2->parent == NULL);

	if(root1 == root2) return;

	UNode<TYPE>* set1 = root1;
	UNode<TYPE>* set2 = root2;

	// Determine which node representing a set has a higher rank. The node with the higher rank is
	// likely to have a bigger subtree so in order to better balance the tree representing the
	// union, the node with the higher rank is made the parent of the one with the lower rank and
	// not the other way around.
	if(set1->rank > set2->rank)
	{
		set2->parent = set1;
		set1->count += set2->count;
	}
	else if(set1->rank < set2->rank)
	{
		set1->parent = set2;
		set2->count += set1->count;
	}
	else // set1->rank == set2->rank
	{
		set2->parent = set1;
		++set1->rank; // update rank
		set1->count += set2->count;
	}
	// Since two sets have fused into one, there is now one less set so update the set count.
	--m_numSets;
}


#endif /* DISJOINTSETS_HPP_ */
