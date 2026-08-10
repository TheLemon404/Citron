#include "renderer.hpp"
#include "assets.hpp"
#include "buffer.hpp"
#include "compiled_shaders.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "resources.hpp"
#include "shader.hpp"
#include "view.hpp"
#include <cstdint>
#include <webgpu.h>
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
		colorAttachment.loadOp = attachment.loadOp;
		colorAttachment.storeOp = wgpu::StoreOp::Store;
		colorAttachment.clearValue = attachment.clearValue;
		renderPassColorAttachments.push_back(colorAttachment);
		colorAttachmentFormats.push_back(attachment.textureFormat);
	}

	wgpu::RenderPassDepthStencilAttachment depthStencilAttachment = {};
	if (params.containsDepthStencil) {
		depthStencilAttachment.view = params.depthStencilAttachment.targetTexture.getTextureView();
		depthStencilAttachment.depthLoadOp = params.depthLoadOp;
		depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
		depthStencilAttachment.stencilLoadOp = params.stencilLoadOp;
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

void RenderPass::drawFullscreenQuadPass(std::shared_ptr<Mesh> fullscreenQuad, std::shared_ptr<Shader> shader) {
	RendererContext context = renderer.getContext();

	std::shared_ptr<Pipeline> pipeline = context.rendererResourcesManager.getPipeline({shader, getColorAttachmentFormats(), params.containsDepthStencil});
	if (!pipeline) {
		CITRON_CORE_ERROR("Failed to get pipeline for render object");
		return;
	}
	setPipeline(pipeline);

	wgpu::BindGroup bindGroup = context.rendererResourcesManager.getBindGroup({
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
				.resource = context.rendererResourcesManager.getFrameUniformsBuffer().buffer,
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

void RenderPass::drawDebugGrid() {
	RendererContext context = renderer.getContext();

	std::shared_ptr<Pipeline> debugPipeline = context.rendererResourcesManager.getPipeline(PipelineKey{
		.shader = context.rendererResourcesManager.getDebugGridShader(),
		.colorAttachmentFormats = getColorAttachmentFormats(),
		.hasDepthStencilAttachment = false,
		.cullMode = PipelineCullMode::None,
	});

	wgpu::BindGroup frameBindGroup = context.rendererResourcesManager.getBindGroup(
		{.layout = context.rendererResourcesManager.getDebugGridShader()->getBindGroupLayout(0),
		 .entries = {
			 {.binding = 0,
			  .resource = context.rendererResourcesManager.getFrameUniformsBuffer().buffer,
			  .offset = 0,
			  .size = Shader::paddedSizeof<FrameUniforms>()}}});

	wgpu::BindGroup drawBindGroup = context.rendererResourcesManager.getBindGroup(
		{.layout = context.rendererResourcesManager.getDebugGridShader()->getBindGroupLayout(1),
		 .entries = {
			 {.binding = 0,
			  .resource = context.rendererResourcesManager.getDrawUniformBuffer(parentFrame.getRenderCount()).buffer,
			  .offset = 0,
			  .size = Shader::paddedSizeof<DrawUniforms>()},
			 {
				 .binding = 1,
				 .resource = renderer.depthBufferTexture.getTextureView(),
				 .offset = 0,
				 .size = WGPU_WHOLE_SIZE,
			 }}});
	setPipeline(debugPipeline);
	setBindGroup(0, frameBindGroup);
	setBindGroup(1, drawBindGroup);
	setMesh(context.rendererResourcesManager.getDebugGridMesh());
	draw(context.rendererResourcesManager.getDebugGridMesh());
}

void RenderPass::drawRenderData(std::vector<RenderObject> renderObjects) {
	if (renderObjects.size() == 0)
		return;

	RendererContext context = renderer.getContext();

	// sorting
	std::shared_ptr<Pipeline> pipeline = nullptr;

	// risky to assume all shaders use the same frame uniforms, but this should be the case.
	wgpu::BindGroup frameBindGroup = context.rendererResourcesManager.getBindGroup(
		{.layout = renderObjects[0].shader->getBindGroupLayout(0),
		 .entries = {
			 {.binding = 0,
			  .resource = context.rendererResourcesManager.getFrameUniformsBuffer().buffer,
			  .offset = 0,
			  .size = Shader::paddedSizeof<FrameUniforms>()}}});

	setBindGroup(0, frameBindGroup);

	// get bind group for frame uniforms
	for (size_t i = 0; i < renderObjects.size(); i++) {
		RenderObject &renderObject = renderObjects[i];
		if (pipeline == nullptr || i > 0 && renderObjects[i].shader->getUUID() != renderObjects[i - 1].shader->getUUID()) {
			pipeline = context.rendererResourcesManager.getPipeline({
				.shader = renderObject.shader,
				.colorAttachmentFormats = getColorAttachmentFormats(),
				.hasDepthStencilAttachment = params.containsDepthStencil,
			});
			setPipeline(pipeline);

			// for now, we just reset the draw bind groups each time we change the pipeline. This needs to be optimized in the future.
			wgpu::BindGroup drawBindGroup = context.rendererResourcesManager.getBindGroup(
				{.layout = renderObject.shader->getBindGroupLayout(1),
				 .entries = {
					 {.binding = 0,
					  .resource = context.rendererResourcesManager.getDrawUniformBuffer(parentFrame.getRenderCount()).buffer,
					  .offset = 0,
					  .size = Shader::paddedSizeof<DrawUniforms>()}}});
			setBindGroup(1, drawBindGroup);
		}

		GPUBuffer modelUniformsBuffer = context.rendererResourcesManager.getEntityModelUniformBuffer(renderObject.entityUUID, renderObject.modelUniforms);
		context.device.getQueue().writeBuffer(modelUniformsBuffer.buffer, 0, &renderObject.modelUniforms, Shader::paddedSizeof<ModelUniforms>());

		if (renderObject.mesh && renderObject.shader) {
			wgpu::BindGroup modelBindGroup = context.rendererResourcesManager.getBindGroup({
				.layout = renderObject.shader->getBindGroupLayout(2),
				.entries = {
					{.binding = 0,
					 .resource = modelUniformsBuffer.buffer,
					 .offset = 0,
					 .size = Shader::paddedSizeof<ModelUniforms>()}},
			});

			wgpu::BindGroup materialBindGroup = context.rendererResourcesManager.getBindGroup({
				.layout = renderObject.shader->getBindGroupLayout(3),
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
			setBindGroup(2, modelBindGroup);
			setBindGroup(3, materialBindGroup);
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
	createRenderTargetColorTexture(idBufferTexture, window.getWidth(), window.getHeight());
	createRenderTargetColorTexture(colorBufferTexture, window.getWidth(), window.getHeight());
	createRenderTargetColorTexture(normalBufferTexture, window.getWidth(), window.getHeight());
	createRenderTargetColorTexture(lightingBufferTexture, window.getWidth(), window.getHeight());

	fullscreenQuad = Mesh::createFullscreenQuad(device, assetManager);
	lightingPassShader = assetManager.createAsset<Shader>(device, CompiledShaders::lighting_pass);
}

Frame Renderer::beginFrame() {
	deviceSurfaceTexture = Texture{
		device.getCurrentSurfaceTexture().texture, (uint32_t)device.getLastSurfaceWidth(), (uint32_t)device.getLastSurfaceHeight()};
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

	deviceSurfaceTexture.release();
	rendererResourcesManager.releaseUnusedBindGroups();
}

void Renderer::render(Frame &frame, View &view, std::vector<RenderableReferenceData> renderableReferenceData, glm::ivec2 viewportSize, Texture &outputTexture, bool drawDebug) {
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

		if (!mesh) {
			CITRON_CORE_ERROR("Failed to get mesh for render object");
			continue;
		}

		if (!shader) {
			CITRON_CORE_ERROR("Failed to get shader for render object");
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

	DrawUniforms drawUniforms = {};
	drawUniforms.view = view.getViewMatrix();
	drawUniforms.projection = view.getProjectionMatrix();
	GPUBuffer drawUniformsBuffer = rendererResourcesManager.getDrawUniformBuffer(frame.getRenderCount());
	device.getQueue().writeBuffer(drawUniformsBuffer.buffer, 0, &drawUniforms, Shader::paddedSizeof<DrawUniforms>());

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
	gBufferPass.drawRenderData(renderObjectCache.renderObjects);
	gBufferPass.end();

	RenderPassColorAttachment lightingAttachment = {};
	lightingAttachment.targetTexture = outputTexture;
	RenderPassParams lightingPassParams = {};
	lightingPassParams.colorAttachments.push_back(lightingAttachment);
	RenderPass lightingPass = frame.beginRenderPass(lightingPassParams);
	lightingPass.drawFullscreenQuadPass(fullscreenQuad, lightingPassShader);
	lightingPass.end();
	// debug grid
	if (drawDebug) {
		RenderPassColorAttachment debugAttachment = {
			.loadOp = wgpu::LoadOp::Load,
		};
		debugAttachment.targetTexture = outputTexture;
		RenderPassParams debugPassParams = {};
		debugPassParams.colorAttachments.push_back(debugAttachment);
		debugPassParams.depthStencilAttachment = depthAttachment;
		debugPassParams.depthLoadOp = wgpu::LoadOp::Load;
		debugPassParams.stencilLoadOp = wgpu::LoadOp::Load;
		RenderPass debugPass = frame.beginRenderPass(debugPassParams);
		debugPass.drawDebugGrid();
		debugPass.end();
	}

	frame.incrementRenderCount();
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

void Renderer::createRenderTargetColorTexture(Texture &texture, uint32_t width, uint32_t height) {
	texture = {device.createRenderTargetColorTexture(width, height), width, height};
}
