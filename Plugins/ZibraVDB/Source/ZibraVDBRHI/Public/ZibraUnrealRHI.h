// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include <Zibra/RHI.h>
#include "ZIBRHIConfig.h"
#include "ZibraUnrealRHITypes.h"

namespace Zibra::RHI::APIUnrealRHI
{
	class ZIBRA_RHI_MODULE_API ZibraUnrealRHI final : public RHIRuntime
	{
	public:
		ZibraUnrealRHI();
		~ZibraUnrealRHI() final;

		static ZibraUnrealRHI& Get();

		ReturnCode Initialize() noexcept final;
		void Release() noexcept final;

		ReturnCode GarbageCollect() noexcept final;

		[[nodiscard]] GFXAPI GetGFXAPI() const noexcept final;

		[[nodiscard]] bool QueryFeatureSupport(Feature feature) const noexcept final;
		ReturnCode SetStablePowerState(bool enable) noexcept final;

		[[nodiscard]] ReturnCode CompileComputePSO(const ComputePSODesc& desc, ComputePSO** outPSO) noexcept final;
		[[nodiscard]] ReturnCode CompileGraphicPSO(const GraphicsPSODesc& desc, GraphicsPSO** outPSO) noexcept final;

		ReturnCode ReleaseComputePSO(ComputePSO* pso) noexcept final;
		ReturnCode ReleaseGraphicPSO(GraphicsPSO* pso) noexcept final;

		[[nodiscard]] ReturnCode RegisterBuffer(void* resourceHandle, const char* name, Buffer** outBuffer) noexcept final;
		[[nodiscard]] ReturnCode RegisterTexture2D(void* resourceHandle, const char* name, Texture2D** outTexture) noexcept final;
		[[nodiscard]] ReturnCode RegisterTexture3D(void* resourceHandle, const char* name, Texture3D** outTexture) noexcept final;
		[[nodiscard]] ReturnCode CreateBuffer(size_t size, ResourceHeapType heapType, ResourceUsage usage,
			uint32_t structuredStride, const char* name, Buffer** outBuffer) noexcept final;
		[[nodiscard]] ReturnCode CreateTexture2D(size_t width, size_t height, size_t mips, TextureFormat format,
			ResourceUsage usage, const char* name, Texture2D** outTexture) noexcept final;
		[[nodiscard]] ReturnCode CreateTexture3D(size_t width, size_t height, size_t depth, size_t mips, TextureFormat format,
			ResourceUsage usage, const char* name, Texture3D** outTexture) noexcept final;

		[[nodiscard]] ReturnCode CreateSampler(SamplerAddressMode addressMode, SamplerFilter filter,
			size_t descriptorLocationsCount, const DescriptorLocation* descriptorLocations, const char* name,
			Sampler** outSampler) noexcept final;

		ReturnCode ReleaseBuffer(Buffer* buffer) noexcept final;
		ReturnCode ReleaseTexture2D(Texture2D* texture) noexcept final;
		ReturnCode ReleaseTexture3D(Texture3D* texture) noexcept final;
		ReturnCode ReleaseSampler(Sampler* sampler) noexcept final;

		ReturnCode CreateDescriptorHeap(DescriptorHeapType heapType, const PipelineLayoutDesc& pipelineLayoutDesc, const char* name,
			DescriptorHeap** outHeap) noexcept final;
		ReturnCode CloneDescriptorHeap(const DescriptorHeap* src, const char* name, DescriptorHeap** outHeap) noexcept final;
		ReturnCode ReleaseDescriptorHeap(DescriptorHeap* heap) noexcept final;

		ReturnCode CreateQueryHeap(
			QueryHeapType heapType, uint64_t queryCount, const char* name, QueryHeap** outHeap) noexcept final;

		ReturnCode ReleaseQueryHeap(QueryHeap* heap) noexcept final;

		ReturnCode CreateFramebuffer(size_t renderTargetsCount, Texture2D* const* renderTargets, Texture2D* depthStencil,
			const char* name, Framebuffer** outFramebuffer) noexcept final;
		ReturnCode ReleaseFramebuffer(Framebuffer* framebuffer) noexcept;

		ReturnCode InitializeSRV(Buffer* buffer, InitializeViewDesc desc, size_t descriptorLocationsCount,
			const DescriptorLocation* descriptorLocations) noexcept final;
		ReturnCode InitializeSRV(
			Texture2D* texture, size_t descriptorLocationsCount, const DescriptorLocation* descriptorLocations) noexcept final;
		ReturnCode InitializeSRV(
			Texture3D* texture, size_t descriptorLocationsCount, const DescriptorLocation* descriptorLocations) noexcept final;

		ReturnCode InitializeUAV(Buffer* buffer, InitializeViewDesc desc, size_t descriptorLocationsCount,
			const DescriptorLocation* descriptorLocations) noexcept final;
		ReturnCode InitializeUAV(
			Texture2D* texture, size_t descriptorLocationsCount, const DescriptorLocation* descriptorLocations) noexcept final;
		ReturnCode InitializeUAV(
			Texture3D* texture, size_t descriptorLocationsCount, const DescriptorLocation* descriptorLocations) noexcept final;

		ReturnCode InitializeCBV(
			Buffer* buffer, size_t descriptorLocationsCount, const DescriptorLocation* descriptorLocations) noexcept final;

		ReturnCode StartRecording() noexcept final;
		ReturnCode StopRecording() noexcept final;

		ReturnCode UploadBuffer(Buffer* buffer, const void* data, uint32_t size, uint32_t offset) noexcept final;
		ReturnCode UploadBufferSlow(Buffer* buffer, const void* data, uint32_t size, uint32_t offset) noexcept final;
		ReturnCode UploadTextureSlow(Texture3D* texture, const TextureData& uploadData) noexcept final;

		ReturnCode GetBufferData(Buffer* buffer, void* destination, size_t size, size_t srcOffset) noexcept final;
		ReturnCode GetBufferDataImmediately(Buffer* buffer, void* destination, size_t size, size_t srcOffset) noexcept final;

		ReturnCode Dispatch(const DispatchDesc& desc) noexcept final;
		ReturnCode DrawIndexedInstanced(const DrawIndexedInstancedDesc& desc) noexcept final;
		ReturnCode DrawInstanced(const DrawInstancedDesc& desc) noexcept final;
		ReturnCode DispatchIndirect(const DispatchIndirectDesc& desc) noexcept final;
		ReturnCode DrawInstancedIndirect(const DrawInstancedIndirectDesc& desc) noexcept final;
		ReturnCode DrawIndexedInstancedIndirect(const DrawIndexedInstancedIndirectDesc& desc) noexcept final;

		ReturnCode CopyBufferRegion(
			Buffer* dstBuffer, uint32_t dstOffset, Buffer* srcBuffer, uint32_t srcOffset, uint32_t size) noexcept final;
		ReturnCode ClearFormattedBuffer(
			Buffer* buffer, uint32_t bufferSize, uint8_t clearValue, const DescriptorLocation& descriptorLocation) noexcept final;

		ReturnCode SubmitQuery(QueryHeap* queryHeap, uint64_t queryIndex, const QueryDesc& queryDesc) noexcept final;
		ReturnCode ResolveQuery(
			QueryHeap* queryHeap, QueryType queryType, uint64_t offset, uint64_t count, void* result, size_t size) noexcept final;
		[[nodiscard]] uint64_t GetTimestampFrequency() const noexcept final;
		[[nodiscard]] TimestampCalibrationResult GetTimestampCalibration() const noexcept final;
		[[nodiscard]] uint64_t GetCPUTimestamp() const noexcept final;

		ReturnCode StartDebugRegion(const char* RegionName) noexcept final;
		ReturnCode EndDebugRegion() noexcept final;
		void StartFrameCapture(const char* CaptureName) noexcept final;
		void StopFrameCapture() noexcept final;

	private:
		void ValidateGroupCount(const DispatchDesc& Desc);
		void BindResources(
			FRHICommandListImmediate& RHICmdList, const BindingsDescBase& Desc, FRHIShader* ShaderRHI, ShaderStage Stage) noexcept;
		void UnbindResources(
			FRHICommandListImmediate& RHICmdList, const BindingsDescBase& Desc, FRHIShader* ShaderRHI, ShaderStage Stage) noexcept;
		FRHIRenderPassInfo BeginDraw(FRHICommandListImmediate& RHICmdList, const DrawDescBase& Desc);
		void PrepareDraw(FRHICommandListImmediate& RHICmdList, FUnrealGraphicsPSO* GraphicsPSO, const DrawDescBase& DrawDesc,
			const BindingsDescBase& CommonDesc);
		void EndDraw(FRHICommandListImmediate& RHICmdList, const DrawDescBase& Desc);

		static EBufferUsageFlags ConvertToERHIUsageFlags(ResourceHeapType HeapType, ResourceUsage Usage) noexcept;
		static ERHIAccess ConvertToInitialState(ResourceHeapType HeapType, ResourceUsage Usage) noexcept;
		static ESamplerAddressMode ConvertToESamplerAddressMode(SamplerAddressMode AddressMode) noexcept;
		static ESamplerFilter ConvertToESamplerFilter(SamplerFilter Filter) noexcept;
		static int32 GetMaxAnisotrophy(SamplerFilter Filter) noexcept;
		static ECompareFunction ConvertToUnrealComparisonFunc(DepthComparisonFunc DepthComparison) noexcept;
		static ERasterizerCullMode ConvertToUnrealCullMode(TriangleCullMode CullMode, bool bFrontCounterClockwise) noexcept;
		static ERasterizerFillMode ConvertToUnrealFillMode(bool IsWireframeMode) noexcept;
		static EPrimitiveType ConvertToUnrealPrimitiveTopology(PrimitiveTopology Topology) noexcept;
		static EVertexElementType ConvertToUnrealVertexInputFormat(InputElementFormat Format, uint32_t ElementCount) noexcept;
		static uint32_t ConvertToUnrealVertexInputStride(InputElementFormat Format, uint32_t ElementCount) noexcept;
		static uint32_t ConvertToNumVertices(uint32_t VertexBufferSize, uint32_t VertexBufferStride) noexcept;
		static uint32_t ConvertToNumPrimitives(uint32_t IndexCount, EPrimitiveType Topology) noexcept;
		static EPixelFormat ConvertToEPixelFormat(TextureFormat Format) noexcept;
		static EPixelFormat ConvertToEPixelFormat(BufferFormat Format) noexcept;
		static ETextureCreateFlags ConvertToETextureCreateFlags(ResourceUsage Usage) noexcept;

		static void InitializeSRV(FUnrealRHIBuffer* Buffer) noexcept;
		static void InitializeFormattedSRV(FUnrealRHIBuffer* Buffer, EPixelFormat Format) noexcept;
		static void InitializeUAV(FUnrealRHIBuffer* Buffer) noexcept;
		static void InitializeFormattedUAV(FUnrealRHIBuffer* Buffer, EPixelFormat Format) noexcept;
	};
}	 // namespace Zibra::RHI::APIUnrealRHI
