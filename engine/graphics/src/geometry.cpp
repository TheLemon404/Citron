#include "geometry.hpp"
#include "buffer.hpp"
#include "glm/fwd.hpp"
#include <webgpu/webgpu.hpp>

using namespace CitronGraphics;

Geometry::Geometry(std::vector<Vertex> vertices, std::vector<uint32_t> indices, Device &device) : Asset<Geometry, AssetType::GEOMETRY>(UUID()), vertices(vertices), indices(indices), device(device) {
	wgpu::BufferDescriptor vertexBufferDesc;
	vertexBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
	vertexBufferDesc.size = vertices.size() * sizeof(Vertex);
	vertexBufferDesc.mappedAtCreation = false;

	vertexBuffer.buffer = device.getWGPUDevice().createBuffer(vertexBufferDesc);
	vertexBuffer.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
	vertexBuffer.size = vertices.size() * sizeof(Vertex);
	vertexBuffer.entryCount = vertices.size();

	device.getQueue().writeBuffer(vertexBuffer.buffer, 0, vertices.data(), vertexBufferDesc.size);
}

std::shared_ptr<AssetBase> GeometryImporter::importAsset(AssetMetadata metadata) {
	return nullptr;
}
