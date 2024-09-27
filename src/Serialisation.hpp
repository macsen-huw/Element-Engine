#pragma once
#include <string>
#include <vector>

namespace elmt {


	class EntitySerialisationData {
		/*
		A temporary class used for cereal serialisation.
		Data is written to this class on load
		Then when all Entities and Components are loaded,
		the class data is used to hook them all up

		NOTE: this ONLY stores data which is used later
		So not like... a name
		*/
	public:

		EntitySerialisationData() ;

		std::string parentID;
		std::vector<std::string> componentIDs;
	};



	class ComponentSerialisationData {
		/*
		A temporary class used for cereal serialisation.
		Data is written to this class on load
		Then when all Entities and Components are loaded,
		the class data is used to hook them all up

		NOTE: this ONLY stores data which is used later
		So not like... a name
		*/
	public:

		ComponentSerialisationData();

	};

}