#pragma once

#include <any>
#include <map>
#include <string>
#include <iostream>

namespace elmt {

	class ObjectData
	{
		// Attributes
	public:
		std::map<std::string, std::any> data;

		// Methods
	public:
		ObjectData();

		friend std::ostream& operator<< (std::ostream& os, const ObjectData& o);
	};

}

