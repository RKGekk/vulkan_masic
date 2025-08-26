#pragma once

enum RendererColorSpace {
  SRGB_LINEAR,
  SRGB_NONLINEAR
};

enum class Renderer {
	Renderer_Unknown,
	Renderer_D3D9,
	Renderer_D3D11,
	Renderer_D3D12,
	Renderer_OpenGL,
	Renderer_Vulkan
};