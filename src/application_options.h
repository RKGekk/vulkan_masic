#pragma once

#include <string>

#include <pugixml.hpp>

#include "engine/renderer_enum.h"

struct ApplicationOptions {
	Renderer Renderer = Renderer::Renderer_Vulkan;
    RendererColorSpace ColorSpace = RendererColorSpace::SRGB_NONLINEAR;
	bool RunFullSpeed = false;
	bool FullScreen = false;
	bool FullScreenMax = true;
	bool ScreenTearing = false;
	int ScreenWidth = 800;
	int ScreenHeight = 600;
	bool DebugUI = true;

    std::string WindowTitle = "Vulkan Test";
    std::string AppName = "Hello Triangle";
    std::string Engine_Name = "No Engine";

	float SoundEffectsVolume = 1.0f;
	float MusicVolume = 1.0f;

	pugi::xml_node RootNode;

	bool Init(const std::string& xmlFilePath);
	float GetAspect() const;
};