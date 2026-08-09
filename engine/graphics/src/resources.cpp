#include "resources.hpp"
#include "buffer.hpp"
#include "logger.hpp"
#include "mesh.hpp"
#include "pipeline.hpp"
#include <webgpu/webgpu.hpp>

using namespace CitronGraphics;

void RendererResourceManager::initResources() {
}

void RendererResourceManager::releaseUnusedBindGroups() {
	for (auto it = bindGroupCache.begin(); it != bindGroupCache.end();) {
		if (!usedBindGroupKeysThisFrame.contains(it->first)) {
			CITRON_CORE_INFO("RELEASE");
			it->second.release();
			it = bindGroupCache.erase(it);
		} else {
			++it;
		}
	}
	usedBindGroupKeysThisFrame.clear();
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

wgpu::BindGroup RendererResourceManager::getBindGroup(BindGroupKey key) {
	std::sort(key.entries.begin(), key.entries.end(), [](const BindGroupEntry &a, const BindGroupEntry &b) {
		return a.binding < b.binding;
	});

	if (!bindGroupCache.contains(key)) {
		CITRON_CORE_INFO("Creating new bind group");
		std::vector<wgpu::BindGroupEntry> entries;
		for (const auto &e : key.entries) {
			switch (e.resource.index()) {
			case 0: {
				wgpu::BindGroupEntry entry = {};
				entry.setDefault();
				entry.binding = e.binding;
				entry.buffer = std::get<wgpu::Buffer>(e.resource);
				entry.offset = e.offset;
				entry.size = e.size;
				entries.push_back(entry);
				break;
			}
			case 1: {
				wgpu::BindGroupEntry entry = {};
				entry.setDefault();
				entry.binding = e.binding;
				entry.textureView = std::get<wgpu::TextureView>(e.resource);
				entry.offset = e.offset;
				entry.size = e.size;
				entries.push_back(entry);
				break;
			}
			default:
				break;
			}
		}
		wgpu::BindGroupDescriptor bindGroupDesc = {};
		bindGroupDesc.setDefault();
		bindGroupDesc.nextInChain = nullptr;
		bindGroupDesc.entryCount = entries.size();
		bindGroupDesc.entries = entries.data();
		bindGroupDesc.layout = key.layout;
		wgpu::BindGroup bindGroup = device.getWGPUDevice().createBindGroup(bindGroupDesc);
		bindGroupCache[key] = bindGroup;
	}
	usedBindGroupKeysThisFrame.insert(key);
	return bindGroupCache[key];
}

std::shared_ptr<Pipeline> RendererResourceManager::getPipeline(PipelineKey key) {
	if (!pipelineCache.contains(key)) {
		auto pipeline = std::make_shared<Pipeline>(device.getWGPUDevice(), key.colorAttachmentFormats, key.hasDepthStencilAttachment, key.shader);
		pipelineCache[key] = pipeline;
	}
	return pipelineCache[key];
}

GPUBuffer &RendererResourceManager::getEntityModelUniformBuffer(uint32_t entityUUID, ModelUniforms &modelUniforms) {
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
	}

	return entityModelUniformBufferCache[entityUUID];
}

GPUBuffer &RendererResourceManager::getDrawUniformBuffer(uint16_t renderCount) {
	wgpu::Device &wgpuDevice = device.getWGPUDevice();
	if (!drawUniformBufferCache.contains(renderCount)) {
		wgpu::BufferDescriptor frameUniformBufferDesc = {};
		frameUniformBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
		frameUniformBufferDesc.mappedAtCreation = false;
		frameUniformBufferDesc.size = Shader::paddedSizeof<DrawUniforms>();
		GPUBuffer drawUniformBuffer = {};
		drawUniformBuffer.buffer = device.getWGPUDevice().createBuffer(frameUniformBufferDesc);
		drawUniformBufferCache[renderCount] = drawUniformBuffer;
	}
	return drawUniformBufferCache[renderCount];
}
