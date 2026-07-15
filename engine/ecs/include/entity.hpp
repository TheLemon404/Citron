#pragma once

#include "entt/entity/fwd.hpp"
#include <entt/entt.hpp>
#include <layer.hpp>

using namespace CitronCore;

namespace CitronECS {
class EntityLayer : public Layer {
public:
  EntityLayer() : Layer("EntityLayer") {}
  ~EntityLayer() override = default;

  void onAttach() override;
  void onDetach() override;
  void onUpdate() override;
  void onEvent(Event &e) override;

private:
  entt::registry registry;
};

class Entity {

private:
  entt::entity m_internalEntity;
};
} // namespace CitronECS
