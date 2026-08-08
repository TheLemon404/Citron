#include "resources.hpp"
#include "buffer.hpp"
#include "mesh.hpp"
#include <webgpu/webgpu.hpp>

using namespace CitronGraphics;

void RendererResourceManager::initResources() {
}

void RendererResourceManager::releaseResources() {
	for (auto &[entityUUID, buffer] : entityModelUniformBufferCache) {
		buffer.buffer.release();
	}
	for (auto &[key, bindGroup] : bindGroupCache) {
		bindGroup.release();
	}
	for (auto &[key, buffer] : drawUniformBufferCache) {
		buffer.buffer.release();
	}
	entityModelUniformBufferCache.clear();
	bindGroupCache.clear();
	drawUniformBufferCache.clear();
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

GPUBuffer &RendererResourceManager::getDrawUniformBuffer(DrawUniforms drawUniforms, bool isDirty) {
	wgpu::Device &wgpuDevice = device.getWGPUDevice();
	if (!drawUniformBufferCache.contains(drawUniforms)) {
		wgpu::BufferDescriptor frameUniformBufferDesc = {};
		frameUniformBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
		frameUniformBufferDesc.mappedAtCreation = false;
		frameUniformBufferDesc.size = Shader::paddedSizeof<DrawUniforms>();
		GPUBuffer frameUniformBuffer = {};
		frameUniformBuffer.buffer = device.getWGPUDevice().createBuffer(frameUniformBufferDesc);
		device.getQueue().writeBuffer(frameUniformBuffer.buffer, 0, &drawUniforms, Shader::paddedSizeof<DrawUniforms>());
		drawUniformBufferCache[drawUniforms] = frameUniformBuffer;
	}
	if (isDirty) {
		device.getQueue().writeBuffer(drawUniformBufferCache[drawUniforms].buffer, 0, &drawUniforms, Shader::paddedSizeof<DrawUniforms>());
	}
	return drawUniformBufferCache[drawUniforms];
}
