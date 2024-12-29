#version 450

layout(push_constant) uniform u_ {
  layout(offset = 8)
  vec2 displ;
  float scale;
};

layout(binding = 0) buffer b_ {
  uint sprites[];
};

layout(location = 0) in  vec2 frag_pos;
layout(location = 0) out vec4 colour;

const ivec2 grid_size = ivec2(16);

void main() {
  float n = scale;

  vec2 p = (frag_pos + 1.0) * (n / 2.0) + displ;
  float x = smoothstep(0.99, 1.0, cos(p.x * 6.28));
  float y = smoothstep(0.99, 1.0, cos(p.y * 6.28));
  float d = max(x, y);

  vec3 c = vec3(0);
  ivec2 id = ivec2(floor(p));
  if (min(id.x, id.y) >= 0 && id.x < grid_size.x && id.y < grid_size.y) {
    uint spr = sprites[id.y * grid_size.x + id.x];
    c = vec3(spr);
  }

  colour = vec4(c, 1);
}
