#include "GameObject.hpp"
#include "globals.hpp"

using namespace globals;

elmt::Model* GameObject::loadModel(std::string modelName)
{
    auto importedModel = g::assMan->importAsset(g::BASE_PATH, modelName);
    return importedModel;
}





void GameObject::Reset()
{
    pos = startPos;
    if (physComp) {
        physComp->setPosition(pos);
    }
}

GameObject::GameObject(const char* name, elmt::Entity* parent, glm::vec3 pos, elmt::Model* model, bool hasPhysics) : Entity(name, parent, pos), hasPhysics(hasPhysics)
{


    g::groupMan->addEntityToGroup("GameObject", this);




    meshComp = new elmt::MeshRenderer((this->name + " Renderer").c_str(), this, model);


    onClone(this, this);

    typeName = "GameObject";
}

bool GameObject::onClone(elmt::Entity* e, elmt::Entity* c)
{

    GameObject* pe = (GameObject*)e;
    GameObject* pc = (GameObject*)c;

    pc->startPos = pc->pos;
    pc->callbackClone = GameObject::onClone;

    elmt::MeshRenderer* mr = (elmt::MeshRenderer *)(pe->getComponentOfType("MeshRenderer"));

    if (mr && pe->hasPhysics) {
        pc->physComp = new elmt::PhysicsComponent((pc->name + " Physics").c_str(), pc, elmt::PhysicsPrimitives::PRIM_TRIMESH, mr->model);

       /* for (auto& mesh : mr->model->meshes)
        {
            pc->physComp->addGeom(mesh);
            
        }*/
        pc->physComp->callbackCollide = collideCallback;
        //physComp->setKinematic();
    }

    return true;
}

bool GameObject::Update()
{
    bool res = Entity::Update();
    if (!res) return false;

    if (hasPhysics && hasGravity) {
        float dt = elmt::core::getDeltaTime();
        auto gravChange = g::gravity * dt;

        //TODO change
        velocity = physComp->getLinearVelocity();

        velocity += gravChange;


        //std::cout << velocity.y << std::endl;

        //physComp->setLinearVelocity(velocity);
    }

    return true;
}


bool GameObject::collideCallback(elmt::PhysicsComponent* p1, elmt::PhysicsComponent* p2, elmt::BodyContact& contactInfo) {
    //std::cout << p1->getName() << " -> " << p2->getName() << ": " << contactInfo.pos.x << ", " << contactInfo.pos.y << ", " << contactInfo.pos.z << std::endl;

    GameObject* obj1 = (GameObject*)(p1->getEntity());
    GameObject* obj2 = (GameObject*)(p2->getEntity());

    obj1->velocity = { 0.0,0.0,0.0 };
    //p1->setLinearVelocity(obj1->velocity);

    return true;


}
