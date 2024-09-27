#include "AnimationManager.hpp"
#include "Animator.hpp"

using namespace elmt;

void AnimationManager::AddAnimator(Animator* animator) {
	animatorInstances.emplace_back(animator);
}

void AnimationManager::UpdateAll() {
	for (Animator* animator : animatorInstances) {
		animator->UpdateAnimation(core::getDeltaTime());
	}
}
