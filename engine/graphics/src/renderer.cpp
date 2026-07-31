#include "renderer.hpp"
#include "buffer.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "resources.hpp"
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

void RenderPass::drawRenderData(std::vector<RenderableReferenceData> renderableReferenceData) {
	std::vector<RenderObject> renderObjects;
	RendererContext context = renderer.getContext();

	// need to cache assets already gethered in previous frames
	for (size_t i = 0; i < renderableReferenceData.size(); i++) {
		std::shared_ptr<Mesh> mesh = context.assetManager.getAsset<Mesh>(renderableReferenceData[i].meshUUID);
		std::shared_ptr<Material> material = context.assetManager.getAsset<Material>(renderableReferenceData[i].materialUUID);
		if (!material) {
			CITRON_CORE_ERROR("Failed to get material for render object");
			continue;
		}

		std::shared_ptr<Shader> shader = context.assetManager.getAsset<Shader>(material->shader.uuid);

		if (!mesh || !shader) {
			CITRON_CORE_ERROR("Failed to get mesh or shader for render object");
			continue;
		}

		renderObjects.push_back({
			renderableReferenceData[i].entityUUID,
			{.transform = renderableReferenceData[i].transform},
			mesh,
			material,
			shader,
		});
	}
	if (renderObjects.size() == 0)
		return;

	renderObjects = Renderer::sortByShader(renderObjects);
	std::shared_ptr<Pipeline> pipeline = renderer.getPipeline({renderObjects[0].shader, context.device.getWGPUPreferredSurfaceFormat()});
	setPipeline(pipeline);

	std::vector<BindGroupEntry> bindGroupEntries;
	bindGroupEntries.push_back({.binding = 0,
								.resource = renderer.getContext().rendererResourcesManager.frameUniformBuffer.buffer,
								.offset = 0,
								.size = Shader::paddedSizeof<FrameUniforms>()});
	wgpu::BindGroup frameBindGroup = renderer.getBindGroup({
		.layout = renderObjects[0].shader->getBindGroupLayout(0),
		.entries = bindGroupEntries,
	});
	setBindGroup(0, frameBindGroup);

	for (size_t i = 0; i < renderObjects.size(); i++) {
		RenderObject &renderObject = renderObjects[i];
		if (i > 0 && renderObjects[i].shader->getUUID() != renderObjects[i - 1].shader->getUUID()) {
			pipeline = renderer.getPipeline({renderObject.shader, context.device.getWGPUPreferredSurfaceFormat()});
			setPipeline(pipeline);
		}
		if (!pipeline) {
			CITRON_CORE_ERROR("Failed to get pipeline for render object");
			return;
		}

		if (renderObject.mesh && renderObject.shader) {
			bindGroupEntries.clear();
			bindGroupEntries.push_back({.binding = 0,
										.resource = context.rendererResourcesManager.getEntityModelUniformBuffer(renderObject.entityUUID, renderObject.modelUniforms, true).buffer,
										.offset = 0,
										.size = Shader::paddedSizeof<ModelUniforms>()});
			wgpu::BindGroup modelBindGroup = renderer.getBindGroup({
				.layout = renderObject.shader->getBindGroupLayout(1),
				.entries = bindGroupEntries,
			});

			bindGroupEntries.clear();
			bindGroupEntries.push_back({.binding = 0,
										.resource = renderObject.material->getMaterialUniformBuffer().buffer,
										.offset = 0,
										.size = Shader::paddedSizeof<MaterialUniforms>()});
			wgpu::BindGroup materialBindGroup = renderer.getBindGroup({
				.layout = renderObject.shader->getBindGroupLayout(2),
				.entries = bindGroupEntries,
			});

			if (!frameBindGroup || !materialBindGroup) {
				CITRON_CORE_ERROR("Failed to get bind groups from cache for render object");
				continue;
			}

			setMesh(renderObject.mesh);
			setBindGroup(1, modelBindGroup);
			setBindGroup(2, materialBindGroup);
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

	rendererResourcesManager.initResources();

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

std::vector<RenderObject> Renderer::sortByShader(std::vector<RenderObject> &renderables, int start, int end) {
	if (end == -1)
		end = renderables.size();
	std::sort(renderables.begin() + start, renderables.begin() + end, [](const RenderObject &a, const RenderObject &b) {
		return a.shader->getUUID() < b.shader->getUUID();
	});
	return renderables;
}

std::vector<RenderObject> Renderer::sortByMesh(std::vector<RenderObject> &renderables, int start, int end) {
	if (end == -1)
		end = renderables.size();
	std::sort(renderables.begin() + start, renderables.begin() + end, [](const RenderObject &a, const RenderObject &b) {
		return a.mesh->getUUID() < b.mesh->getUUID();
	});
	return renderables;
}

std::vector<RenderObject> Renderer::sortByMaterial(std::vector<RenderObject> &renderables, int start, int end) {
	if (end == -1)
		end = renderables.size();
	std::sort(renderables.begin() + start, renderables.begin() + end, [](const RenderObject &a, const RenderObject &b) {
		return a.material->getUUID() < b.material->getUUID();
	});
	return renderables;
}
