#version 450

layout(push_constant) uniform u_ {
  vec2 aspect;
};

layout(location = 0) in  vec2 pos;
layout(location = 0) out vec2 frag_pos;

void main() {
  vec2 v = pos * 2.0 - 1.0;
  frag_pos = v * aspect;
  gl_Position = vec4(v, 0, 1);
}
