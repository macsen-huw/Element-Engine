#include "miniaudio.h"

#include "AudioManager.hpp"
#include "MicrophoneComponent.hpp"
#include "Logger.hpp"
#include "core.hpp"

using namespace elmt;

MicrophoneComponent::MicrophoneComponent() : Component()
{
	typeName = "MicrophoneComponent";
}

MicrophoneComponent::MicrophoneComponent(const char* name, Entity* entity, MicrophoneInfo info) : Component(name, entity)
{

	updateInfo(info);


	typeName = "MicrophoneComponent";
}

/*
Update the microphone's position to fit with it's entity
This is called automatically every frame (when the microphone has an entity) but can also be called manually if needed
*/
int MicrophoneComponent::updateTransform()
{
	if (!entity) {
		Logger::Print("Tried to update listener transform for MicrophoneComponent \"" + name + "\", but it had no attached Entity",
			LOGCAT::CAT_CORE, LOGSEV::SEV_ERROR);
		return 1;
	}

	ma_engine_listener_set_position(&(core::getAudioManager()->engine), listenerIndex, entity->pos.x, entity->pos.y, entity->pos.z);

	glm::vec3 forward = entity->getForward();
	ma_engine_listener_set_direction(&(core::getAudioManager()->engine), listenerIndex, forward.x, forward.y, forward.z);
	

	return 0;
}

void MicrophoneComponent::updateInfo(MicrophoneInfo newInfo)
{
	info = newInfo;
	ma_engine_listener_set_world_up(&(core::getAudioManager()->engine), listenerIndex, 0, 1, 0);
	ma_engine_listener_set_cone(&(core::getAudioManager()->engine), listenerIndex, info.innerAngle, info.outerAngle, info.outerGain);
}

bool MicrophoneComponent::Update()
{
	bool res = Component::Update();
	if (!res) return false;

	if (entity) {
		updateTransform();
	}

	return true;

}

void MicrophoneComponent::clone(Component*& clonePointer, Entity* entityToAttach)
{
	clonePointer = new MicrophoneComponent(name.c_str(), entityToAttach, info);
	Logger::Print("Cloned MicrophoneComponent " + name + ", UUID " + clonePointer->getName(), LOGCAT::CAT_CORE, LOGSEV::SEV_INFO | LOGSEV::SEV_TRACE);
}