#include "UIManager.hpp"
#include "UIEntity.hpp"
//#include "AudioManager.hpp"

#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <imgui.h>

#include <GLFW/glfw3.h>

#include "UIEditor.hpp"
#include "RenderManager.hpp"
#include <glm/gtc/type_ptr.hpp> 

using namespace elmt;

GLFWwindow* UIManager::s_Window = nullptr;

ImVec4 UIManager::globalTextColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // set default as white

UIManager::UIManager()
{
	init();
}

UIManager::~UIManager()
{
	shutdown();
}

void UIManager::setupWindow(GLFWwindow* window)
{
	s_Window = window;
}

UIFont* UIManager::GetFont(const char* fontFile, float fontSize)
{
	if (!fontFile) return NULL;

	for (UIFont* font : vecFonts)
	{
		if (font->fontFile == fontFile && font->fontSize == fontSize) {
			return font;
		}
	}
	UIFont* font = new UIFont;
	font->fontFile = fontFile;
	font->fontSize = fontSize;
	font->imFont = NULL;
	vecFonts.push_back(font);
	return font;
}


void UIManager::init()
{
	// Init ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontDefault();

	ImGui_ImplGlfw_InitForOpenGL(s_Window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
}

void UIManager::newFrame()
{

	for (UIFont* font : vecFonts)
	{
		if (!font->imFont) {
			font->imFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(font->fontFile.c_str(), font->fontSize);
		}
	}

	// New ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

}

void UIManager::render()
{

	Update();

	uiEditor->Update();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIManager::shutdown()
{
	// Release & unload ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void UIManager::setUIEditor(UIEditor* editor)
{
	uiEditor = editor;
}