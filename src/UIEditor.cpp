#include "UIEditor.hpp"

using namespace elmt;

UIEditor::UIEditor(UIManager* uiManager) : uiMgr(uiManager), isInitialized(false)
{
}

UIEditor::~UIEditor() {}

void UIEditor::Update()
{
	if (!isInitialized)
	{
		SetupComponents();
		isInitialized = true;
	}
	else
	{
		UpdateComponents();
	}
}

void UIEditor::UpdateComponents()
{
	int currentWidth, currentHeight;
	glfwGetWindowSize(core::getWindow(), &currentWidth, &currentHeight);

	// Update the location of the main window
	if (mainWindowComponent)
	{
		mainWindowComponent->SetPos(ImVec2(0, 0));
		mainWindowComponent->SetSize(ImVec2(480, currentHeight));
	}

	// update the location of the inspector window
	if (inspectorWindowComponent)
	{
		inspectorWindowComponent->SetPos(ImVec2(currentWidth - 480, 0));
		inspectorWindowComponent->SetSize(ImVec2(480, currentHeight));
	}

	for (auto& comp : compHeaders)
	{
		comp.DynamicUpdate();
	}

}


void UIEditor::SetupComponents()
{
	int width, height;
	glfwGetWindowSize(core::getWindow(), &width, &height);

	Entity* rootEntity = core::rootEntity;
	// Main window
	UIEntity* wndEntity = new UIEntity("leftWindow", uiMgr);
	mainWindowComponent = new UIWindowComponent("leftWindow", wndEntity, false, "Main Window",
		ImVec2(0, 0), ImVec2(480, height), ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

	// Inspector window
	UIEntity* inspectorWindowEntity = new UIEntity("inspectorWindow", uiMgr);
	inspectorWindowComponent = new UIWindowComponent("inspectorWindow", inspectorWindowEntity, false,
		"Inspector Window", ImVec2(width - 480, 0), ImVec2(480, height), ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

	UISeparatorComponent* sTextColorComponent = new UISeparatorComponent("sTextColor", wndEntity, "Text Color", false);

	UITextColorEditorComponent* colorEditorComponent = new UITextColorEditorComponent("colorEditor", wndEntity, false);

	UIPushColorComponent* pushColorComponent = new UIPushColorComponent("pushColor", wndEntity, false);

	// Entity Tree
	UITreeNodeComponent* treeComponent = new UITreeNodeComponent("tree", wndEntity, false, rootEntity);
	treeComponent->callbackClick = [this](UITreeNodeComponent* node, Entity* entity)
		{
			this->onTreeNodeClick(node, entity);
		};

	// Light
	UISpacingComponent* spacingComponent1 = new UISpacingComponent("spacing", wndEntity, false);

	UISeparatorComponent* sLightComponent = new UISeparatorComponent("sLight", wndEntity,
		"Directional Light Intensity", false);

	UISliderComponent* sliderLightComponent = new UISliderComponent("##sliderLight", wndEntity, false);
	sliderLightComponent->text = "slider";
	sliderLightComponent->currentValue = 100;
	sliderLightComponent->maxValue = 100;
	sliderLightComponent->minValue = 1;
	sliderLightComponent->callbackValueChanged = [this](UISliderComponent* lightSlider)
		{
			this->onLightChanged(lightSlider);
		};

	// Volumn
	UISpacingComponent* spacingComponent2 = new UISpacingComponent("spacing2", wndEntity, false);

	UISeparatorComponent* sVolumnComponent = new UISeparatorComponent("sVolumn", wndEntity, "Audio", false);

	UISliderComponent* sliderVolumnComponent = new UISliderComponent("sliderVolumn", wndEntity, false);
	sliderVolumnComponent->text = "Global Volumn";
	sliderVolumnComponent->currentValue = 100;
	sliderVolumnComponent->maxValue = 100;
	sliderVolumnComponent->minValue = 1;
	sliderVolumnComponent->callbackValueChanged = [this](UISliderComponent* volumnSlider)
		{
			this->onVolumnChanged(volumnSlider);
		};

	UISpacingComponent* spacingComponent3 = new UISpacingComponent("spacing3", wndEntity, false);

	UIButtonComponent* btnStartComponent = new UIButtonComponent("btnPlay", wndEntity, "Play", false);
	btnStartComponent->callbackClick = [this](UIButtonComponent* btnPlay)
		{
			this->onPlayButtonClick(btnPlay);
		};

	UISamelineComponent* samelineComponent1 = new UISamelineComponent("sameline1", wndEntity, false);

	UIButtonComponent* btnStopComponent = new UIButtonComponent("btnStop", wndEntity, "Stop", false);
	btnStopComponent->callbackClick = [this](UIButtonComponent* btnStop)
		{
			this->onStopButtonClick(btnStop);
		};

	// Render Mode
	UISpacingComponent* spacingComponent15 = new UISpacingComponent("spacing15", wndEntity, false);

	UISeparatorComponent* sModeComponent = new UISeparatorComponent("sMode", wndEntity, "Render Mode", false);

	UIComboComponent* modeComboComponent = new UIComboComponent("modeCombo", wndEntity, false);
	const char* renderModes[] = {
		"No Debug", "Normals", "Ambient Occlusion", "Environment Light",
		"RGBA Texture", "Shadows Only", "Mipmap Levels", "Blinn Phong Shading",
		"Diffuse Only", "Specular Only", "Ambient Only", "Roughness Only", "Metallicness Only", "Parallax Only"
	};
	modeComboComponent->SetItems(renderModes, IM_ARRAYSIZE(renderModes));
	modeComboComponent->callbackItemSelected = [this](UIComboComponent* modeCombo)
		{
			this->onRenderModesChanged(modeCombo);
		};


	// UI scaling
	UISpacingComponent* spacingComponent4 = new UISpacingComponent("spacing4", wndEntity, false);

	UISeparatorComponent* sScaleComponent = new UISeparatorComponent("sScale", wndEntity, "UI Scaling", false);

	UISliderComponent* sliderScale1Component = new UISliderComponent("sliderScale1", wndEntity, false);
	sliderScale1Component->text = "Current Window Scaling";
	sliderScale1Component->currentValue = 100;
	sliderScale1Component->maxValue = 200;
	sliderScale1Component->minValue = 30;
	sliderScale1Component->callbackValueChanged = [this](UISliderComponent* sliderScale)
		{
			this->onWindowScaleChanged(sliderScale);
		};

	UISpacingComponent* spacingComponent5 = new UISpacingComponent("spacing5", wndEntity, false);

	UISliderComponent* sliderScale2Component = new UISliderComponent("sliderScale2", wndEntity, false);
	sliderScale2Component->text = "Global Scaling";
	sliderScale2Component->currentValue = 100;
	sliderScale2Component->maxValue = 200;
	sliderScale2Component->minValue = 30;
	sliderScale2Component->callbackValueChanged = [this](UISliderComponent* globalScale)
		{
			//this->onGlobalScaleChanged(globalScale);

			ImGuiIO& io = ImGui::GetIO();
			io.FontGlobalScale = globalScale->currentValue / 100.0f;
		};


	// Rounding
	UISpacingComponent* spacingComponent6 = new UISpacingComponent("spacing6", wndEntity, false);

	UISeparatorComponent* sRoundComponent = new UISeparatorComponent("sRound", wndEntity, "Corner Radius", false);

	UISliderComponent* sliderRoundComponent = new UISliderComponent("##sliderRound", wndEntity, false);
	sliderRoundComponent->text = "Controls' Corner Radius";
	sliderRoundComponent->currentValue = 0;
	sliderRoundComponent->maxValue = 100;
	sliderRoundComponent->minValue = 0;
	sliderRoundComponent->callbackValueChanged = [this](UISliderComponent* sliderRound)
		{
			//this->onFrameRoundingChanged(sliderRound);

			ImGuiStyle style = ImGui::GetStyle();
			float roundingScale = 0.1f;
			style.FrameRounding = sliderRound->currentValue * roundingScale;
			ImGui::GetStyle() = style; // make sure style changes are applied
		};

	UISliderComponent* sliderWinRoundComponent = new UISliderComponent("sliderWinRound", wndEntity, false);
	sliderWinRoundComponent->text = "Window Corner Radius";
	sliderWinRoundComponent->currentValue = 0;
	sliderWinRoundComponent->maxValue = 100;
	sliderWinRoundComponent->minValue = 0;
	sliderWinRoundComponent->callbackValueChanged = [this](UISliderComponent* sliderWinRound)
		{
			//this->onWindowRoundingChanged(sliderWinRound);

			ImGuiStyle style = ImGui::GetStyle();
			float roundingScale = 0.1f;
			style.WindowRounding = sliderWinRound->currentValue * roundingScale;
			ImGui::GetStyle() = style;
		};


	// Alpha
	UISpacingComponent* spacingComponent7 = new UISpacingComponent("spacing7", wndEntity, false);

	UISeparatorComponent* sAlphaComponent = new UISeparatorComponent("sAlpha", wndEntity, "Global Alpha", false);

	ImGuiStyle style = ImGui::GetStyle();
	UISliderComponent* sliderAlphaComponent = new UISliderComponent("##sliderAlpha", wndEntity, false);
	sliderAlphaComponent->currentValue = 100;
	sliderAlphaComponent->maxValue = 100;
	sliderAlphaComponent->minValue = 30;
	sliderAlphaComponent->callbackValueChanged = [this](UISliderComponent* sliderAlpha)
		{
			//this->onGlobalAlphaChanged(sliderAlpha);

			ImGuiStyle style = ImGui::GetStyle();
			style.Alpha = sliderAlpha->currentValue / 100.0f;
			ImGui::GetStyle() = style;
		};

	// Item Spacing
	UISpacingComponent* spacingComponent8 = new UISpacingComponent("spacing8", wndEntity, false);

	UISeparatorComponent* sSpaceComponent = new UISeparatorComponent("sSpace", wndEntity, "Item Spacing", false);

	UISliderComponent* sliderSpaceComponent1 = new UISliderComponent("##sliderSpace1", wndEntity, false);
	sliderSpaceComponent1->currentValue = 0;
	sliderSpaceComponent1->maxValue = 20;
	sliderSpaceComponent1->minValue = 0;
	sliderSpaceComponent1->callbackValueChanged = [this](UISliderComponent* sliderSpace1)
		{
			//this->onItemSpacingChanged1(sliderSpace1);

			ImGuiStyle style = ImGui::GetStyle();
			style.ItemSpacing.x = sliderSpace1->currentValue * 10.0f;
			ImGui::GetStyle() = style;
		};

	UISliderComponent* sliderSpaceComponent2 = new UISliderComponent("##sliderSpace2", wndEntity, false);
	sliderSpaceComponent2->currentValue = 0;
	sliderSpaceComponent2->maxValue = 20;
	sliderSpaceComponent2->minValue = 0;
	sliderSpaceComponent2->callbackValueChanged = [this](UISliderComponent* sliderSpace2)
		{
			//this->onItemSpacingChanged2(sliderSpace2);

			ImGuiStyle style = ImGui::GetStyle();
			style.ItemSpacing.y = sliderSpace2->currentValue * 10.0f;
			ImGui::GetStyle() = style;
		};


	// Item Inner Spacing
	UISpacingComponent* spacingComponent9 = new UISpacingComponent("spacing9", wndEntity, false);

	UISeparatorComponent* sInnerComponent = new UISeparatorComponent("sInner", wndEntity, "Item Inner Spacing", false);

	UISliderComponent* sliderInnerComponent = new UISliderComponent("##sliderInner", wndEntity, false);
	sliderInnerComponent->currentValue = 0;
	sliderInnerComponent->maxValue = 20;
	sliderInnerComponent->minValue = 0;
	sliderInnerComponent->callbackValueChanged = [this](UISliderComponent* sliderInner1)
		{
			//this->onItemInnerSpacingChanged1(sliderInner1);

			ImGuiStyle style = ImGui::GetStyle();
			style.ItemInnerSpacing.x = sliderInner1->currentValue * 10.0f;
			ImGui::GetStyle() = style;
		};

	// Indentation
	UISpacingComponent* spacingComponent10 = new UISpacingComponent("spacing10", wndEntity, false);

	UISeparatorComponent* sIndentComponent = new UISeparatorComponent("sIndent", wndEntity, "Indentation", false);

	UISliderComponent* sliderIndentComponent = new UISliderComponent("##sliderIndent", wndEntity, false);
	sliderIndentComponent->currentValue = 0;
	sliderIndentComponent->maxValue = 20;
	sliderIndentComponent->minValue = 0;
	sliderIndentComponent->callbackValueChanged = [this](UISliderComponent* sliderIndent1)
		{
			//this->onIndentSpacingChanged(sliderIndent1);

			ImGuiStyle style = ImGui::GetStyle();
			style.IndentSpacing = sliderIndent1->currentValue * 10.0f;
			ImGui::GetStyle() = style;
		};


	// UI theme
	UISpacingComponent* spacingComponent11 = new UISpacingComponent("spacing11", wndEntity, false);

	UISeparatorComponent* sThemeComponent = new UISeparatorComponent("sTheme", wndEntity, "UI Theme", false);

	UIComboComponent* comboThemeComponent = new UIComboComponent("##comboTheme", wndEntity, false);
	const char* classic[] = {
		"Dark", "Light", "Classic"
	};
	comboThemeComponent->SetItems(classic, IM_ARRAYSIZE(classic));
	comboThemeComponent->callbackItemSelected = [this](UIComboComponent* comboTheme)
		{
			this->onClassicChanged(comboTheme);
		};

	UIPopColorComponent* popColorComponent = new UIPopColorComponent("popColor", wndEntity, false);



	UIPushColorComponent* pushColorComponent1 = new UIPushColorComponent("pushColor1", inspectorWindowEntity, false);

	UISeparatorComponent* sEntityNameComponent = new UISeparatorComponent("sName", inspectorWindowEntity, "Current Selected Entity", false);


	inspectorLabel = new UILabelComponent("inspectorLabelComponent",
		inspectorWindowEntity, inspectorEntity->getName().c_str(), false);

	UISpacingComponent* spacingComponent12 = new UISpacingComponent("spacing12", inspectorWindowEntity, false);
	UISpacingComponent* spacingComponent16 = new UISpacingComponent("spacing16", inspectorWindowEntity, false);

	// POS, ROT, SCALE
	UICollapsingComponent* transCollapsComponent = new UICollapsingComponent("transCollaps", inspectorWindowEntity,
		"Transformation", inspectorWindowEntity, false);
	transCollapsComponent->callbackOnToggle = [this](UICollapsingComponent* transCollaps, Entity* currentWindow)
		{
			this->ShowCollapsingHeader(transCollaps, currentWindow);
		};

	UISpacingComponent* spacingComponent13 = new UISpacingComponent("spacing13", inspectorWindowEntity, false);

	UISpacingComponent* spacingComponent14 = new UISpacingComponent("spacing14", inspectorWindowEntity, false);

	compHeaders.clear();
	int index = 0;
	for (const auto& component : inspectorEntity->getComponents())
	{
		std::string componentName = component->getTypeName() + " #" + std::to_string(index);
		compHeaders.emplace_back(componentName.c_str(), inspectorWindowEntity,
			component->getTypeName(), component, true);
		compHeaders.back().callbackShowComponents = [this](UICollapsingComponent* collap, Entity* winEntity, Component* comp)
			{
				this->ShowComponents(collap, winEntity, comp);
			};

		std::cout << "Adding component header: " << componentName << std::endl;




		/*if (ImGui::CollapsingHeader(component->getTypeName().c_str()))
		{
			ImGui::Text(component->getTypeName().c_str());
		}

		UICollapsingComponent* transCollapsComponent = new UICollapsingComponent("transCollaps", inspectorWindowEntity,
			"Transformation", inspectorWindowEntity, false);
		transCollapsComponent->callbackOnToggle = [this](UICollapsingComponent* transCollaps, Entity* currentWindow)
		{
			this->ShowCollapsingHeader(transCollaps, currentWindow);
		};*/

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
	}

	UIPopColorComponent* popColorComponent1 = new UIPopColorComponent("popColor1", inspectorWindowEntity, false);

}

void UIEditor::onTreeNodeClick(UITreeNodeComponent* node, Entity* currentEntity)
{
	inspectorEntity = currentEntity;
	inspectorLabel->text = inspectorEntity->getName().c_str();
}

void UIEditor::onPlayButtonClick(UIButtonComponent* btn)
{
	if (testSoundPointer != nullptr && !testSoundPointer->isPlaying())
	{
		testSoundPointer->Play();
	}
}

void UIEditor::onStopButtonClick(UIButtonComponent* btn)
{
	if (testSoundPointer != nullptr && testSoundPointer->isPlaying())
	{
		testSoundPointer->Stop();
	}
}

void UIEditor::onLightChanged(UISliderComponent* slider)
{
	globalDirectionalLight1->intensity = (float)slider->currentValue / 100.0f;
}

void UIEditor::onVolumnChanged(UISliderComponent* slider)
{
	// Get the current value of the slider and converts it to a floating point ratio of the volume
	float volume = static_cast<float>(slider->currentValue) / static_cast<float>(slider->maxValue);
	core::getAudioManager()->setGlobalVolume(volume);
}

void UIEditor::onRenderModesChanged(UIComboComponent* combo)
{
	// get the index of the currently selected rendering mode
	int currentIndex = combo->currentItem;

	// set the rendering mode according to the currently selected index
	switch (currentIndex) {
	case 0: core::getRenderManager()->renderMode = NO_DEBUG; break;
	case 1: core::getRenderManager()->renderMode = NORMALS; break;
	case 2: core::getRenderManager()->renderMode = AMBIENT_OCCLUSION; break;
	case 3: core::getRenderManager()->renderMode = ENVIRONMENT_LIGHT; break;
	case 4: core::getRenderManager()->renderMode = RGBA_TEXTURE; break;
	case 5: core::getRenderManager()->renderMode = SHADOWS_ONLY; break;
	case 6: core::getRenderManager()->renderMode = MIPMAP_LEVELS; break;
	case 7: core::getRenderManager()->renderMode = BLINN_PHONG_SHADING; break;
	case 8: core::getRenderManager()->renderMode = DIFFUSE_ONLY; break;
	case 9: core::getRenderManager()->renderMode = SPECULAR_ONLY; break;
	case 10: core::getRenderManager()->renderMode = AMBIENT_ONLY; break;
	case 11: core::getRenderManager()->renderMode = ROUGHNESS_ONLY; break;
	case 12: core::getRenderManager()->renderMode = METALLICNESS_ONLY; break;
	case 13: core::getRenderManager()->renderMode = PARALLAX_ONLY; break;
	}
}

void UIEditor::onWindowScaleChanged(UISliderComponent* slider)
{
	ImGuiIO io = ImGui::GetIO();
	ImGui::SetWindowFontScale(slider->currentValue / 100.0f);
}

void UIEditor::onGlobalScaleChanged(UISliderComponent* slider)
{
	ImGuiIO io = ImGui::GetIO();
	io.FontGlobalScale = slider->currentValue / 100.0f;  // Scale the value to 0-2.0 range
	std::cout << "Global scale changed to: " << io.FontGlobalScale << std::endl;

}

void UIEditor::onFrameRoundingChanged(UISliderComponent* slider)
{
	ImGuiStyle style = ImGui::GetStyle();
	float roundingScale = 0.1f;
	style.FrameRounding = slider->currentValue * roundingScale;
}

void UIEditor::onWindowRoundingChanged(UISliderComponent* slider)
{
	ImGuiStyle style = ImGui::GetStyle();
	float roundingScale = 0.1f;
	style.WindowRounding = slider->currentValue * roundingScale;
}

void UIEditor::onGlobalAlphaChanged(UISliderComponent* slider)
{
	ImGuiStyle style = ImGui::GetStyle();
	style.Alpha = slider->currentValue / 100.0f;
}

void UIEditor::onItemSpacingChanged1(UISliderComponent* slider)
{
	ImGuiStyle style = ImGui::GetStyle();
	style.ItemSpacing.x = slider->currentValue * 10.0f;
}

void UIEditor::onItemSpacingChanged2(UISliderComponent* slider)
{
	ImGuiStyle style = ImGui::GetStyle();
	style.ItemSpacing.y = slider->currentValue * 10.0f;
}

void UIEditor::onItemInnerSpacingChanged1(UISliderComponent* slider)
{
	ImGuiStyle style = ImGui::GetStyle();
	style.ItemInnerSpacing.x = slider->currentValue * 10.0f;
}

void UIEditor::onIndentSpacingChanged(UISliderComponent* slider)
{
	ImGuiStyle style = ImGui::GetStyle();
	style.IndentSpacing = slider->currentValue * 10.0f;
}

void UIEditor::onClassicChanged(UIComboComponent* combo)
{
	ImGuiStyle style = ImGui::GetStyle();
	static ImGuiStyle ref_saved_style;

	// get the index of the current selection
	int currentColor = combo->currentItem;

	switch (currentColor)
	{
	case 0: // Classic
		ImGui::StyleColorsDark();

		break;
	case 1: // Light
		ImGui::StyleColorsLight();
		break;
	case 2: // Dark
		ImGui::StyleColorsClassic();
		break;
	}

	// save current style
	ref_saved_style = style;
}

void UIEditor::ShowCollapsingHeader(UICollapsingComponent* collap, Entity* currentWindow)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
	static glm::vec3 rotationEuler;
	rotationEuler = glm::degrees(inspectorEntity->getRotationEuler());

	ImGui::SeparatorText("Position");
	glm::vec3 position = inspectorEntity->pos;
	if (ImGui::InputFloat3("xyz##1", &position[0])) {
		inspectorEntity->pos = position;
		// find the PhysicsComponent and update its location
		for (auto component : inspectorEntity->getComponents()) {
			if (auto physicsComponent = dynamic_cast<PhysicsComponent*>(component)) {
				physicsComponent->setOverrideEntityPos(true);
				physicsComponent->setPosition(position);
			}
		}
	}

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::SeparatorText("Rotation");

	rotationEuler = glm::degrees(inspectorEntity->getRotationEuler());
	static glm::vec3 inputEuler = rotationEuler; // used to record user-entered Euler angles

	if (ImGui::InputFloat3("xyz##3", &inputEuler[0])) {
		// converting Euler angles to quaternions
		glm::quat rotationQuat = glm::quat(glm::radians(inputEuler));

		// updating the rotation of an entity
		inspectorEntity->setRotationEuler(glm::eulerAngles(rotationQuat));

		// updating the rotation matrix of a PhysicsComponent
		for (auto component : inspectorEntity->getComponents()) {
			if (auto physicsComponent = dynamic_cast<PhysicsComponent*>(component)) {
				physicsComponent->setRotationMatrix(glm::mat4_cast(rotationQuat));
			}
		}

		// updating the displayed Euler angles
		rotationEuler = glm::degrees(inspectorEntity->getRotationEuler());
	}


	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::SeparatorText("Scaling");
	ImGui::InputFloat3("xyz##2", &inspectorEntity->scale[0]);
}

void UIEditor::ShowComponents(UICollapsingComponent* collap, Entity* currentWindow, Component* comp)
{
	ImGui::Text(comp->getTypeName().c_str());
}

void UIEditor::setDirectionalLight(DirectionalLight* l)
{
	globalDirectionalLight1 = l;
}

void UIEditor::setTestSoundPointer(SoundComponent* s)
{
	testSoundPointer = s;
}


