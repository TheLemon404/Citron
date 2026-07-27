#include "renderer.hpp"
#include "window.hpp"
#include <event.hpp>
#include <logger.hpp>
#include <webgpu/webgpu.hpp>

using namespace CitronGraphics;

RenderPass::RenderPass(Renderer &renderer, Device &device, wgpu::Texture &targetTexture,
					   wgpu::CommandEncoder &commandEncoder, Frame &parentFrame)
	: renderer(renderer), device(device), commandEncoder(commandEncoder),
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

void RenderPass::drawRenderData(std::vector<RenderObject> &renderObjects) {
	for (auto &renderObject : renderObjects) {
		if (renderObject.mesh && renderObject.shader) {
			std::shared_ptr<Pipeline> pipeline = renderer.getPipeline(renderObject.shader, device.getWGPUPreferredSurfaceFormat());
			if (!pipeline)
				continue;

			setPipeline(pipeline);
			setMesh(renderObject.mesh);
			draw(renderObject.mesh);
		}
	}
}

void RenderPass::setPipeline(std::shared_ptr<Pipeline> pipeline) {
	renderPassEncoder.setPipeline(pipeline->getPipeline());
}

void RenderPass::setMesh(std::shared_ptr<Mesh> geometry) {
	renderPassEncoder.setVertexBuffer(0, geometry->getVertexBuffer().buffer, 0, WGPU_WHOLE_SIZE);
	renderPassEncoder.setIndexBuffer(geometry->getIndexBuffer().buffer, wgpu::IndexFormat::Uint32, 0, WGPU_WHOLE_SIZE);
}

void RenderPass::draw(std::shared_ptr<Mesh> geometry) {
	renderPassEncoder.drawIndexed(geometry->getIndexBuffer().entryCount, 1, 0, 0, 0);
}

void RenderPass::end() {
	renderPassEncoder.end();
	renderPassEncoder.release();
}

RenderPass Frame::beginRenderPass(wgpu::Texture &targetTexture) {
	return RenderPass(renderer, device, targetTexture, encoder, *this);
}

void Renderer::init() { device.aquirePlatformResources(); }

Frame Renderer::beginFrame() {
	return Frame(*this, device,
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
