#version 460

in vec2 TexCoord0;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float alpha = 0.2;

out vec4 FragColor;

uniform sampler2D gSampler;

void main()
{
    // FragColor = texture2D(gSampler, TexCoord0.xy);
    FragColor = mix(texture(texture1, TexCoord0), texture(texture2, TexCoord0), alpha);
}