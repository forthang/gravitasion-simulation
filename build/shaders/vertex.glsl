#version 330 core

layout (location = 0) in vec2 aPos;

uniform vec2 uPosition;
uniform float uScale;

uniform mat4 uView;
uniform mat4 uProj;

void main() {
    vec2 scaledPos = aPos * uScale;
    vec2 worldPos = uPosition + scaledPos;

    vec4 modelPos = vec4(worldPos, 0.0, 1.0);
    gl_Position = uProj * uView * modelPos;
}
