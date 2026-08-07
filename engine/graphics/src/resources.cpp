#include "resources.hpp"
#include "buffer.hpp"
#include "mesh.hpp"
#include <webgpu/webgpu.hpp>

using namespace CitronGraphics;

void RendererResourceManager::initResources() {
	wgpu::BufferDescriptor frameUniformBufferDesc = {};
	frameUniformBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
	frameUniformBufferDesc.mappedAtCreation = false;
	frameUniformBufferDesc.size = Shader::paddedSizeof<FrameUniforms>();
	frameUniformBuffer.buffer = device.getWGPUDevice().createBuffer(frameUniformBufferDesc);
	device.getQueue().writeBuffer(frameUniformBuffer.buffer, 0, &frameUniforms, Shader::paddedSizeof<FrameUniforms>());

	wgpu::BufferDescriptor modelUniformBufferDesc = {};
	modelUniformBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage;
	modelUniformBufferDesc.mappedAtCreation = false;
	modelUniformBufferDesc.size = Shader::paddedSizeof<ModelUniforms>() * MAX_MODEL_UNIFORMS;
	modelUniformsBuffer.buffer = device.getWGPUDevice().createBuffer(modelUniformBufferDesc);
}

void RendererResourceManager::releaseResources() {
	for (auto &[entityUUID, buffer] : entityModelUniformBufferCache) {
		buffer.buffer.release();
	}
	for (auto &[key, bindGroup] : bindGroupCache) {
		bindGroup.release();
	}
	entityModelUniformBufferCache.clear();
	modelUniformsBuffer.buffer.release();
	frameUniformBuffer.buffer.release();
}

GPUBuffer &RendererResourceManager::getEntityModelUniformBuffer(uint32_t entityUUID, ModelUniforms &modelUniforms, bool isDirty) {
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
