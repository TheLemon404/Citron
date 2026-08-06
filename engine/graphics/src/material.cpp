#include "material.hpp"
#include "logger.hpp"
#include "serialization.hpp"
#include "yaml-cpp/node/node.h"
#include <webgpu/webgpu.hpp>
#include <yaml-cpp/yaml.h>

using namespace CitronGraphics;

void Material::serialize(StreamWriter &writer) {
	CITRON_CORE_INFO("Serializing material asset: {}", (uint32_t)uuid);
	writer.writeData(&shader.uuid, sizeof(shader.uuid));
}

void Material::deserialize(StreamReader &reader) {
	CITRON_CORE_INFO("Deserializing material asset: {}", (uint32_t)uuid);
	try {
		reader.readData(&shader.uuid, sizeof(shader.uuid));
	} catch (const std::exception &e) {
		CITRON_CORE_ERROR("Failed to deserialize material asset: {}", e.what());
	}
}

Material::Material(const UUID uuid, Device &device) : Asset<Material, AssetType::MATERIAL>(uuid), device(device) {
	wgpu::BufferDescriptor materialUniformBufferDesc = {};
	materialUniformBufferDesc.nextInChain = nullptr;
	materialUniformBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
	materialUniformBufferDesc.size = Shader::paddedSizeof<MaterialUniforms>();
	materialUniformBuffer.buffer = device.getWGPUDevice().createBuffer(materialUniformBufferDesc);

	device.getQueue().writeBuffer(materialUniformBuffer.buffer, 0, &materialUniforms, Shader::paddedSizeof<MaterialUniforms>());
}

std::shared_ptr<AssetBase> MaterialImporter::importAsset(AssetMetadata metadata) {
	std::shared_ptr<Material> material = std::make_shared<Material>(metadata.uuid, device);
	FileStreamReader reader(metadata.assetPath);
	material->deserialize(reader);
	return material;
}
