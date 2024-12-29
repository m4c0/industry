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
    ivec2 nei = ivec2(round(fract(p)) * 2 - 1);

    uint spr = sprites[id.y * grid_size.x + id.x];
    uint spr_x = sprites[id.y * grid_size.x + id.x + nei.x];
    uint spr_y = sprites[(id.y + nei.y) * grid_size.x + id.x];
    uint spr_xy = sprites[(id.y + nei.y) * grid_size.x + id.x + nei.x];

    vec2 f = abs(fract(p) * 2.0 - 1.0);

    vec3 c0 = vec3(spr);
    vec3 cx = mix(c0, vec3(spr_x), smoothstep(0.9, 1.0, f.x) * 0.5);
    vec3 cy = mix(c0, vec3(spr_y), smoothstep(0.9, 1.0, f.y) * 0.5);
    vec3 cxy = mix(c0, vec3(spr_xy), smoothstep(0.9, 1.0, f.x * f.y) * 0.5);
    c = (spr == spr_y && spr == spr_x)
      ? cxy
      : (spr == spr_x)
      ? cy
      : (spr == spr_y)
      ? cx
      : (f.x > f.y ? cx : cy);
  }

  colour = vec4(c, 1);
}
