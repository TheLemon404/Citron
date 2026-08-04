#include "component_registry.hpp"
#include "assets.hpp"
#include "component.hpp"
#include "material.hpp"
#include "mesh.hpp"

using namespace CitronECS;

std::unordered_map<uint32_t, ComponentMetadata> ComponentRegistry::m_componentRegistry;
std::unordered_map<uint32_t, PropertyGuiDrawer> ComponentRegistry::m_propertyGuiDrawers;

void ComponentRegistry::registerDefaultComponents() {
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
