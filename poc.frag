#version 450

layout(push_constant) uniform u_ {
  layout(offset = 8)
  vec2 displ;
  float scale;
};

layout(location = 0) in  vec2 frag_pos;
layout(location = 0) out vec4 colour;

void main() {
  float n = scale;

  vec2 p = (frag_pos + 1.0) * (n / 2.0) + displ;
  float x = smoothstep(0.99, 1.0, cos(p.x * 6.28));
  float y = smoothstep(0.99, 1.0, cos(p.y * 6.28));
  float d = max(x, y);

  vec2 id = floor(p);

  vec3 c = vec3(d, id / n);
  colour = vec4(c, 1);
}
