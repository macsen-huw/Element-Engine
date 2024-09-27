#include "ObjectData.hpp"

using namespace elmt;

ObjectData::ObjectData()
{
}

//TODO REMOVE THIS CLASS

/*
Print the Object
*/
std::ostream& operator <<(std::ostream& os, const ObjectData& o)
{

	os << "Object(){" << std::endl;


	os << "}" << std::endl;
	return os;
}