#define WEBGPU_CPP_IMPLEMENTATION

#include "context.hpp"
#include <logger.hpp>
#include <webgpu/webgpu.hpp>

using namespace CitronCore;
using namespace CitronGraphics;

void Context::aquirePlatformResources() {
  instance = wgpu::createInstance();
  CITRON_CORE_ASSERT(instance, "Failed to create WGPU instance");
  CITRON_CORE_INFO("WGPU instance created");

  wgpu::RequestAdapterOptions adapterOptions = {};
  adapterOptions.nextInChain = nullptr;
  wgpu::Adapter adapter = instance.requestAdapter(adapterOptions);
  CITRON_CORE_ASSERT(adapter, "Failed to obtained WGPU adapter");
  CITRON_CORE_INFO("WGPU adapter obtained");

  wgpu::Limits adapterLimits = {};
  wgpu::Status adapterLimitsStatus = adapter.getLimits(&adapterLimits);
  CITRON_CORE_ASSERT(adapterLimitsStatus == wgpu::Status::Success,
                     "failed to get adapter limits");
  CITRON_CORE_INFO("Adapter limits: ");
  CITRON_CORE_INFO(" - maxTextureDimension1D: {0}",
                   adapterLimits.maxTextureDimension1D);
  CITRON_CORE_INFO(" - maxTextureDimension2D: {0}",
                   adapterLimits.maxTextureDimension2D);
  CITRON_CORE_INFO(" - maxTextureDimension3D: {0}",
                   adapterLimits.maxTextureDimension3D);
  CITRON_CORE_INFO(" - maxTextureArrayLayers: {0}",
                   adapterLimits.maxTextureArrayLayers);

  wgpu::SupportedFeatures supportedFeatures = {};
  adapter.getFeatures(&supportedFeatures);
  CITRON_CORE_INFO("Adapter features: ");
  for (size_t i = 0; i < supportedFeatures.featureCount; ++i) {
    CITRON_CORE_INFO(" - 0x{0}",
                     static_cast<uint32_t>(supportedFeatures.features[i]));
  }

  wgpu::AdapterInfo info = {};
  adapter.getInfo(&info);

  CITRON_CORE_INFO("Adapter limits: ");
  CITRON_CORE_INFO(" - vendorId: {0}", info.vendorID);
  CITRON_CORE_INFO(" - vendorName: {0}", info.vendor.data);
  CITRON_CORE_INFO(" - architecture: {0}", info.architecture.data);
  CITRON_CORE_INFO(" - deviceId {0}", info.deviceID);
  CITRON_CORE_INFO(" - driverDescription: {0}", info.description.data);
  CITRON_CORE_INFO(" - adapterType: 0x{0}",
                   static_cast<uint32_t>(info.adapterType));
  CITRON_CORE_INFO(" - backendType: 0x{0}",
                   static_cast<uint32_t>(info.backendType));
}
