#include <cstddef>

#define WEBGPU_CPP_IMPLEMENTATION

#include "device.hpp"
#include <logger.hpp>
#include <webgpu/webgpu.hpp>

using namespace CitronCore;
using namespace CitronGraphics;

Device::Device(Window &window) : window(window) {}

std::string_view toStringView(wgpu::StringView s) {
	return s.data ? std::string_view(s.data, s.length) : std::string_view{};
}

void Device::aquirePlatformResources() {
	instance = wgpu::createInstance();
	CITRON_CORE_ASSERT(instance, "Failed to create WGPU instance");
	CITRON_CORE_INFO("WGPU instance created");

	surface = window.getSDL_WGPUSurface(instance);
	CITRON_CORE_ASSERT(surface, "Failed to get WGPU surface");
	CITRON_CORE_INFO("WGPU surface obtained");

	wgpu::RequestAdapterOptions adapterOptions = {};
	adapterOptions.nextInChain = nullptr;
	adapterOptions.compatibleSurface = surface;
	adapter = instance.requestAdapter(adapterOptions);
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
	CITRON_CORE_INFO(" - maxVertexAttributed: {0}", adapterLimits.maxVertexAttributes);

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
	CITRON_CORE_INFO(" - vendorName: {0}", toStringView(info.vendor));
	CITRON_CORE_INFO(" - architecture: {0}", toStringView(info.architecture));
	CITRON_CORE_INFO(" - deviceId {0}", info.deviceID);
	CITRON_CORE_INFO(" - driverDescription: {0}", toStringView(info.description));
	CITRON_CORE_INFO(" - adapterType: 0x{0}",
					 static_cast<uint32_t>(info.adapterType));
	CITRON_CORE_INFO(" - backendType: 0x{0}",
					 static_cast<uint32_t>(info.backendType));

	wgpu::DeviceDescriptor deviceDescriptor = {};
	deviceDescriptor.nextInChain = nullptr;
	deviceDescriptor.label = wgpu::StringView("Graphics Device");
	deviceDescriptor.requiredFeatureCount = 0;
	deviceDescriptor.requiredFeatures = nullptr;
	deviceDescriptor.requiredLimits = nullptr;
	deviceDescriptor.defaultQueue = {};
	deviceDescriptor.defaultQueue.nextInChain = nullptr;
	deviceDescriptor.defaultQueue.label = wgpu::StringView("Default Queue");
	deviceDescriptor.deviceLostCallbackInfo = {};
	deviceDescriptor.deviceLostCallbackInfo.callback =
		[](WGPUDevice const *device, WGPUDeviceLostReason reason,
		   WGPUStringView message, WGPU_NULLABLE void *userdata1,
		   WGPU_NULLABLE void *userdata2) {
			CITRON_CORE_CRITICAL("Device lost: reason={0}, message={1}",
								 static_cast<uint32_t>(reason), message.data);
		};
	deviceDescriptor.deviceLostCallbackInfo.nextInChain = nullptr;
	deviceDescriptor.uncapturedErrorCallbackInfo = {};
	deviceDescriptor.uncapturedErrorCallbackInfo.callback =
		[](WGPUDevice const *device, WGPUErrorType type, WGPUStringView message,
		   WGPU_NULLABLE void *userdata1, WGPU_NULLABLE void *userdata2) {
			CITRON_CORE_CRITICAL("Uncaptured error: type={0}, message={1}",
								 static_cast<uint32_t>(type), message.data);
		};
	deviceDescriptor.uncapturedErrorCallbackInfo.nextInChain = nullptr;
	device = adapter.requestDevice(deviceDescriptor);
	wgpu::Limits deviceLimits = {};
	device.getLimits(&deviceLimits);
	CITRON_CORE_INFO("Device Limits: ");
	CITRON_CORE_INFO(" - maxTextureDimension1D: {0}",
					 deviceLimits.maxTextureDimension1D);
	CITRON_CORE_INFO(" - maxTextureDimension2D: {0}",
					 deviceLimits.maxTextureDimension2D);
	CITRON_CORE_INFO(" - maxTextureDimension3D: {0}",
					 deviceLimits.maxTextureDimension3D);
	CITRON_CORE_INFO(" - maxTextureArrayLayers: {0}",
					 deviceLimits.maxTextureArrayLayers);
	CITRON_CORE_INFO(" - maxVertexAttributed: {0}", deviceLimits.maxVertexAttributes);

	queue = device.getQueue();
	wgpu::QueueWorkDoneCallbackInfo eventDoneCallbackInfo = {};
	eventDoneCallbackInfo.nextInChain = nullptr;
	eventDoneCallbackInfo.callback = [](WGPUQueueWorkDoneStatus status,
										void *userData1, void *userData2) {
		if (status == wgpu::QueueWorkDoneStatus::Error) {
			CITRON_CORE_ERROR("Queue work done error status: {0}",
							  static_cast<uint32_t>(status));
		} else {
			CITRON_CORE_INFO("Queue work done with status: {0}",
							 static_cast<uint32_t>(status));
		}
	};
	queue.onSubmittedWorkDone(eventDoneCallbackInfo);

	wgpu::SurfaceConfiguration surfaceConfiguration = {};
	surfaceConfiguration.nextInChain = nullptr;
	surfaceConfiguration.width = window.getWidth();
	surfaceConfiguration.height = window.getHeight();
	// May need to revesit how we get supported surface formats in the future
	wgpu::SurfaceCapabilities cap = {};
	surface.getCapabilities(adapter, &cap);
	preferredSurfaceFormat = cap.formats[0];
	surfaceConfiguration.format = preferredSurfaceFormat;
	surfaceConfiguration.viewFormatCount = 0;
	surfaceConfiguration.viewFormats = nullptr;
	surfaceConfiguration.usage = wgpu::TextureUsage::RenderAttachment;
	surfaceConfiguration.device = device;
	surfaceConfiguration.presentMode = wgpu::PresentMode::Mailbox;
	surfaceConfiguration.alphaMode = wgpu::CompositeAlphaMode::Auto;
	surface.configure(surfaceConfiguration);
}

void Device::releasePlatformResources() {
	if (surface) {
		surface.unconfigure();
		surface.release();
	}
	if (queue)
		queue.release();
	if (device)
		device.release();
	if (adapter)
		adapter.release();
	if (instance)
		instance.release();
}

bool Device::prepareCurrentSurfaceTexture() {
	if (currentSurfaceTexture.texture)
		((wgpu::Texture)currentSurfaceTexture.texture).release();

	if (getLastSurfaceWidth() != window.getWidth() ||
		getLastSurfaceHeight() != window.getHeight()) {
		CITRON_CORE_WARN("Current WGPU surface size is different from window "
						 "size, attempting to resize...");
		resizeSurface(window.getWidth(), window.getHeight());
		return false;
	}

	surface.getCurrentTexture(&currentSurfaceTexture);
	if (currentSurfaceTexture.status ==
		wgpu::SurfaceGetCurrentTextureStatus::Error) {
		CITRON_CORE_ERROR(
			"Current WGPU surface failed in pre-render with status Error");
		return false;
	}
	if (currentSurfaceTexture.status ==
		wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
		if (currentSurfaceTexture.texture)
			((wgpu::Texture)currentSurfaceTexture.texture).release();

		resizeSurface(window.getWidth(), window.getHeight());
		CITRON_CORE_ERROR("Current WGPU surface completed in pre-render with "
						  "status SuccessSuboptimal, attempting to resize...");
		return false;
	}
	if (((wgpu::Texture)currentSurfaceTexture.texture).getWidth() !=
			getLastSurfaceWidth() ||
		((wgpu::Texture)currentSurfaceTexture.texture).getHeight() !=
			getLastSurfaceHeight()) {
		((wgpu::Texture)currentSurfaceTexture.texture).release();

		resizeSurface(window.getWidth(), window.getHeight());
		CITRON_CORE_ERROR("Current surface and surface texture size mismatch, "
						  "attempting to resize...");

		return false;
	}

	return currentSurfaceTexture.status ==
		   wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal;
}

void Device::presentCurrentSurfaceTexture() { surface.present(); }

const int Device::getLastSurfaceWidth() { return m_lastSurfaceWidth; }

const int Device::getLastSurfaceHeight() { return m_lastSurfaceHeight; }

void Device::resizeSurface(int width, int height) {
	wgpu::SurfaceConfiguration surfaceConfiguration = {};
	surfaceConfiguration.nextInChain = nullptr;
	surfaceConfiguration.width = width;
	surfaceConfiguration.height = height;
	surfaceConfiguration.format = preferredSurfaceFormat;
	surfaceConfiguration.viewFormatCount = 0;
	surfaceConfiguration.viewFormats = nullptr;
	surfaceConfiguration.usage = wgpu::TextureUsage::RenderAttachment;
	surfaceConfiguration.device = device;
	surfaceConfiguration.presentMode = wgpu::PresentMode::Fifo;
	surfaceConfiguration.alphaMode = wgpu::CompositeAlphaMode::Auto;
	surface.configure(surfaceConfiguration);

	m_lastSurfaceWidth = width;
	m_lastSurfaceHeight = height;
}

wgpu::Texture Device::createRenderTargetDepthTexture(int width, int height) {
	wgpu::TextureDescriptor depthTargetDesc = {};
	depthTargetDesc.label = wgpu::StringView("depthTarget");
	depthTargetDesc.dimension = wgpu::TextureDimension::_2D;
	depthTargetDesc.size.width = width;
	depthTargetDesc.size.height = height;
	depthTargetDesc.size.depthOrArrayLayers = 1;
	depthTargetDesc.mipLevelCount = 1;
	depthTargetDesc.sampleCount = 1;
	depthTargetDesc.format = wgpu::TextureFormat::Depth32Float;
	depthTargetDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
	return device.createTexture(depthTargetDesc);
}

wgpu::Texture Device::createRenderTargetColorTexture(int width, int height) {
	wgpu::TextureDescriptor colorTargetDesc = {};
	colorTargetDesc.label = wgpu::StringView("colorTarget");
	colorTargetDesc.dimension = wgpu::TextureDimension::_2D;
	colorTargetDesc.size.width = width;
	colorTargetDesc.size.height = height;
	colorTargetDesc.size.depthOrArrayLayers = 1;
	colorTargetDesc.mipLevelCount = 1;
	colorTargetDesc.sampleCount = 1;
	colorTargetDesc.format = preferredSurfaceFormat;
	colorTargetDesc.usage = wgpu::TextureUsage::RenderAttachment |
							wgpu::TextureUsage::TextureBinding;
	return device.createTexture(colorTargetDesc);
}

wgpu::Texture Device::createRenderTargetColorTexture(int width, int height, wgpu::TextureFormat format) {
	wgpu::TextureDescriptor colorTargetDesc = {};
	colorTargetDesc.label = wgpu::StringView("colorTarget");
	colorTargetDesc.dimension = wgpu::TextureDimension::_2D;
	colorTargetDesc.size.width = width;
	colorTargetDesc.size.height = height;
	colorTargetDesc.size.depthOrArrayLayers = 1;
	colorTargetDesc.mipLevelCount = 1;
	colorTargetDesc.sampleCount = 1;
	colorTargetDesc.format = format;
	colorTargetDesc.usage = wgpu::TextureUsage::RenderAttachment |
							wgpu::TextureUsage::TextureBinding;
	return device.createTexture(colorTargetDesc);
}

wgpu::TextureView Device::createTextureView(wgpu::Texture &texture) {
	return texture.createView();
}
