#include "renderer.hpp"
#include "buffer.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include <event.hpp>
#include <logger.hpp>
#include <webgpu/webgpu.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>

using namespace CitronGraphics;

RenderPass::RenderPass(Renderer &renderer, wgpu::Texture &targetTexture,
					   wgpu::CommandEncoder &commandEncoder, Frame &parentFrame)
	: renderer(renderer), commandEncoder(commandEncoder),
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

void RenderPass::drawRenderData(std::vector<uint64_t> &entityUUIDs, std::vector<glm::mat4> &transforms, std::vector<uint64_t> &meshUUIDs, std::vector<uint64_t> &materialUUIDs) {
	if (meshUUIDs.size() != materialUUIDs.size()) {
		CITRON_CORE_ERROR("You must provide the same number of mesh and material UUIDs to draw render data");
		return;
	}

	std::vector<RenderObject> renderObjects(meshUUIDs.size());
	RendererContext context = renderer.getContext();

	for (size_t i = 0; i < meshUUIDs.size(); i++) {
		std::shared_ptr<Mesh> mesh = context.assetManager.getAsset<Mesh>(meshUUIDs[i]);
		std::shared_ptr<Material> material = context.assetManager.getAsset<Material>(materialUUIDs[i]);
		std::shared_ptr<Shader> shader = context.assetManager.getAsset<Shader>(material->shader.uuid);

		if (!mesh || !material || !shader) {
			CITRON_CORE_ERROR("Failed to get mesh, material, or shader for render object");
			continue;
		}

		renderObjects[i] = {
			0,
			{.transform = glm::identity<glm::mat4>()},
			mesh,
			material,
			shader,
		};
	}

	for (auto &renderObject : renderObjects) {
		if (renderObject.mesh && renderObject.shader) {
			std::shared_ptr<Pipeline> pipeline = renderer.getPipeline({renderObject.shader, context.device.getWGPUPreferredSurfaceFormat()});
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

			/*

			entries.clear();
			entries.push_back({.binding = 0,
							   .resource = context.rendererResourcesManager.getEntityModelUniformBuffer(renderObject.entityUUID, renderObject.modelUniforms).buffer,
							   .offset = 0,
							   .size = Shader::paddedSizeof<ModelUniforms>()});
			wgpu::BindGroup modelBindGroup = renderer.getBindGroup({
				.layout = renderObject.shader->getBindGroupLayout(1),
				.entries = entries,
			});

*/

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

Frame::Frame(Renderer &renderer, wgpu::CommandEncoder encoder,
			 wgpu::SurfaceTexture &surfaceTexture)
	: renderer(renderer), encoder(encoder) {
}

RenderPass Frame::beginRenderPass(wgpu::Texture &targetTexture) {
	return RenderPass(renderer, targetTexture, encoder, *this);
}

void Renderer::init() {
	device.aquirePlatformResources();

	wgpu::BufferDescriptor frameUniformBufferDesc = {};
	frameUniformBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
	frameUniformBufferDesc.mappedAtCreation = false;
	frameUniformBufferDesc.size = Shader::paddedSizeof<FrameUniforms>();
	frameUniformBuffer.buffer = device.getWGPUDevice().createBuffer(frameUniformBufferDesc);
	device.getQueue().writeBuffer(frameUniformBuffer.buffer, 0, &frameUniforms, Shader::paddedSizeof<FrameUniforms>());

	colorTarget = device.createEmptyRenderTargetTexture();
	colorTargetView = device.createTextureView(colorTarget);
}

Frame Renderer::beginFrame() {
	return Frame(*this,
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
