#pragma once

#include "assets.hpp"
#include "device.hpp"
#include <webgpu/webgpu.hpp>

using namespace CitronAssets;

namespace CitronGraphics {
class CITRON_GRAPHICS_API Texture : public Asset<Texture, AssetType::TEXTURE> {
  public:
	Texture(const UUID uuid, uint32_t width, uint32_t height);
	~Texture();

	static std::shared_ptr<Texture> loadFromFile(const std::filesystem::path &path, Device &device);

	static void writeMipMaps(Device &device, std::shared_ptr<Texture> texture, wgpu::Extent3D textureSize, uint32_t mipLevelCount, const unsigned char *data);

	uint32_t width;
	uint32_t height;
	wgpu::Texture texture;

	wgpu::TextureView &getTextureView() { return textureView; }
	wgpu::TextureView textureView;
};

class CITRON_GRAPHICS_API TextureImporter : public AssetImporter {
  public:
	TextureImporter(Device &device) : AssetImporter({".png", ".jpg"}), device(device) {}

	virtual std::shared_ptr<AssetBase> importAsset(AssetMetadata metadata) override;

  private:
	Device &device;
};
} // namespace CitronGraphics
