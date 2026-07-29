#include "renderer.hpp"
#include "buffer.hpp"
#include "material.hpp"
#include <event.hpp>
#include <logger.hpp>
#include <webgpu/webgpu.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>

using namespace CitronGraphics;

RenderPass::RenderPass(Renderer &renderer, Device &device, wgpu::Texture &targetTexture,
					   wgpu::CommandEncoder &commandEncoder, Frame &parentFrame)
	: renderer(renderer), device(device), commandEncoder(commandEncoder),
	  targetTexture(targetTexture), parentFrame(parentFrame) {

	// Render pass encoder setup
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
			std::shared_ptr<Pipeline> pipeline = renderer.getPipeline({renderObject.shader, device.getWGPUPreferredSurfaceFormat()});
			if (!pipeline) {
				CITRON_CORE_ERROR("Failed to get pipeline for render object");
				continue;
			}

			std::vector<BindGroupEntry> entries;
			entries.push_back({.binding = 0,
							   .resource = renderer.getFrameUniformBuffer().buffer,
							   .offset = 0,
							   .size = Shader::paddedSizeof<FrameUniforms>()});
			wgpu::BindGroup frameBindGroup = renderer.getBindGroup({
				.layout = renderObject.shader->getBindGroupLayout(0),
				.entries = entries,
			});

			entries.clear();
			entries.push_back({.binding = 0,
							   .resource = renderObject.material->getMaterialUniformBuffer().buffer,
							   .offset = 0,
							   .size = Shader::paddedSizeof<MaterialUniforms>()});
			wgpu::BindGroup materialBindGroup = renderer.getBindGroup({
				.layout = renderObject.shader->getBindGroupLayout(1),
				.entries = entries,
			});

			if (!frameBindGroup || !materialBindGroup) {
				CITRON_CORE_ERROR("Failed to get bind groups from cache for render object");
				continue;
			}

			setPipeline(pipeline);
			setMesh(renderObject.mesh);
			setBindGroup(0, frameBindGroup);
			setBindGroup(1, materialBindGroup);
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

void RenderPass::setBindGroup(int index, wgpu::BindGroup bindGroup) {
	renderPassEncoder.setBindGroup(index, bindGroup, 0, nullptr);
}

void RenderPass::draw(std::shared_ptr<Mesh> geometry) {
	renderPassEncoder.drawIndexed(geometry->getIndexBuffer().entryCount, 1, 0, 0, 0);
}

void RenderPass::end() {
	renderPassEncoder.end();
	renderPassEncoder.release();
}

Frame::Frame(Renderer &renderer, Device &device, wgpu::CommandEncoder encoder,
			 wgpu::SurfaceTexture &surfaceTexture)
	: renderer(renderer), device(device), encoder(encoder), surfaceTexture(surfaceTexture) {
}

RenderPass Frame::beginRenderPass(wgpu::Texture &targetTexture) {
	return RenderPass(renderer, device, targetTexture, encoder, *this);
}

void Renderer::init() {
	device.aquirePlatformResources();

	wgpu::BufferDescriptor frameUniformBufferDesc = {};
	frameUniformBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
	frameUniformBufferDesc.mappedAtCreation = false;
	frameUniformBufferDesc.size = sizeof(FrameUniforms);
	frameUniformBuffer.buffer = device.getWGPUDevice().createBuffer(frameUniformBufferDesc);
	device.getQueue().writeBuffer(frameUniformBuffer.buffer, 0, &frameUniforms, sizeof(FrameUniforms));
}

Frame Renderer::beginFrame() {
	frameUniforms.mvp = glm::rotate(frameUniforms.mvp, glm::radians(2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	device.getQueue().writeBuffer(frameUniformBuffer.buffer, 0, &frameUniforms, Shader::paddedSizeof<FrameUniforms>());
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

void Renderer::end() {
	device.releasePlatformResources();
}

void Renderer::onEvent(Event &e) {}
