struct VertexInput {
    @location(0) in_vertex_position: vec3f,
    @location(1) in_vertex_normal: vec3f,
    @location(2) in_vertex_color: vec3f,
    @location(3) in_vertex_uv: vec2f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
};

@vertex
fn vs_main(input: VertexInput) -> VertexOutput {
    return VertexOutput(
        vec4f(input.in_vertex_position, 1.0),
    );
}

@group(0) @binding(0) var colorTexture: texture_2d<f32>;
@group(0) @binding(1) var normalTexture: texture_2d<f32>;

@fragment
fn fs_main(input: VertexOutput) -> @location(0) vec4f {
    let color = textureLoad(colorTexture, vec2i(input.position.xy), 0).rgb;
    let normal = textureLoad(normalTexture, vec2i(input.position.xy), 0).rgb;
    return vec4f(normal, 1.0);
}
