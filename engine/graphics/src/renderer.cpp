#include "renderer.hpp"
#include "buffer.hpp"
#include "compiled_shaders.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "resources.hpp"
#include "shader.hpp"
#include "view.hpp"
#include <cstdint>
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
		colorAttachment.view = attachment.targetTexture.getTextureView();
		colorAttachment.resolveTarget = nullptr;
		colorAttachment.loadOp = wgpu::LoadOp::Clear;
		colorAttachment.storeOp = wgpu::StoreOp::Store;
		colorAttachment.clearValue = attachment.clearValue;
		renderPassColorAttachments.push_back(colorAttachment);
		colorAttachmentFormats.push_back(attachment.textureFormat);
	}

	wgpu::RenderPassDepthStencilAttachment depthStencilAttachment = {};
	if (params.containsDepthStencil) {
		depthStencilAttachment.view = params.depthStencilAttachment.targetTexture.getTextureView();
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

	wgpu::BindGroup bindGroup = renderer.getBindGroup({
		.layout = shader->getBindGroupLayout(0),
		.entries = {
			{
				.binding = 0,
				.resource = renderer.colorBufferTexture.getTextureView(),
				.offset = 0,
				.size = WGPU_WHOLE_SIZE,
			},
			{
				.binding = 1,
				.resource = renderer.normalBufferTexture.getTextureView(),
				.offset = 0,
				.size = WGPU_WHOLE_SIZE,
			},
			{
				.binding = 2,
				.resource = context.rendererResourcesManager.frameUniformBuffer.buffer,
				.offset = 0,
				.size = Shader::paddedSizeof<FrameUniforms>(),
			}},
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
	std::shared_ptr<Pipeline> pipeline = nullptr;

	// get bind group for frame uniforms

	for (size_t i = 0; i < renderObjects.size(); i++) {
		RenderObject &renderObject = renderObjects[i];
		if (pipeline == nullptr || i > 0 && renderObjects[i].shader->getUUID() != renderObjects[i - 1].shader->getUUID()) {
			pipeline = renderer.getPipeline({renderObject.shader, renderPass.getColorAttachmentFormats(), renderPass.getParams().containsDepthStencil});
			setPipeline(pipeline);

			// for now, we just reset the frame bind groups each time we change the pipeline. This needs to be optimized in the future.
			wgpu::BindGroup frameBindGroup = renderer.getBindGroup(
				{.layout = renderObject.shader->getBindGroupLayout(0),
				 .entries = {
					 {.binding = 0,
					  .resource = context.rendererResourcesManager.frameUniformBuffer.buffer,
					  .offset = 0,
					  .size = Shader::paddedSizeof<FrameUniforms>()}}});
			setBindGroup(0, frameBindGroup);
		}

		if (renderObject.mesh && renderObject.shader) {
			wgpu::BindGroup modelBindGroup = renderer.getBindGroup({
				.layout = renderObject.shader->getBindGroupLayout(1),
				.entries = {
					{.binding = 0,
					 .resource = context.rendererResourcesManager.getEntityModelUniformBuffer(renderObject.entityUUID, renderObject.modelUniforms).buffer,
					 .offset = 0,
					 .size = Shader::paddedSizeof<ModelUniforms>()}},
			});

			wgpu::BindGroup materialBindGroup = renderer.getBindGroup({
				.layout = renderObject.shader->getBindGroupLayout(2),
				.entries = {
					{.binding = 0,
					 .resource = renderObject.material->getMaterialUniformBuffer().buffer,
					 .offset = 0,
					 .size = Shader::paddedSizeof<MaterialUniforms>()}},
			});

			if (!materialBindGroup || !modelBindGroup) {
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

RenderPass Frame::beginRenderPass(RenderPassParams &params) {
	return RenderPass(renderer, params, encoder, *this);
}

void Renderer::init() {
	device.aquirePlatformResources();

	rendererResourcesManager.initResources();

	depthBufferTexture = {device.createRenderTargetDepthTexture(window.getWidth(), window.getHeight()), (uint32_t)window.getWidth(), (uint32_t)window.getHeight()};
	depthBufferTexture.regenerateTextureView();
	idBufferTexture = {device.createRenderTargetColorTexture(window.getWidth(), window.getHeight()), (uint32_t)window.getWidth(), (uint32_t)window.getHeight()};
	idBufferTexture.regenerateTextureView();
	colorBufferTexture = {device.createRenderTargetColorTexture(window.getWidth(), window.getHeight()), (uint32_t)window.getWidth(), (uint32_t)window.getHeight()};
	colorBufferTexture.regenerateTextureView();
	normalBufferTexture = {device.createRenderTargetColorTexture(window.getWidth(), window.getHeight()), (uint32_t)window.getWidth(), (uint32_t)window.getHeight()};
	normalBufferTexture.regenerateTextureView();
	lightingBufferTexture = {device.createRenderTargetColorTexture(window.getWidth(), window.getHeight()), (uint32_t)window.getWidth(), (uint32_t)window.getHeight()};
	lightingBufferTexture.regenerateTextureView();

	fullscreenQuad = Mesh::createFullscreenQuad(device);
	lightingPassShader = std::make_shared<Shader>(UUID(), device, lighting_pass);
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

void Renderer::render(Frame &frame, View &view, std::vector<RenderableReferenceData> renderableReferenceData, glm::ivec2 viewportSize, Texture &outputTexture) {
	resizeRenderTargets(viewportSize);

	// need to cache assets already gethered in previous frames
	renderObjectCache.renderObjects.clear();

	for (size_t i = 0; i < renderableReferenceData.size(); i++) {
		std::shared_ptr<Mesh> mesh = assetManager.getAsset<Mesh>(renderableReferenceData[i].meshUUID);
		// frustrum culling
		glm::vec4 transformedMin = glm::vec4(mesh->getBoundsMin(), 1.0f) * renderableReferenceData[i].transform;
		glm::vec4 transformedMax = glm::vec4(mesh->getBoundsMax(), 1.0f) * renderableReferenceData[i].transform;
		if (!view.isInsideBounds(glm::xyz(transformedMin)) && !view.isInsideBounds(glm::xyz(transformedMax))) {
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
			{.transform = renderableReferenceData[i].transform},
			mesh,
			material,
			shader,
		});
	}

	renderObjectCache.renderObjects = Renderer::sortByShader(renderObjectCache.renderObjects);

	// update frame uniforms if needed
	FrameUniforms lastFrameUniforms = rendererResourcesManager.frameUniforms;
	rendererResourcesManager.frameUniforms.viewProjection = view.getProjectionMatrix() * view.getViewMatrix();
	if (lastFrameUniforms != rendererResourcesManager.frameUniforms) {
		device.getWGPUDevice().getQueue().writeBuffer(rendererResourcesManager.frameUniformBuffer.buffer, 0, &rendererResourcesManager.frameUniforms, Shader::paddedSizeof<FrameUniforms>());
	}

	// depth and gbuffer pass
	RenderPassDepthStencilAttachment depthAttachment = {};
	depthAttachment.targetTexture = depthBufferTexture;
	RenderPassColorAttachment colorAttachment = {};
	colorAttachment.targetTexture = colorBufferTexture;
	RenderPassColorAttachment normalAttachment = {};
	normalAttachment.targetTexture = normalBufferTexture;
	RenderPassColorAttachment idAttachment = {};
	idAttachment.targetTexture = idBufferTexture;
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
	lightingAttachment.targetTexture = outputTexture;
	RenderPassParams lightingPassParams = {};
	lightingPassParams.colorAttachments.push_back(lightingAttachment);
	RenderPass lightingPass = frame.beginRenderPass(lightingPassParams);
	lightingPass.drawFullscreenQuadPass(fullscreenQuad, lightingPassShader, lightingPass);
	lightingPass.end();
}

void Renderer::end() {
	rendererResourcesManager.releaseResources();
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

void Renderer::resizeRenderTargets(glm::ivec2 viewportSize) {
	if (viewportSize.x != idBufferTexture.getWidth() || viewportSize.y != idBufferTexture.getHeight()) {
		idBufferTexture.release();
		idBufferTexture = {device.createRenderTargetColorTexture(viewportSize.x, viewportSize.y), (uint32_t)viewportSize.x, (uint32_t)viewportSize.y};
		idBufferTexture.regenerateTextureView();
	}

	if (viewportSize.x != depthBufferTexture.getWidth() || viewportSize.y != depthBufferTexture.getHeight()) {
		depthBufferTexture.release();
		depthBufferTexture = {device.createRenderTargetDepthTexture(viewportSize.x, viewportSize.y), (uint32_t)viewportSize.x, (uint32_t)viewportSize.y};
		depthBufferTexture.regenerateTextureView();
	}

	if (viewportSize.x != colorBufferTexture.getWidth() || viewportSize.y != colorBufferTexture.getHeight()) {
		colorBufferTexture.release();
		colorBufferTexture = {device.createRenderTargetColorTexture(viewportSize.x, viewportSize.y), (uint32_t)viewportSize.x, (uint32_t)viewportSize.y};
		colorBufferTexture.regenerateTextureView();
	}

	if (viewportSize.x != normalBufferTexture.getWidth() || viewportSize.y != normalBufferTexture.getHeight()) {
		normalBufferTexture.release();
		normalBufferTexture = {device.createRenderTargetColorTexture(viewportSize.x, viewportSize.y), (uint32_t)viewportSize.x, (uint32_t)viewportSize.y};
		normalBufferTexture.regenerateTextureView();
	}

	if (viewportSize.x != lightingBufferTexture.getWidth() || viewportSize.y != lightingBufferTexture.getHeight()) {
		lightingBufferTexture.release();
		lightingBufferTexture = {device.createRenderTargetColorTexture(viewportSize.x, viewportSize.y), (uint32_t)viewportSize.x, (uint32_t)viewportSize.y};
		lightingBufferTexture.regenerateTextureView();
	}
}
