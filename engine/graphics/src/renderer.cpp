#include "renderer.hpp"
#include "buffer.hpp"
#include "compiled_shaders.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "resources.hpp"
#include "view.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vec_swizzle.hpp>
#include <event.hpp>
#include <logger.hpp>
#include <webgpu/webgpu.hpp>
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
		colorAttachmentFormats.push_back(attachment.textureFormat);
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

void RenderPass::drawFullscreenQuadPass(std::shared_ptr<Mesh> fullscreenQuad, std::shared_ptr<Shader> shader, RenderPass &renderPass) {
	RendererContext context = renderer.getContext();

	std::shared_ptr<Pipeline> pipeline = renderer.getPipeline({shader, renderPass.getColorAttachmentFormats(), renderPass.getParams().containsDepthStencil});
	if (!pipeline) {
		CITRON_CORE_ERROR("Failed to get pipeline for render object");
		return;
	}
	setPipeline(pipeline);

	bindGroupEntries.push_back({
		.binding = 0,
		.resource = renderer.colorBufferTextureView,
		.offset = 0,
		.size = WGPU_WHOLE_SIZE,
	});
	bindGroupEntries.push_back({
		.binding = 1,
		.resource = renderer.normalBufferTextureView,
		.offset = 0,
		.size = WGPU_WHOLE_SIZE,
	});
	bindGroupEntries.push_back({
		.binding = 2,
		.resource = context.rendererResourcesManager.frameUniformBuffer.buffer,
		.offset = 0,
		.size = WGPU_WHOLE_SIZE,
	});

	wgpu::BindGroup bindGroup = renderer.getBindGroup({
		.layout = shader->getBindGroupLayout(0),
		.entries = bindGroupEntries,
	});
	setBindGroup(0, bindGroup);

	if (fullscreenQuad && shader) {
		setMesh(fullscreenQuad);
		draw(fullscreenQuad);
	}
}

void RenderPass::drawRenderData(std::vector<RenderObject> renderObjects, RenderPass &renderPass) {
	if (renderObjects.size() == 0)
		return;

	RendererContext context = renderer.getContext();

	// sorting
	std::shared_ptr<Pipeline> pipeline = renderer.getPipeline({renderObjects[0].shader, renderPass.getColorAttachmentFormats(), renderPass.getParams().containsDepthStencil});
	setPipeline(pipeline);

	// get bind group for frame uniforms
	bindGroupEntries.clear();
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
			pipeline = renderer.getPipeline({renderObject.shader, renderPass.getColorAttachmentFormats(), renderPass.getParams().containsDepthStencil});
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
	idBufferTexture = device.createRenderTargetColorTexture(window.getWidth(), window.getHeight());
	idBufferTextureView = device.createTextureView(idBufferTexture);
	colorBufferTexture = device.createRenderTargetColorTexture(window.getWidth(), window.getHeight());
	colorBufferTextureView = device.createTextureView(colorBufferTexture);
	normalBufferTexture = device.createRenderTargetColorTexture(window.getWidth(), window.getHeight());
	normalBufferTextureView = device.createTextureView(normalBufferTexture);
	lightingBufferTexture = device.createRenderTargetColorTexture(window.getWidth(), window.getHeight());
	lightingBufferTextureView = device.createTextureView(lightingBufferTexture);

	fullscreenQuad = Mesh::createFullscreenQuad(device);
	lightingPassShader = std::make_shared<Shader>(UUID(), device, lighting_pass);
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

	// need to cache assets already gethered in previous frames
	renderObjectCache.renderObjects.clear();

	View &currentFrameView = frame.getView();

	for (size_t i = 0; i < renderableReferenceData.size(); i++) {
		std::shared_ptr<Mesh> mesh = assetManager.getAsset<Mesh>(renderableReferenceData[i].meshUUID);
		// frustrum culling
		glm::vec4 transformedMin = glm::vec4(mesh->getBoundsMin(), 1.0f) * renderableReferenceData[i].transform;
		glm::vec4 transformedMax = glm::vec4(mesh->getBoundsMax(), 1.0f) * renderableReferenceData[i].transform;
		if (!currentFrameView.isInsideBounds(glm::xyz(transformedMin)) && !currentFrameView.isInsideBounds(glm::xyz(transformedMax))) {
			continue;
		}
		std::shared_ptr<Material> material = assetManager.getAsset<Material>(renderableReferenceData[i].materialUUID);
		if (!material) {
			CITRON_CORE_ERROR("Failed to get material for render object");
			continue;
		}

		std::shared_ptr<Shader> shader = assetManager.getAsset<Shader>(material->shader.uuid);

		if (!mesh || !shader) {
			CITRON_CORE_ERROR("Failed to get mesh or shader for render object");
			continue;
		}

		renderObjectCache.renderObjects.push_back({
			renderableReferenceData[i].entityUUID,
			{.transform = renderableReferenceData[i].transform, .uuid = (uint32_t)(uint64_t)renderableReferenceData[i].entityUUID},
			mesh,
			material,
			shader,
		});
	}

	renderObjectCache.renderObjects = Renderer::sortByShader(renderObjectCache.renderObjects);

	// update frame uniforms if needed
	FrameUniforms lastFrameUniforms = rendererResourcesManager.frameUniforms;
	rendererResourcesManager.frameUniforms.viewProjection = currentFrameView.getProjectionMatrix() * currentFrameView.getViewMatrix();
	if (lastFrameUniforms != rendererResourcesManager.frameUniforms) {
		device.getWGPUDevice().getQueue().writeBuffer(rendererResourcesManager.frameUniformBuffer.buffer, 0, &rendererResourcesManager.frameUniforms, Shader::paddedSizeof<FrameUniforms>());
	}

	// depth and gbuffer pass
	RenderPassDepthStencilAttachment depthAttachment = {};
	depthAttachment.targetTexture = depthBufferTexture;
	depthAttachment.targetTextureView = depthBufferTextureView;
	RenderPassColorAttachment colorAttachment = {};
	colorAttachment.targetTexture = colorBufferTexture;
	colorAttachment.targetTextureView = colorBufferTextureView;
	RenderPassColorAttachment normalAttachment = {};
	normalAttachment.targetTexture = normalBufferTexture;
	normalAttachment.targetTextureView = normalBufferTextureView;
	RenderPassColorAttachment idAttachment = {};
	idAttachment.targetTexture = idBufferTexture;
	idAttachment.targetTextureView = idBufferTextureView;
	RenderPassParams gBufferPassParams = {};
	gBufferPassParams.containsDepthStencil = true;
	gBufferPassParams.depthStencilAttachment = depthAttachment;
	gBufferPassParams.colorAttachments.push_back(colorAttachment);
	gBufferPassParams.colorAttachments.push_back(normalAttachment);
	gBufferPassParams.colorAttachments.push_back(idAttachment);
	RenderPass gBufferPass = frame.beginRenderPass(gBufferPassParams);
	gBufferPass.drawRenderData(renderObjectCache.renderObjects, gBufferPass);
	gBufferPass.end();

	RenderPassColorAttachment lightingAttachment = {};
	lightingAttachment.targetTexture = lightingBufferTexture;
	lightingAttachment.targetTextureView = lightingBufferTextureView;
	RenderPassParams lightingPassParams = {};
	lightingPassParams.colorAttachments.push_back(lightingAttachment);
	RenderPass lightingPass = frame.beginRenderPass(lightingPassParams);
	lightingPass.drawFullscreenQuadPass(fullscreenQuad, lightingPassShader, lightingPass);
	lightingPass.end();
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
	if (viewportSize.x != idBufferTexture.getWidth() || viewportSize.y != idBufferTexture.getHeight()) {
		idBufferTexture.release();
		idBufferTexture = device.createRenderTargetColorTexture(viewportSize.x, viewportSize.y);
		idBufferTextureView.release();
		idBufferTextureView = idBufferTexture.createView();
	}

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

	if (viewportSize.x != lightingBufferTexture.getWidth() || viewportSize.y != lightingBufferTexture.getHeight()) {
		lightingBufferTexture.release();
		lightingBufferTexture = device.createRenderTargetColorTexture(viewportSize.x, viewportSize.y);
		lightingBufferTextureView.release();
		lightingBufferTextureView = lightingBufferTexture.createView();
	}
}
