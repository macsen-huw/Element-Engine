#include "Collectable.hpp"
#include "Player.hpp"

Collectable::Collectable(glm::vec3 pos, elmt::Model* model) : GameObject( ( "Collectable" + g::getObjectID()).c_str(), g::collectablesBase, pos, model, true )
{
	if (physComp) {
		elmt::RayInfo info = g::colMan->rayCast(pos + physComp->getBVH()->root.b2.y, glm::vec3(0, -1.0, 0.0));
		if (info.hit) {
			pos = info.hitPos;
			pos += 20.0;
		}

		physComp->setPosition(pos);
		physComp->setKinematic();
	}
	
}

bool Collectable::Update()
{
	auto res = GameObject::Update();
	if (!res) {
		return false;
	}

	auto colliding = g::colMan->isIntersecting<Entity,Entity>( (Entity*)this, (Entity*)(g::player) );
	if (!colliding.entity) {
		colliding = g::colMan->isIntersecting<Entity, Entity>((Entity*)this, (Entity*)(g::player->cam));
	}

	if (colliding.entity) {
		std::cout << "Colliding!" << std::endl;
		Collect();
	}

	return true;
}

void Collectable::Collect()
{
	delete this;
}
