#pragma once

#include <vector>
#include "Entity.hpp"
#include "UIEntity.hpp"
#include <imgui.h>
#include "core.hpp"
#include "DirectionalLight.h"
#include "SoundComponent.hpp"

struct GLFWwindow;
namespace elmt
{
	struct UIFont {
		std::string fontFile;
		float fontSize;
		ImFont* imFont;
	};

	class UIEditor;
	class UIManager : public Entity {
	public:
		UIManager();
		virtual ~UIManager();

		static void setupWindow(GLFWwindow* window);

		UIFont* GetFont(const char* fontFile, float fontSize);

		Entity* inspectorEntity = core::rootEntity;
		static ImVec4 globalTextColor;

		void setUIEditor(UIEditor* editor);

		friend class RenderManager;

		void init();
		void newFrame();
		void render();
		void shutdown();

	private:
		std::vector< UIFont*> vecFonts;

		static GLFWwindow* s_Window;

		void CreateEntityTree(Entity* entity, ImGuiTreeNodeFlags flags);

		UIEditor* uiEditor = nullptr;
	};
}
