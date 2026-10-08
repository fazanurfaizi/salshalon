#version 300 es

layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec2 a_TexCoord;
layout (location = 2) in vec2 a_Color;

uniform mat4 u_Projection;

out vec2 v_TexCoord;
out vec4 v_Color;

void main() {
  v_TexCoord = a_TexCoord;
  v_Color = a_Color;
  gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
}
