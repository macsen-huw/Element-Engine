#include "UIImageComponent.hpp"
#include "TextureManager.hpp"
#include "core.hpp"
#define STB_IMAGE_STATIC
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif
using namespace elmt;

UIImageComponent::UIImageComponent()
{
	typeName = "UIImageComponent";
}
UIImageComponent::UIImageComponent(const char* name, Entity* entity, bool isFixed) :UIComponent(name, entity, isFixed)
{
	typeName = "UIImageComponent";
	text = name;
}

void UIImageComponent::FixedUpdate(RectTransformComponent* rectTransform)
{
	ImGui::Image((ImTextureID)(intptr_t)textureID, ImVec2(rectTransform->size.x, rectTransform->size.y));
}

void UIImageComponent::DynamicUpdate()
{
	ImGui::Image((ImTextureID)(intptr_t)textureID, ImVec2(size.x, size.y));
}


void UIImageComponent::SetImage(const char* imageFile)
{
	int width, height, n;
	stbi_set_flip_vertically_on_load(true);
	uint8_t* textureData = stbi_load(imageFile, &width, &height, &n, 4);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	stbi_image_free(textureData);
}

