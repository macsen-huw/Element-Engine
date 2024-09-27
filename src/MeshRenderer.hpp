#pragma once
#include "glm.hpp"

#include "Component.hpp"
#include "Entity.hpp"
#include "RenderManager.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Model.hpp"
#include "Animator.hpp"

namespace elmt {
	class MeshRenderer : public Component	
	{
	public:
		size_t instanceID;
		Model* model = nullptr;
        std::vector<Model*> models;
        std::vector<size_t> instanceIDs;
		bool visible = true;

	public:
		MeshRenderer(); //only used for serialisation
		MeshRenderer(const char* name, Entity* entity, Model* model);
        MeshRenderer(const char* name, Entity* entity, std::vector<Model*> &models, std::vector<float> &lodPartitions);

		virtual bool Update();
		void updateInfo();

		virtual ~MeshRenderer();

		virtual void clone(Component*& clonePointer, Entity* entityToAttach);


	protected:
		size_t getInstanceId() {
			return this->instanceID;
		};
		size_t getModelId() {
			return this->model->modelID;
		};

		friend class Animator;

	private:
		static bool treeChangeCallback(Component* c);
		

	};
}

