#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include "glm.hpp"
#include "Component.hpp"
#include "Serialisation.hpp"
#include "LogType.hpp"
#include "Logger.hpp"

#include "cereal/access.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"
#include "core.hpp"



namespace elmt {
	class Entity;
	class Component;
    class core;
}


namespace elmt {
	/*
	Base class for ALL game objects
	*/
	class Entity
	{
		//Properties
	protected:


		std::string name;
		std::string uuid;


		Entity* parent;
		std::vector<Entity*> children;
		std::vector<Component*> components;


		EntitySerialisationData* serialisedData;
		// Whether this component has already been deleted (to prevent double deletes)
		bool deleted = false;

		// The position of this Entity in it's parent's local space
		glm::vec3 localPos = glm::vec3(0.0, 0.0, 0.0);

		// Updated by "updateInTree", stores whether or not this Entity is a direct or indirect child of root
		bool inTree = false;
		bool oldInTree = !inTree;

	public:
		Entity(); // Only used for serialisation TODO fix

		glm::vec3 pos = glm::vec3(0.0, 0.0, 0.0);
		glm::mat4x4 rotation = glm::mat4x4(1.f); //TODO quaternions?
		glm::vec3 scale = glm::vec3(1, 1, 1); //how scaled the entity is in x,y,z directions

		// How much Entity position/rotation has changed since last frame
		// Used for updating children
		glm::vec3 posChange = glm::vec3(0.0, 0.0, 0.0);
		glm::mat4x4 rotChange = glm::mat4x4(1.f);

		glm::vec3 oldPos = glm::vec3(0.0, 0.0, 0.0);
		glm::mat4x4 oldRot = glm::mat4x4(1.f);

		// If this is not set, the Entity will only update it's position and not it's rotation (or it's rotated position) relative to it's parent
		bool rotateWithParent = true;

		friend std::ostream& operator<< (std::ostream& os, const Entity& e);

		// Callbacks
		// Users should assign these
		bool (*callbackPreInit)(Entity* e) = nullptr; // Before Entity initialisation
		bool (*callbackInit)(Entity* e) = nullptr; // After Entity initialisation
		bool (*callbackUpdate)(Entity* e) = nullptr; // Before Entity update
		bool (*callbackRender)(Entity* e) = nullptr; // Before Entity render
		bool (*callbackDelete)(Entity* e) = nullptr; // Before Entity deletion
		bool (*callbackClone)(Entity* e, Entity* c) = nullptr; // When cloned (e is this Entity, c is the clone)
		bool (*callbackInTreeChange)(Entity* e) = nullptr; // When the object is added or removed from the tree

		//Methods
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

			// Store parent info
			std::string parentID;
			if (parent) {
				parentID = parent->uuid;
			}
			else {
				parentID = "";
			}
			archive(cereal::make_nvp("parent", parentID));

			// Store components info
			std::vector<std::string> componentUUIDs;
			for (Component* component : components) {
				componentUUIDs.push_back(component->getUUID());
			}
			archive(cereal::make_nvp("components", componentUUIDs));

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
			//entDat = (EntData*)malloc(sizeof(EntData));
			serialisedData = new EntitySerialisationData();
			//serialisedData =  (EntitySerialisationData*)malloc(sizeof(EntitySerialisationData));


			// Attributes used later
			archive(cereal::make_nvp("parent", serialisedData->parentID));
			archive(cereal::make_nvp("components", serialisedData->componentIDs));



		}
	public:
		Entity(const char* name, Entity* parent);
		Entity(const char* name, Entity* parent, glm::vec3 pos);

		template <typename T>
		T* cloneComponent(T* component, Entity* newParent) {

			T* newComponent = new T();
			component->clone(newComponent, newParent);

			//*newComponent = *component;
			return newComponent;
		}

		template <typename T>
		void clone(T*& clonePointer, const char* newName, Entity* parent)
		{
			if (!clonePointer) {
				clonePointer = new T();
			}

			*clonePointer = *((T*)this);
			clonePointer->name = newName;
			// Reset UUID
			clonePointer->uuid = core::generateUUID();

			// Copy over properties
			clonePointer->rotation = rotation;
			clonePointer->scale = scale;
			clonePointer->posChange = posChange;
			clonePointer->rotChange = rotChange;
			clonePointer->oldPos = oldPos;
			clonePointer->oldRot = oldRot;

			clonePointer->callbackPreInit;
			clonePointer->callbackInit = callbackInit;
			clonePointer->callbackUpdate = callbackUpdate;
			clonePointer->callbackRender = callbackRender;
			clonePointer->callbackDelete = callbackDelete;
			clonePointer->callbackClone = callbackClone;

			clonePointer->setParent(parent);

			// Copy all components
			clonePointer->components.clear();
			for (Component* component : components) {
				// Don't copy if it was created in Entity constructor
				const std::string typeName = component->getTypeName();
				if (clonePointer->getComponentOfType(typeName)) {
					continue;
				}

				Component* clonedComponent;
				component->clone(clonedComponent, clonePointer);
			}

			for (Entity* child : children) {
				Entity* clonedChild = nullptr;
				child->clone(clonedChild, child->name.c_str(), clonePointer);
			}

			elmt::Logger::Print("Cloned Entity (type \"" + std::string(typeid(T).name()) + "\") \"" + name + "\" into \"" + clonePointer->name + "\", new UUID: \"" + clonePointer->uuid + "\" (" + std::to_string(clonePointer->components.size()) + " components)",
				elmt::LOGCAT::CAT_CORE, elmt::LOGSEV::SEV_INFO | elmt::LOGSEV::SEV_TRACE);

			if (callbackClone) callbackClone(this, clonePointer);


		}
		template <typename T>
		void clone(T*& clonePointer, const char* newName) { return this->clone(clonePointer, newName, parent); }


		/*
		Add a new component to this Entity
		*/
		template <typename T>
		bool addComponent(T* newComponent)
		{
			auto index = std::find(components.begin(), components.end(), newComponent);
			if (index != components.end()) {
				elmt::Logger::Print("Attempted to add Component \"" + newComponent->getName() + "\" to  Entity \"" + name + "\", but it was already added",
					elmt::LOGCAT::CAT_CORE, elmt::LOGSEV::SEV_WARNING);
				return false;
			}
			components.push_back(newComponent);
			return true;
		}

		bool removeComponent(Component* component);

		bool addChild(Entity* newChild);
		bool removeChild(Entity* child);
		bool setParent(Entity* parent);

		// Get the forward (normal) vector of this Entity's rotation
		glm::vec3 getForward();
		// Get the right (tangent) vector of this Entity's rotation
		glm::vec3 getRight();
		// Get the up (bitangent) vector of this Entity's rotation
		glm::vec3 getUp();
		// Get the Entity's rotation as a vec3 XYZ of euler angles
		glm::vec3 getRotationEuler();
		// Set the Entity rotation using a vec3 XYZ of euler angles
		void setRotationEuler(glm::vec3 euler);
		// Rotate along the Entity's Up axis
		void rotateHorizontal(float amount);
		// Rotate along the Entity's Right axis
		void rotateVertical(float amount);
		// Get the position of this Entity relative to it's parent (including rotation)
		glm::vec3 getLocalPos() { return localPos; };
		// Update stored position relative to parent. Called automatically during parent->Update
		void updateLocalPos();

		// Sets inTree. Called recursively if updateChildren is set
		void updateInTree(bool val, bool updateChildren);
		virtual bool Update();
		virtual bool Render();

		void debugPrint();



		std::vector<Entity*> const& getChildren() const { return children; }
		std::vector<Entity*> getChildrenRecursive();
		void getChildrenRecursive(std::vector<Entity*>& existingChildren);
		Entity* findChild(const char* name, const char* uuid, bool recursive);

		std::vector<Component*> const& getComponents() const { return components; }

		/*
		Get the first component of this Entity that fits the typename provided
		*/
		Component* getComponentOfType(const std::string& typeName);
		/*
		get the first component of this Entity that fits the name provided
		*/
		Component* getComponentOfName(const std::string& compName);

		const std::string& getName() const { return name; };
		const std::string& getUUID() const { return uuid; };
		const std::string& getTypeName() const { return typeName; };
		const bool getDeleted() { return deleted; };
		const bool getInTree() { return inTree; }

		int finishSerialisation(const std::vector<Entity*>& entities);

		virtual ~Entity();

	protected:
		// Properties
		// Used for serialisation, MAKE SURE TO SET IN CHILDREN
		std::string typeName;

		// Methods
		bool updateComponents();

		Entity(const char* name);


	};



}

