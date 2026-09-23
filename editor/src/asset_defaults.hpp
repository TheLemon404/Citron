#pragma once

struct AssetDefaults {
  public:
	static inline const char *getDefaultShader() {
		return R"(
struct VertexInput {
	@location(0) in_vertex_position: vec3f,
	@location(1) in_vertex_normal: vec3f,
	@location(2) in_vertex_color: vec3f,
	@location(3) in_vertex_uv: vec2f,
};

struct VertexOutput {
	@builtin(position) position: vec4f,
	@location(0) normal: vec3f,
	@location(1) color: vec3f,
	@location(2) uv: vec2f,
};

struct FrameUniforms {
	sunLight: vec4f,
	sunLightColor: vec4f,
	ambientLight: vec4f,
};

struct DrawUniforms {
	view: mat4x4f,
	projection: mat4x4f,
};

struct ModelUniforms {
	transform: mat4x4f,
};

@group(0) @binding(0) var<uniform> frameUniforms: FrameUniforms;
@group(1) @binding(0) var<uniform> drawUniforms: DrawUniforms;
@group(2) @binding(0) var<uniform> modelUniforms: ModelUniforms;

@vertex
fn vs_main(
	input: VertexInput
) -> VertexOutput {
	var worldPosition: vec4f = drawUniforms.projection * drawUniforms.view * modelUniforms.transform * vec4f(input.in_vertex_position, 1.0);

	return VertexOutput(
	    worldPosition,
	    input.in_vertex_normal,
	    input.in_vertex_color,
	    input.in_vertex_uv,
	);
}

struct MaterialUniforms {
	diffuse: vec3f,
};

struct GBufferOutput {
	@location(0) color: vec4f,
	@location(1) normal: vec4f,
	@location(2) ids: vec4f,
};

@group(3) @binding(0) var<uniform> materialUniforms: MaterialUniforms;

@fragment
fn fs_main(input: VertexOutput) -> GBufferOutput {
	return GBufferOutput(
	    vec4f(input.color * materialUniforms.diffuse, 1.0),
	    vec4f(input.normal, 1.0f),
	    vec4f(1.0, 0.0, 0.0, 1.0),
	);
}
		)";
	}
};
