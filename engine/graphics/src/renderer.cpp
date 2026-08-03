#include "renderer.hpp"
#include "buffer.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "resources.hpp"
#include "view.hpp"
#include <event.hpp>
#include <logger.hpp>
#include <webgpu/webgpu.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>

using namespace CitronGraphics;

RenderPass::RenderPass(Renderer &renderer, RenderPassParams &params,
					   wgpu::CommandEncoder &commandEncoder, Frame &parentFrame)
	: renderer(renderer), commandEncoder(commandEncoder),
	  params(params), parentFrame(parentFrame) {

	// Render pass encoder setup

	for (RenderPassColorAttachment attachment : params.colorAttachments) {
		wgpu::RenderPassColorAttachment colorAttachment = {};
		colorAttachment.nextInChain = nullptr;
		colorAttachment.view = attachment.targetTextureView;
		colorAttachment.resolveTarget = nullptr;
		colorAttachment.loadOp = wgpu::LoadOp::Clear;
		colorAttachment.storeOp = wgpu::StoreOp::Store;
		colorAttachment.clearValue = attachment.clearValue;
		renderPassColorAttachments.push_back(colorAttachment);
	}

	wgpu::RenderPassDepthStencilAttachment depthStencilAttachment = {};
	if (params.containsDepthStencil) {
		depthStencilAttachment.view = params.depthStencilAttachment.targetTextureView;
		depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
		depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
		depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Clear;
		depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
		depthStencilAttachment.depthClearValue = params.depthStencilAttachment.depthClearValue;
		depthStencilAttachment.stencilClearValue = params.depthStencilAttachment.stencilClearValue;
	}

	wgpu::RenderPassDescriptor renderPassDescriptor = {};
	renderPassDescriptor.nextInChain = nullptr;
	renderPassDescriptor.colorAttachmentCount = renderPassColorAttachments.size();
	renderPassDescriptor.colorAttachments = renderPassColorAttachments.data();
	renderPassDescriptor.depthStencilAttachment = params.containsDepthStencil ? &depthStencilAttachment : nullptr;
	renderPassEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
}

void RenderPass::drawRenderData(std::vector<RenderableReferenceData> renderableReferenceData, RenderPass &renderPass) {
	std::vector<RenderObject> renderObjects;
	RendererContext context = renderer.getContext();
	std::vector<wgpu::RenderPassColorAttachment> colorAttachments = renderPass.renderPassColorAttachments;

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

	// sorting
	renderObjects = Renderer::sortByShader(renderObjects);
	std::shared_ptr<Pipeline> pipeline = renderer.getPipeline({renderObjects[0].shader, colorAttachments, renderPass.getParams().containsDepthStencil, context.device.getWGPUPreferredSurfaceFormat()});
	setPipeline(pipeline);

	// update frame uniforms if needed
	FrameUniforms lastFrameUniforms = context.rendererResourcesManager.frameUniforms;
	context.rendererResourcesManager.frameUniforms.viewProjection = getParentFrame().getView().getProjectionMatrix() * getParentFrame().getView().getViewMatrix();
	if (lastFrameUniforms != context.rendererResourcesManager.frameUniforms) {
		context.device.getWGPUDevice().getQueue().writeBuffer(context.rendererResourcesManager.frameUniformBuffer.buffer, 0, &context.rendererResourcesManager.frameUniforms, Shader::paddedSizeof<FrameUniforms>());
	}

	// get bind group for frame uniforms
	std::vector<BindGroupEntry> bindGroupEntries;
	bindGroupEntries.push_back({.binding = 0,
								.resource = context.rendererResourcesManager.frameUniformBuffer.buffer,
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
			pipeline = renderer.getPipeline({renderObject.shader, colorAttachments, renderPass.getParams().containsDepthStencil, context.device.getWGPUPreferredSurfaceFormat()});
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
			 wgpu::SurfaceTexture &surfaceTexture, View &view)
	: renderer(renderer), encoder(encoder), view(view) {
}

RenderPass Frame::beginRenderPass(RenderPassParams &params) {
	return RenderPass(renderer, params, encoder, *this);
}

void Renderer::init() {
	device.aquirePlatformResources();

	rendererResourcesManager.initResources();

	depthBufferTexture = device.createRenderTargetDepthTexture(window.getWidth(), window.getHeight());
	depthBufferTextureView = device.createTextureView(depthBufferTexture);
	colorBufferTexture = device.createRenderTargetColorTexture(window.getWidth(), window.getHeight());
	colorBufferTextureView = device.createTextureView(colorBufferTexture);
	normalBufferTexture = device.createRenderTargetColorTexture(window.getWidth(), window.getHeight());
	normalBufferTextureView = device.createTextureView(normalBufferTexture);
}

Frame Renderer::beginFrame(View &view) {
	return Frame(*this,
				 device.getWGPUDevice().createCommandEncoder(),
				 device.getCurrentSurfaceTexture(), view);
}

void Renderer::endFrame(Frame &frame) {
	wgpu::CommandBuffer commandBuffer = frame.getEncoder().finish();
	frame.getEncoder().release();
	device.getQueue().submit(commandBuffer);

	commandBuffer.release();

	device.presentCurrentSurfaceTexture();
}

void Renderer::render(Frame &frame, std::vector<RenderableReferenceData> renderableReferenceData, glm::vec2 viewportSize) {
	resizeRenderTargets(viewportSize);
	RenderPassDepthStencilAttachment depthAttachment = {};
	depthAttachment.targetTexture = depthBufferTexture;
	depthAttachment.targetTextureView = depthBufferTextureView;
	RenderPassColorAttachment colorAttachment = {};
	colorAttachment.targetTexture = colorBufferTexture;
	colorAttachment.targetTextureView = colorBufferTextureView;
	RenderPassColorAttachment normalAttachment = {};
	normalAttachment.targetTexture = normalBufferTexture;
	normalAttachment.targetTextureView = normalBufferTextureView;
	RenderPassParams gBufferPassParams = {};
	gBufferPassParams.containsDepthStencil = true;
	gBufferPassParams.depthStencilAttachment = depthAttachment;
	gBufferPassParams.colorAttachments.push_back(colorAttachment);
	gBufferPassParams.colorAttachments.push_back(normalAttachment);
	RenderPass gBufferPass = frame.beginRenderPass(gBufferPassParams);
	gBufferPass.drawRenderData(renderableReferenceData, gBufferPass);
	gBufferPass.end();
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

void Renderer::resizeRenderTargets(glm::vec2 viewportSize) {
	if (viewportSize.x != depthBufferTexture.getWidth() || viewportSize.y != depthBufferTexture.getHeight()) {
		depthBufferTexture.release();
		depthBufferTexture = device.createRenderTargetDepthTexture(viewportSize.x, viewportSize.y);
		depthBufferTextureView.release();
		depthBufferTextureView = depthBufferTexture.createView();
	}

	if (viewportSize.x != colorBufferTexture.getWidth() || viewportSize.y != colorBufferTexture.getHeight()) {
		colorBufferTexture.release();
		colorBufferTexture = device.createRenderTargetColorTexture(viewportSize.x, viewportSize.y);
		colorBufferTextureView.release();
		colorBufferTextureView = colorBufferTexture.createView();
	}

	if (viewportSize.x != normalBufferTexture.getWidth() || viewportSize.y != normalBufferTexture.getHeight()) {
		normalBufferTexture.release();
		normalBufferTexture = device.createRenderTargetColorTexture(viewportSize.x, viewportSize.y);
		normalBufferTextureView.release();
		normalBufferTextureView = normalBufferTexture.createView();
	}
}
