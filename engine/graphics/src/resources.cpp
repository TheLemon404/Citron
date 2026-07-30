#include "resources.hpp"
#include "buffer.hpp"
#include "mesh.hpp"

using namespace CitronGraphics;

GPUBuffer RendererResourceManager::getEntityModelUniformBuffer(uint64_t entityUUID, ModelUniforms &modelUniforms, bool isDirty) {
	wgpu::Device &wgpuDevice = device.getWGPUDevice();
	if (!entityModelUniformBufferCache.contains(entityUUID)) {
		wgpu::BufferDescriptor modelUniformBufferDesc = {};
		modelUniformBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
		modelUniformBufferDesc.mappedAtCreation = false;
		modelUniformBufferDesc.size = Shader::paddedSizeof<ModelUniforms>();

		GPUBuffer modelUniformBuffer = {};
		modelUniformBuffer.buffer = wgpuDevice.createBuffer(modelUniformBufferDesc);
		modelUniformBuffer.size = modelUniformBufferDesc.size;
		modelUniformBuffer.entryCount = 1;

		device.getQueue().writeBuffer(modelUniformBuffer.buffer, 0, &modelUniforms, Shader::paddedSizeof<ModelUniforms>());
		entityModelUniformBufferCache[entityUUID] = modelUniformBuffer;
	}

	if (isDirty) {
		device.getQueue().writeBuffer(entityModelUniformBufferCache[entityUUID].buffer, 0, &modelUniforms, Shader::paddedSizeof<ModelUniforms>());
	}

	return entityModelUniformBufferCache[entityUUID];
}
