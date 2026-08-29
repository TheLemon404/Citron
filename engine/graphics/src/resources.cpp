#include "resources.hpp"
#include "buffer.hpp"
#include "debug.hpp"
#include "mesh.hpp"
#include "pipeline.hpp"
#include "shader.hpp"
#include <memory>
#include <webgpu/webgpu.hpp>
#include "compiled_shaders.hpp"
#include "uuid.hpp"

constexpr uint32_t MAX_DEBUG_LINES = 1000;

using namespace CitronGraphics;

void RendererResourceManager::initResources() {
	// frame uniforms buffer
	wgpu::BufferDescriptor frameUniformBufferDesc = {};
	frameUniformBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
	frameUniformBufferDesc.mappedAtCreation = false;
	frameUniformBufferDesc.size = Shader::paddedSizeof<FrameUniforms>();
	frameUniformsBuffer.buffer = device.getWGPUDevice().createBuffer(frameUniformBufferDesc);
	frameUniformsBuffer.size = frameUniformBufferDesc.size;
	frameUniformsBuffer.entryCount = 1;
	device.getQueue().writeBuffer(frameUniformsBuffer.buffer, 0, &frameUniforms, Shader::paddedSizeof<FrameUniforms>());

	// debug shaders
	debugGridShader = assetManager.createAsset<Shader>(device, CompiledShaders::debug_grid);
	debugWireframeShader = assetManager.createAsset<Shader>(device, CompiledShaders::debug_wireframe);

	// debug meshes
	debugGridMesh = Mesh::createPlane(device, assetManager);

	DebugUtils::initialize(&debugLines);
}

void RendererResourceManager::releaseUnusedBindGroups() {
	for (auto it = bindGroupCache.begin(); it != bindGroupCache.end();) {
		if (!usedBindGroupKeysThisFrame.contains(it->first)) {
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
		auto pipeline = std::make_shared<Pipeline>(device.getWGPUDevice(), key.colorAttachmentFormats, key.hasDepthStencilAttachment, key.shader, key.cullMode, key.topology);
		pipelineCache[key] = pipeline;
	}
	return pipelineCache[key];
}

void RendererResourceManager::constructDebugLinesMultiMesh() {
	if (debugLines.size() == 0) {
		return;
	}

	debugLinesMeshVertices.clear();
	debugLinesMeshIndices.clear();
	glm::vec3 minBounds;
	glm::vec3 maxBounds;

	for (const DebugLine &line : debugLines) {
		Vertex startVertex(line.start.x, line.start.y, line.start.z);
		startVertex.color = line.color;
		Vertex endVertex(line.end.x, line.end.y, line.end.z);
		endVertex.color = line.color;

		debugLinesMeshVertices.push_back(startVertex);
		debugLinesMeshVertices.push_back(endVertex);

		debugLinesMeshIndices.push_back(debugLinesMeshVertices.size() - 2);
		debugLinesMeshIndices.push_back(debugLinesMeshVertices.size() - 1);

		// bounding box logic does not work!!!
		if (line.start.x < minBounds.x) {
			minBounds.x = line.start.x;
		}
		if (line.end.x > maxBounds.x) {
			maxBounds.x = line.end.x;
		}
		if (line.start.y < minBounds.y) {
			minBounds.y = line.start.y;
		}
		if (line.end.y > maxBounds.y) {
			maxBounds.y = line.end.y;
		}
		if (line.start.z < minBounds.z) {
			minBounds.z = line.start.z;
		}
		if (line.end.z > maxBounds.z) {
			maxBounds.z = line.end.z;
		}
	}
	if (!debugLinesMultiMesh) {
		debugLinesMultiMesh = std::make_shared<Mesh>(
			UUID::nullID,
			MAX_DEBUG_LINES * 2,
			MAX_DEBUG_LINES * 2, device);
	}

	debugLinesMultiMesh->updateVertexBuffer(debugLinesMeshVertices);
	debugLinesMultiMesh->updateIndexBuffer(debugLinesMeshIndices);
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
