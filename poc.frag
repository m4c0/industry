#version 450

layout(constant_id = 0) const int grid_size = 16;

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
layout(location = 1) out vec4 selection;

vec2 random(ivec2 p) {
  int n = p.x + p.y*11111;
  // Hugo Elias hash
  n = (n << 13) ^ n;
  n = (n * (n * n * 15731 + 789221) + 1376312589) >> 16;
  return vec2(cos(float(n)), sin(float(n)));
}
// https://www.shadertoy.com/view/XdXGW8
float noise(vec2 p) {
  ivec2 i = ivec2(floor(p));
  vec2 f = fract(p);
  vec2 u = f * f * (3.0 - 2.0 * f);
  float x0y0 = dot(random(i + ivec2(0, 0)), f - vec2(0, 0));
  float x1y0 = dot(random(i + ivec2(1, 0)), f - vec2(1, 0));
  float x0y1 = dot(random(i + ivec2(0, 1)), f - vec2(0, 1));
  float x1y1 = dot(random(i + ivec2(1, 1)), f - vec2(1, 1));
  float y0 = mix(x0y0, x1y0, u.x);
  float y1 = mix(x0y1, x1y1, u.x);
  return mix(y0, y1, u.y);
}

vec3 s_background(vec2 p) {
  return vec3(0.1, 0.11, 0.12);
}

vec3 blank(vec2 p) { return vec3(0.01); }

vec3 clicker(vec2 p) {
  return s_background(p);
}

vec3 sprite(vec2 p, uint id) {
  switch (id) {
    case  0: return blank(p);
    case  1: return clicker(p);
    default: return vec3(1, 0, 1);
  }
}

vec3 merge_mix(vec3 a, vec3 b, float f) {
  return mix(a, b, smoothstep(0.95, 1.0, f) * 0.5);
}

void main() {
  float n = scale;

  vec2 p = (frag_pos + 1.0) * (n / 2.0) + displ;
  float x = smoothstep(0.99, 1.0, cos(p.x * 6.28));
  float y = smoothstep(0.99, 1.0, cos(p.y * 6.28));
  float d = max(x, y);

  vec4 sel = vec4(0);
  vec3 c = vec3(0);
  ivec2 id = ivec2(floor(p));
  if (min(id.x, id.y) >= 0 && id.x < grid_size && id.y < grid_size) {
    ivec2 nei = ivec2(round(fract(p)) * 2 - 1);

    uint spr    = sprites[ id.y          * grid_size + id.x];
    uint spr_x  = sprites[ id.y          * grid_size + id.x + nei.x];
    uint spr_y  = sprites[(id.y + nei.y) * grid_size + id.x];
    uint spr_xy = sprites[(id.y + nei.y) * grid_size + id.x + nei.x];

    vec2 f = abs(fract(p) * 2.0 - 1.0);
    vec2 pp = fract(p);

    vec3 c0  = sprite(pp, spr);
    vec3 cx  = merge_mix(c0, sprite(pp, spr_x),  f.x);
    vec3 cy  = merge_mix(c0, sprite(pp, spr_y),  f.y);
    vec3 cxy = merge_mix(c0, sprite(pp, spr_xy), f.x * f.y);
    c = (spr == spr_y && spr == spr_x)
      ? cxy
      : (spr == spr_x)
      ? cy
      : (spr == spr_y)
      ? cx
      : (f.x > f.y ? cx : cy);
    sel.xy = id / 256.0;
    sel.w = 1;
  }

  colour = vec4(c, 1);
  selection = sel;
}
