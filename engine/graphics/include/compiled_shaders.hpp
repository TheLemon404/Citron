#pragma once

#include <string>
std::string lighting_pass = R"(struct VertexInput {
    @location(0) in_vertex_position: vec3f,
    @location(1) in_vertex_normal: vec3f,
    @location(2) in_vertex_color: vec3f,
    @location(3) in_vertex_uv: vec2f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec3f,
    @location(1) normal: vec3f,
    @location(2) uv: vec2f,
};

@vertex
fn vs_main(input: VertexInput) -> VertexOutput {
    return VertexOutput(
        vec4f(input.in_vertex_position, 1.0),
        input.in_vertex_color,
        input.in_vertex_normal,
        input.in_vertex_uv,
    );
}

struct FrameUniforms {
    viewProjection: mat4x4f,
    sunLight: vec4f,
    sunLightColor: vec4f,
    ambientLight: vec4f,
};

@group(0) @binding(0) var colorTexture: texture_2d<f32>;
@group(0) @binding(1) var normalTexture: texture_2d<f32>;
@group(0) @binding(2) var<uniform> frameUniforms: FrameUniforms;

@fragment
fn fs_main(input: VertexOutput) -> @location(0) vec4f {
    let color = textureLoad(colorTexture, vec2i(input.position.xy), 0).rgb * input.color.rgb * frameUniforms.sunLightColor.rgb;

    let normal = textureLoad(normalTexture, vec2i(input.position.xy), 0).rgb;

    let lit = dot(normal, normalize(frameUniforms.sunLight.xyz));

    return vec4f((color * lit) + frameUniforms.ambientLight.rgb, 1.0);
}
)";

