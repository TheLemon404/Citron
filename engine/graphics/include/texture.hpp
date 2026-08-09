#pragma once

#include "assets.hpp"
#include "device.hpp"
#include <webgpu/webgpu.hpp>

using namespace CitronAssets;

namespace CitronGraphics {
class CITRON_GRAPHICS_API Texture {
  public:
	Texture() : width(0), height(0) {}
	Texture(uint32_t width, uint32_t height) : width(width), height(height) {}
	Texture(wgpu::Texture texture, uint32_t width, uint32_t height);

	wgpu::TextureView &getTextureView();
	wgpu::Texture &getTexture() { return texture; }

	void release();

	const uint32_t getWidth() const { return width; }
	const uint32_t getHeight() const { return height; }

  protected:
	uint32_t width;
	uint32_t height;
	wgpu::Texture texture;

	wgpu::TextureView textureView;
	wgpu::Sampler sampler;
};

class CITRON_GRAPHICS_API ImageTexture : public Asset<ImageTexture, AssetType::TEXTURE>,
										 public Texture {
  public:
	ImageTexture(const UUID uuid, uint32_t width, uint32_t height);
	~ImageTexture();

	static std::shared_ptr<ImageTexture> loadFromFile(const std::filesystem::path &path, Device &device);

	static void writeMipMaps(Device &device, std::shared_ptr<ImageTexture> texture, wgpu::Extent3D textureSize, uint32_t mipLevelCount, const unsigned char *data);
};

class CITRON_GRAPHICS_API TextureImporter : public AssetImporter {
  public:
	TextureImporter(Device &device) : AssetImporter({".png", ".jpg"}), device(device) {}

	virtual std::shared_ptr<AssetBase> importAsset(AssetMetadata metadata) override;

  private:
	Device &device;
};
} // namespace CitronGraphics
