#include "mesh.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "buffer.hpp"
#include "glm/fwd.hpp"
#include <webgpu/webgpu.hpp>

using namespace CitronGraphics;

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, glm::vec3 worldSpaceBoundsMin, glm::vec3 worldSpaceBoundsMax, Device &device) : Asset<Mesh, AssetType::MESH>(UUID()), vertices(vertices), indices(indices), boundsMin(worldSpaceBoundsMin), boundsMax(worldSpaceBoundsMax), device(device) {
	wgpu::BufferDescriptor vertexBufferDesc = {};
	vertexBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
	vertexBufferDesc.size = vertices.size() * sizeof(Vertex);
	vertexBufferDesc.mappedAtCreation = false;

	vertexBuffer.buffer = device.getWGPUDevice().createBuffer(vertexBufferDesc);
	vertexBuffer.size = vertices.size() * sizeof(Vertex);
	vertexBuffer.entryCount = vertices.size();

	wgpu::BufferDescriptor indexBufferDesc = {};
	indexBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index;
	indexBufferDesc.size = indices.size() * sizeof(uint32_t);
	indexBufferDesc.mappedAtCreation = false;

	indexBuffer.buffer = device.getWGPUDevice().createBuffer(indexBufferDesc);
	indexBuffer.size = indices.size() * sizeof(uint32_t);
	indexBuffer.entryCount = indices.size();

	device.getQueue().writeBuffer(vertexBuffer.buffer, 0, vertices.data(), vertexBufferDesc.size);
	device.getQueue().writeBuffer(indexBuffer.buffer, 0, indices.data(), indexBufferDesc.size);
}

std::shared_ptr<AssetBase> MeshImporter::importAsset(AssetMetadata metadata) {
	CITRON_CORE_INFO("Importing geometry asset {}", metadata.assetPath.string());

	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(metadata.assetPath.string(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenBoundingBoxes);
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
		v.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		if (mesh->HasVertexColors(i)) {
			v.color = glm::vec3(mesh->mColors[i]->r, mesh->mColors[i]->g, mesh->mColors[i]->b);
		}
		if (mesh->HasTextureCoords(i)) {
			v.uv = glm::vec2(mesh->mTextureCoords[i]->x, mesh->mTextureCoords[i]->y);
		}
		vertices.push_back(v);
	}
	for (size_t i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	aiAABB aabb = mesh->mAABB;
	glm::vec3 worldSpaceBoundsMin = glm::vec3(aabb.mMin.x, aabb.mMin.y, aabb.mMin.z);
	glm::vec3 worldSpaceBoundsMax = glm::vec3(aabb.mMax.x, aabb.mMax.y, aabb.mMax.z);

	CITRON_CORE_INFO("Mesh {} loaded: {} vertices, {} indices", metadata.assetPath.string(), vertices.size(), indices.size());

	return std::make_shared<Mesh>(vertices, indices, worldSpaceBoundsMin, worldSpaceBoundsMax, device);
}

std::shared_ptr<Mesh> Mesh::createFullscreenQuad(Device &device) {
	std::vector<CitronGraphics::Vertex> vertices = {
		CitronGraphics::Vertex(-1.0f, -1.0f, 0.0f),
		CitronGraphics::Vertex(1.0f, -1.0f, 0.0f),
		CitronGraphics::Vertex(1.0f, 1.0f, 0.0f),
		CitronGraphics::Vertex(-1.0f, 1.0f, 0.0f),
	};
	std::vector<uint32_t> indices = {
		0,
		1,
		2,
		2,
		3,
		0,
	};
	return std::make_shared<Mesh>(vertices, indices, glm::vec3(0.0), glm::vec3(1.0f, 1.0f, 0.0f), device);
}
