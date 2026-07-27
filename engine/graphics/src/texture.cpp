#include "texture.hpp"
#include <cstdint>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <webgpu/webgpu.hpp>

using namespace CitronGraphics;

Texture::Texture(const UUID uuid, uint32_t width, uint32_t height) : Asset(uuid), width(width), height(height), texture(nullptr) {}

Texture::~Texture() {
	if (textureView) {
		textureView.release();
	}
	if (texture) {
		texture.destroy();
		texture.release();
	}
}

std::shared_ptr<Texture> Texture::loadFromFile(const std::filesystem::path &path, Device &device) {
	int width, height, channels;
	unsigned char *data = stbi_load(path.string().c_str(), &width, &height, &channels, 0);
	if (!data) {
		return nullptr;
	}

	wgpu::TextureDescriptor descriptor = {};

	descriptor.dimension = wgpu::TextureDimension::_2D;
	descriptor.size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
	descriptor.mipLevelCount = 1;
	descriptor.sampleCount = 1;
	descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
	descriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
	descriptor.viewFormatCount = 0;
	descriptor.viewFormats = nullptr;

	std::shared_ptr<Texture> texture = std::make_shared<Texture>(UUID(), static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	texture->texture = device.getWGPUDevice().createTexture(descriptor);

	writeMipMaps(device, texture, descriptor.size, descriptor.mipLevelCount, data);

	texture->textureView = texture->texture.createView();
	stbi_image_free(data);
	return texture;
}

void Texture::writeMipMaps(Device &device, std::shared_ptr<Texture> texture, wgpu::Extent3D textureSize, uint32_t mipLevelCount, const unsigned char *data) {
	wgpu::TexelCopyTextureInfo destination;
	destination.texture = texture->texture;
	destination.mipLevel = 0;
	destination.origin = {0, 0, 0};
	destination.aspect = wgpu::TextureAspect::All;

	wgpu::TexelCopyBufferLayout source;
	source.offset = 0;
	source.bytesPerRow = 4 * textureSize.width;
	source.rowsPerImage = textureSize.height;

	wgpu::Queue queue = device.getWGPUDevice().getQueue();
	queue.writeTexture(destination, data, 4 * textureSize.width * textureSize.height, source, textureSize);
	queue.release();
}

std::shared_ptr<AssetBase> TextureImporter::importAsset(AssetMetadata metadata) {
	return nullptr;
}
