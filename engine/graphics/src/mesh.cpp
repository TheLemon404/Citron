#include "mesh.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "buffer.hpp"
#include "glm/fwd.hpp"
#include <webgpu/webgpu.hpp>

using namespace CitronGraphics;

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, Device &device) : Asset<Mesh, AssetType::MESH>(UUID()), vertices(vertices), indices(indices), device(device) {
	wgpu::BufferDescriptor vertexBufferDesc = {};
	vertexBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
	vertexBufferDesc.size = vertices.size() * sizeof(Vertex);
	vertexBufferDesc.mappedAtCreation = false;

	vertexBuffer.buffer = device.getWGPUDevice().createBuffer(vertexBufferDesc);

	wgpu::BufferDescriptor indexBufferDesc = {};
	indexBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index;
	indexBufferDesc.size = indices.size() * sizeof(uint32_t);
	indexBufferDesc.mappedAtCreation = false;

	indexBuffer.buffer = device.getWGPUDevice().createBuffer(indexBufferDesc);

	device.getQueue().writeBuffer(vertexBuffer.buffer, 0, vertices.data(), vertexBufferDesc.size);
	device.getQueue().writeBuffer(indexBuffer.buffer, 0, indices.data(), indexBufferDesc.size);
}

std::shared_ptr<AssetBase> MeshImporter::importAsset(AssetMetadata metadata) {
	CITRON_CORE_INFO("Importing geometry asset {}", metadata.assetPath.string());

	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(metadata.assetPath.string(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		CITRON_CORE_ERROR("Assimp failed to load scene: {}", importer.GetErrorString());
		return nullptr;
	}

	CITRON_CORE_ASSERT(scene->mRootNode->mNumMeshes > 0, "Mesh scene does not contain any meshes");
	aiMesh *mesh = scene->mMeshes[scene->mRootNode->mMeshes[0]];

	std::vector<CitronGraphics::Vertex> vertices;
	std::vector<uint32_t> indices;

	for (size_t i = 0; i < mesh->mNumVertices; i++) {
		CitronGraphics::Vertex v = CitronGraphics::Vertex(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertices.push_back(v);
	}
	for (size_t i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	CITRON_CORE_INFO("Mesh {} loaded: {} vertices, {} indices", metadata.assetPath.string(), vertices.size(), indices.size());

	return std::make_shared<Mesh>(vertices, indices, device);
}
