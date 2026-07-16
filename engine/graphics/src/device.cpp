#include <cstddef>
#define WEBGPU_CPP_IMPLEMENTATION

#include "device.hpp"
#include <logger.hpp>
#include <webgpu/webgpu.hpp>

using namespace CitronCore;
using namespace CitronGraphics;

void Device::aquirePlatformResources() {
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
}

void Device::releasePlatformResources() {
	if (queue)
		queue.release();
	if (device)
		device.release();
	if (adapter)
		adapter.release();
	if (instance)
		instance.release();
}
