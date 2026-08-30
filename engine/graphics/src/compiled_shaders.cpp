#include "compiled_shaders.hpp"

std::string CompiledShaders::debug_grid = R"(struct VertexInput {
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
    @location(3) vertexPosition: vec3f,
    @location(4) cameraHeight: f32,
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

@group(0) @binding(0) var<uniform> frameUniforms: FrameUniforms;
@group(1) @binding(0) var<uniform> drawUniforms: DrawUniforms;

fn floorMod(x: vec2f, y: vec2f) -> vec2f {
    return x - y * floor(x / y);
}

fn extractCameraWorldPos(view: mat4x4f) -> vec3f {
    let c0 = view[0].xyz;
    let c1 = view[1].xyz;
    let c2 = view[2].xyz;
    let t = view[3].xyz;
    return vec3f(-dot(c0, t), -dot(c1, t), -dot(c2, t));
}

@vertex
fn vs_main(
    input: VertexInput
) -> VertexOutput {
    let cameraWorldPos = extractCameraWorldPos(drawUniforms.view);
    var cameraOffset: vec2f = vec2f(cameraWorldPos.x, cameraWorldPos.z);

    let modifiedVertexPosition: vec3f = (input.in_vertex_position * SCALE_FACTOR) + vec3f(cameraOffset.x, 0.0, cameraOffset.y);

    let worldPosition: vec4f = drawUniforms.projection * drawUniforms.view * vec4f(modifiedVertexPosition, 1.0);
    let modifiedUV: vec2f = modifiedVertexPosition.xz + vec2f(SCALE_FACTOR);

    return VertexOutput(
        worldPosition,
        input.in_vertex_normal,
        input.in_vertex_color,
        modifiedUV,
        input.in_vertex_position,
        cameraWorldPos.y,
    );
}

struct ForwardOutput {
    @location(0) color: vec4f,
};

const SCALE_FACTOR = 1000.0f;
const GRID_CELL_SIZE = 10.0f;
const GRID_CELL_SECONDARY_SIZE = GRID_CELL_SIZE * 0.2f;

const CELL_LINE_THICKNESS = 0.002f;
const CELL_SECONDARY_LINE_THICKNESS = CELL_LINE_THICKNESS * 0.8f;
const GRID_COLOR = vec3f(0.357f);
const GRID_X_COLOR = vec3f(1, 0.373, 0.373);
const GRID_Z_COLOR = vec3f(0.337, 0.596, 0.988);

@group(1) @binding(1) var depthTexture: texture_depth_2d;

@fragment
fn fs_main(input: VertexOutput) -> ForwardOutput {
    let depth = textureLoad(depthTexture, vec2i(input.position.xy), 0);
    if depth < input.position.z {
        return ForwardOutput(vec4f(0.0, 0.0, 0.0, 0.0));
    }

    let modUV: vec2f = input.uv % GRID_CELL_SIZE;
    let secondaryModUV: vec2f = input.uv % GRID_CELL_SECONDARY_SIZE;

    var color = vec4f(0.0, 0.0, 0.0, 0.0);

    let d = fwidth(input.uv);
    let adjustedCellLineThickness: vec2f = (vec2f(CELL_LINE_THICKNESS) + d);
    let adjustedCellSecondaryLineThickness: vec2f = (vec2f(CELL_SECONDARY_LINE_THICKNESS) + d);

    if modUV.x < adjustedCellLineThickness.x || modUV.x > GRID_CELL_SIZE - adjustedCellLineThickness.x {
        color = vec4f(GRID_COLOR, 1.0);
    } else if modUV.y < adjustedCellLineThickness.y || modUV.y > GRID_CELL_SIZE - adjustedCellLineThickness.y {
        color = vec4f(GRID_COLOR, 1.0);
    }

    if secondaryModUV.x < adjustedCellSecondaryLineThickness.x || secondaryModUV.x > GRID_CELL_SIZE - adjustedCellSecondaryLineThickness.x {
        color = vec4f(GRID_COLOR, 0.75);
    } else if secondaryModUV.y < adjustedCellSecondaryLineThickness.y || secondaryModUV.y > GRID_CELL_SIZE - adjustedCellSecondaryLineThickness.y {
        color = vec4f(GRID_COLOR, 0.75);
    }

    color.a -= distance(input.vertexPosition, vec3f(0.0, input.cameraHeight / SCALE_FACTOR, 0.0)) * 2.0f;
    color.a = clamp(color.a, 0.0, 1.0);
    return ForwardOutput(
        color,
    );
}
)";

std::string CompiledShaders::debug_wireframe = R"(struct VertexInput {
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

@group(0) @binding(0) var<uniform> frameUniforms: FrameUniforms;
@group(1) @binding(0) var<uniform> drawUniforms: DrawUniforms;

@vertex
fn vs_main(
    input: VertexInput
) -> VertexOutput {
    var worldPosition: vec4f = drawUniforms.projection * drawUniforms.view * vec4f(input.in_vertex_position, 1.0);

    return VertexOutput(
        worldPosition,
        input.in_vertex_normal,
        input.in_vertex_color,
        input.in_vertex_uv,
    );
}

struct ForwardOutput {
    @location(0) color: vec4f,
};

@fragment
fn fs_main(input: VertexOutput) -> ForwardOutput {
    return ForwardOutput(vec4f(input.color, 1.0));
}
)";

std::string CompiledShaders::lighting_pass = R"(struct VertexInput {
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
    sunLight: vec4f,
    sunLightColor: vec4f,
    ambientLight: vec4f,
};

@group(0) @binding(0) var colorTexture: texture_2d<f32>;
@group(0) @binding(1) var normalTexture: texture_2d<f32>;
@group(0) @binding(2) var<uniform> frameUniforms: FrameUniforms;

@fragment
fn fs_main(input: VertexOutput) -> @location(0) vec4f {
    let colorTextureValue = textureLoad(colorTexture, vec2i(input.position.xy), 0);
    let color = colorTextureValue.rgb * input.color.rgb * frameUniforms.sunLightColor.rgb;

    let normal = textureLoad(normalTexture, vec2i(input.position.xy), 0).rgb;

    let lit = dot(normal, normalize(frameUniforms.sunLight.xyz));

    return vec4f((color * lit) + frameUniforms.ambientLight.rgb, colorTextureValue.a);
}
)";

