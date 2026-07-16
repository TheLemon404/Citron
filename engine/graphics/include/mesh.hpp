#pragma once

#include <memory>
#include <shader.hpp>

namespace CitronGraphics {

class Mesh {
	Mesh(std::shared_ptr<Shader> vertexShader,
		 std::shared_ptr<Shader> fragmentShader, int vertexCount,
		 int indexCount)
		: vertexCount(vertexCount), indexCount(indexCount) {}

  public:
	const int vertexCount;
	const int indexCount;
};

} // namespace CitronGraphics
