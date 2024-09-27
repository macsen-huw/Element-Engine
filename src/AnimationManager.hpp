#pragma once
#include "vector"
#include "element.hpp"
#include "core.hpp"

namespace elmt {
	class Animator;

	class AnimationManager {
	public:
		struct AnimationStateMachine {
			// TODO: implement this
		};

	private:
		std::vector<Animator*> animatorInstances;

	public:
		AnimationManager() {};

		void UpdateAll();

		~AnimationManager() {};

	protected:
		void AddAnimator(Animator* animator);
		
		friend class Animator;
	};
}

