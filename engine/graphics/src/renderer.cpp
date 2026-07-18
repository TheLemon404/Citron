#include "renderer.hpp"
#include "window.hpp"
#include <event.hpp>
#include <logger.hpp>
#include <webgpu/webgpu.hpp>

using namespace CitronGraphics;

RenderPass::RenderPass(wgpu::Device &device, wgpu::Texture &targetTexture,
					   wgpu::CommandEncoder &commandEncoder, Frame &parentFrame)
	: device(device), commandEncoder(commandEncoder),
	  targetTexture(targetTexture), parentFrame(parentFrame) {

	targetView = targetTexture.createView();

	wgpu::RenderPassColorAttachment colorAttachment = {};
	colorAttachment.nextInChain = nullptr;
	colorAttachment.view = targetView;
	colorAttachment.resolveTarget = nullptr;
	colorAttachment.loadOp = wgpu::LoadOp::Clear;
	colorAttachment.storeOp = wgpu::StoreOp::Store;
	colorAttachment.clearValue = {1.0, 0.0, 1.0, 1.0};

	wgpu::RenderPassDescriptor renderPassDescriptor = {};
	renderPassDescriptor.nextInChain = nullptr;
	renderPassDescriptor.colorAttachmentCount = 1;
	renderPassDescriptor.colorAttachments = &colorAttachment;

	renderPassEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
}

RenderPass::~RenderPass() { targetView.release(); }

void RenderPass::end() {
	renderPassEncoder.end();
	renderPassEncoder.release();
}

RenderPass Frame::beginRenderPass(wgpu::Texture &targetTexture) {
	return RenderPass(device, targetTexture, encoder, *this);
}

void Renderer::init() { device.aquirePlatformResources(); }

Frame Renderer::beginFrame() {
	return Frame(device.getWGPUDevice(),
				 device.getWGPUDevice().createCommandEncoder(),
				 device.getCurrentSurfaceTexture());
}

void Renderer::endFrame(Frame &frame) {
	wgpu::CommandBuffer commandBuffer = frame.getEncoder().finish();
	frame.getEncoder().release();
	device.getQueue().submit(commandBuffer);

	commandBuffer.release();

	device.presentCurrentSurfaceTexture();
}

void Renderer::end() { device.releasePlatformResources(); }

void Renderer::onEvent(Event &e) {}
