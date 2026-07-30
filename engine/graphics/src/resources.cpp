#include "resources.hpp"
#include "buffer.hpp"
#include "mesh.hpp"

using namespace CitronGraphics;

GPUBuffer &RendererResourceManager::getEntityModelUniformBuffer(uint64_t entityUUID, ModelUniforms &modelUniforms, bool isDirty) {
	wgpu::Device &wgpuDevice = device.getWGPUDevice();
	if (!entityModelUniformBufferCache.contains(entityUUID)) {
		wgpu::BufferDescriptor modelUniformBufferDesc = {};
		modelUniformBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
		modelUniformBufferDesc.mappedAtCreation = false;
		modelUniformBufferDesc.size = Shader::paddedSizeof<ModelUniforms>();

		entityModelUniformBufferCache[entityUUID] = {};
		entityModelUniformBufferCache[entityUUID].buffer = wgpuDevice.createBuffer(modelUniformBufferDesc);
		entityModelUniformBufferCache[entityUUID].size = modelUniformBufferDesc.size;
		entityModelUniformBufferCache[entityUUID].entryCount = 1;

		device.getQueue().writeBuffer(entityModelUniformBufferCache[entityUUID].buffer, 0, &modelUniforms, Shader::paddedSizeof<ModelUniforms>());
	}

	if (isDirty) {
		device.getQueue().writeBuffer(entityModelUniformBufferCache[entityUUID].buffer, 0, &modelUniforms, Shader::paddedSizeof<ModelUniforms>());
	}

	return entityModelUniformBufferCache[entityUUID];
}
