#pragma once

#include "core.hpp"
#include "RenderManager.hpp"
#include "UIManager.hpp"
#include "UIEntity.hpp"
#include "UILabelComponent.hpp"
#include "UIImageComponent.hpp"
#include "UIButtonComponent.hpp"
#include "UICheckboxComponent.hpp"
#include "UIWindowComponent.hpp"
#include "UISliderComponent.hpp"
#include "UIInputTextComponent.hpp"
#include "UIComboComponent.hpp"
#include "UITreeNodeComponent.hpp"
#include "UISeparatorComponent.hpp"
#include "UITextColorEditorComponent.hpp"
#include "UISpacingComponent.hpp"
#include "UISamelineComponent.hpp"
#include "UICollapsingComponent.hpp"
#include "UIInputFloat3Component.hpp"
#include "UIPushColorComponent.hpp"
#include "UIPopColorComponent.hpp"

#include "DirectionalLight.h"

namespace elmt {
	class UIEditor {
	public:
		UIEditor(UIManager* uiManager);
		~UIEditor();

		void Update();
		void UpdateComponents();
		void SetupComponents();
		void onTreeNodeClick(UITreeNodeComponent* node, Entity* currentEntity);
		void onPlayButtonClick(UIButtonComponent* btn);
		void onStopButtonClick(UIButtonComponent* btn);
		void onLightChanged(UISliderComponent* slider);
		void onRenderModesChanged(UIComboComponent* combo);
		void onVolumnChanged(UISliderComponent* slider);
		void onWindowScaleChanged(UISliderComponent* slider);
		void onGlobalScaleChanged(UISliderComponent* slider);
		void onFrameRoundingChanged(UISliderComponent* slider);
		void onWindowRoundingChanged(UISliderComponent* slider);
		void onGlobalAlphaChanged(UISliderComponent* slider);
		void onItemSpacingChanged1(UISliderComponent* slider);
		void onItemSpacingChanged2(UISliderComponent* slider);
		void onItemInnerSpacingChanged1(UISliderComponent* slider);
		void onIndentSpacingChanged(UISliderComponent* slider);
		void onClassicChanged(UIComboComponent* combo);
		void ShowCollapsingHeader(UICollapsingComponent* collap, Entity* currentWindow);
		void ShowComponents(UICollapsingComponent* collap, Entity* currentWindow, Component* comp);
		void setDirectionalLight(DirectionalLight* l);
		void setTestSoundPointer(SoundComponent* s);

	private:
		UIManager* uiMgr;
		Entity* inspectorEntity = core::rootEntity;
		UILabelComponent* inspectorLabel;

		UIWindowComponent* mainWindowComponent;
		UIWindowComponent* inspectorWindowComponent;

		DirectionalLight* globalDirectionalLight1 = nullptr;
		SoundComponent* testSoundPointer = nullptr;

		bool isInitialized;

		std::vector<UICollapsingComponent> compHeaders;

		glm::vec3 rotationEuler;
		Entity* previousEntity = nullptr;
		bool rotationUpdated = false;
	};
}



