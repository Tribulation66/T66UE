// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include <Zibra/RHI.h>

namespace Zibra::RHI
{
	class BufferInternal : public Buffer
	{
	public:
		[[nodiscard]] Type GetType() const noexcept final
		{
			return Type::Buffer;
		}
		void* metadata = nullptr;
	};
	class Texture2DInternal : public Texture2D
	{
	public:
		[[nodiscard]] Type GetType() const noexcept final
		{
			return Type::Texture2D;
		}
		void* metadata = nullptr;
	};
	class Texture3DInternal : public Texture3D
	{
	public:
		[[nodiscard]] Type GetType() const noexcept final
		{
			return Type::Texture3D;
		}
		void* metadata = nullptr;
	};

	class ComputePSOInternal : public ComputePSO
	{
	public:
		void* metadata = nullptr;
	};

	class GraphicsPSOInternal : public GraphicsPSO
	{
	public:
		void* metadata = nullptr;
	};

	class FramebufferInternal : public Framebuffer
	{
	public:
		void* metadata = nullptr;
	};

	class DescriptorHeapInternal : public DescriptorHeap
	{
	public:
		void* metadata = nullptr;
	};

	class SamplerInternal : public Sampler
	{
	public:
		void* metadata = nullptr;
	};
}	 // namespace Zibra::RHI
