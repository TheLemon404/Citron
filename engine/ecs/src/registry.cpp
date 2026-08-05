#include "registry.hpp"
#include "assets.hpp"
#include "component.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "test_system.hpp"

using namespace CitronECS;

std::unordered_map<uint32_t, ComponentMetadata> ECSRegistry::m_componentRegistry;
std::unordered_map<uint32_t, PropertyGuiDrawer> ECSRegistry::m_propertyGuiDrawers;
std::unordered_map<uint32_t, SystemMetadata> ECSRegistry::m_systemRegistry;

void ECSRegistry::registerDefaultComponents() {
	registerComponent<EntityBaseComponent>("Entity Base Component");
	registerComponentMember<EntityBaseComponent, UUID>("uuid", offsetof(EntityBaseComponent, uuid));
	registerComponentMember<EntityBaseComponent, std::string>("name", offsetof(EntityBaseComponent, name));
	registerComponent<TransformComponent>("Transform Component");
	registerComponentMember<TransformComponent, glm::vec3>("position", offsetof(TransformComponent, position));
	registerComponentMember<TransformComponent, glm::quat>("rotation", offsetof(TransformComponent, rotation));
	registerComponentMember<TransformComponent, glm::vec3>("scale", offsetof(TransformComponent, scale));
	registerComponent<MeshComponent>("Mesh Component");
	registerComponentMember<MeshComponent, AssetReference<Mesh>>("mesh", offsetof(MeshComponent, meshAsset));
	registerComponentMember<MeshComponent, AssetReference<Material>>("material", offsetof(MeshComponent, materialAsset));
}

void ECSRegistry::registerDefaultSystems() {
	registerSystem<TestSystem>("Test System");
}
