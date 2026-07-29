#include "material.hpp"
#include "logger.hpp"
#include <webgpu/webgpu.hpp>
#include <yaml-cpp/yaml.h>

using namespace CitronGraphics;

Material::Material(const UUID uuid, Device &device) : Asset<Material, AssetType::MATERIAL>(uuid), device(device) {
	wgpu::BufferDescriptor materialUniformBufferDesc = {};
	materialUniformBufferDesc.nextInChain = nullptr;
	materialUniformBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
	materialUniformBufferDesc.size = Shader::paddedSizeof<MaterialUniforms>();
	materialUniformBuffer.buffer = device.getWGPUDevice().createBuffer(materialUniformBufferDesc);

	device.getQueue().writeBuffer(materialUniformBuffer.buffer, 0, &materialUniforms, Shader::paddedSizeof<MaterialUniforms>());
}

std::shared_ptr<AssetBase> MaterialImporter::importAsset(AssetMetadata metadata) {
	CITRON_CORE_INFO("Loading material: {}", metadata.assetPath.string());

	std::shared_ptr<Material> material = std::make_shared<Material>(metadata.uuid, device);
	YAML::Node materialNode = YAML::LoadFile(metadata.assetPath.string());
	material->shader.uuid = materialNode["shader"].as<uint64_t>();
	return material;
}
