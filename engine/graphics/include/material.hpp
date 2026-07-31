#pragma once

#include "buffer.hpp"
#include "device.hpp"
#include "glm/fwd.hpp"
#include "shader.hpp"
#include <assets.hpp>
#include <webgpu/webgpu.hpp>

using namespace CitronAssets;

namespace CitronGraphics {

struct CITRON_GRAPHICS_API MaterialUniforms {
	glm::vec3 diffuse = glm::vec3(1.0f);
};

class CITRON_GRAPHICS_API Material : public Asset<Material, AssetType::MATERIAL>, public ISerializable {
  public:
	Material(const UUID uuid, Device &device);

	AssetReference<Shader> shader;

	virtual void serialize(StreamWriter &archive) override;
	virtual void deserialize(StreamReader &archive) override;

	GPUBuffer &getMaterialUniformBuffer() { return materialUniformBuffer; }
	MaterialUniforms &getMaterialUniforms() { return materialUniforms; }

  private:
	Device &device;
	MaterialUniforms materialUniforms;
	GPUBuffer materialUniformBuffer;
};

class CITRON_GRAPHICS_API MaterialImporter : public AssetImporter {
  public:
	MaterialImporter(Device &device) : AssetImporter({".mat"}), device(device) {}

	virtual std::shared_ptr<AssetBase> importAsset(AssetMetadata metadata) override;

  private:
	Device &device;
};

} // namespace CitronGraphics
