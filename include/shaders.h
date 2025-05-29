#pragma once 

const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout(location=0) in vec3 aPos;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    out float lightIntensity;
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        vec3 worldPos = (model * vec4(aPos, 1.0)).xyz;
        vec3 normal = normalize(aPos);
        vec3 dirToCenter = normalize(-worldPos);
        lightIntensity = max(dot(normal, dirToCenter), 0.15);})glsl";
    
    const char* fragmentShaderSource = R"glsl(
    #version 330 core
    in float lightIntensity;
    out vec4 FragColor;
    uniform vec4 objectColor;
    uniform bool isGrid; // Add this uniform
    uniform bool GLOW;
    void main() {
        if (isGrid) {
            // If it's the grid, use the original color without lighting
            FragColor = objectColor;
        } else if(GLOW){
            FragColor = vec4(objectColor.rgb * 100000, objectColor.a);
        }else {
            // If it's an object, apply the lighting effect
            float fade = smoothstep(0.0, 10.0, lightIntensity*10);
            FragColor = vec4(objectColor.rgb * fade, objectColor.a);
        }})glsl";