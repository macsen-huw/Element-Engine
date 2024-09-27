#pragma once
#include <string>
#include "Entity.hpp"

#include "cereal/archives/json.hpp"
#include "cereal/access.hpp"
#include "Serialisation.hpp"

 namespace elmt{
    class Component;
    class Entity;


    class Component {
        // Properties
    protected:

        std::string name;
        std::string uuid;
        Entity *entity;
		ComponentSerialisationData* serialisedData;
        // Whether this component has already been deleted (to prevent double deletes)
        bool deleted = false;

        // Methods
        friend class cereal::JSONInputArchive;
        friend class cereal::access;
        friend class core;


		template <class Archive>
		void save(Archive& archive) const
		{
			// Store attributes
			archive(
				cereal::make_nvp("name", name),
				cereal::make_nvp("uuid", uuid),
				cereal::make_nvp("typeName", typeName)
			);

		}

		template <class Archive>
		void load(Archive& archive)
		{
			//auto e = new EntitySerialisationData();
			// Load attributes
			archive(
				cereal::make_nvp("name", name),
				cereal::make_nvp("uuid", uuid),
				cereal::make_nvp("typeName", typeName)
			);

			// Make sure to delete this later
			serialisedData = new ComponentSerialisationData();


			// Attributes used later
			// Unused for now for components
			// Entities store the ID of the components they use

		}
    public:
        Component() ; // Only used for serialisation TODO fix

        virtual bool Update();

        /*
        Clone this component.
        This should be implemented for each component type
        */ 
        virtual void clone(Component*& clonePointer, Entity* entityToAttach);

        Entity* getEntity() { return entity; }

        // Set the entity this component is connect to
        void setEntity(Entity* ent);
        const std::string &getName() const { return name; };
        const std::string &getUUID() const { return uuid; };
        const std::string& getTypeName() const { return typeName; };
        const bool getDeleted() { return deleted; };

        bool (*callbackUpdate)(Component* c) = nullptr; // Before Component update
        bool (*callbackDelete)(Component* c) = nullptr; // Before Component deletion
        bool (*callbackInTreeChange)(Component* c) = nullptr; // When the Component is added or removed from the tree

        friend std::ostream &operator<<(std::ostream &os, const Component &c);

		int finishSerialisation(const std::vector<Component*>& entities);

        //virtual std::unique_ptr<Component> clone() const = 0;

        virtual ~Component();

    protected:
        Component(const char *name, Entity *entity);
        Component(const char *name) : Component(name, nullptr) {};


        // Used for serialisation, MAKE SURE TO SET IN CHILDREN
        std::string typeName;

    };

}


