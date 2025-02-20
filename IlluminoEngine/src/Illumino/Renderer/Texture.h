#pragma once

#include "Illumino/Core/Core.h"

namespace IlluminoEngine
{
	class Texture2D
	{
	public:
		virtual ~Texture2D() = default;
		virtual void Bind(uint32_t slot) = 0;
		virtual uint64_t GetRendererID() = 0;

		static Ref<Texture2D> Create(const char* filepath);
		static Ref<Texture2D> Create(uint32_t width, uint32_t height, void* data);
	};
}
