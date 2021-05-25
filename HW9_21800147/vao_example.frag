#version 430

in vec4 fColor;
out vec4 FragColor;
layout(location=4) uniform int mode;

void main()
{
    switch(mode)
    {
    case 0:
        FragColor = fColor;
        break;
    case 1:
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        break;
    case 2:
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        break;
    case 3:
        FragColor = vec4(1.0, 0.0, 0.0, 0.0);
        break;
    case 4:
        FragColor = vec4(0.0, 1.0, 0.0, 0.0);
        break;
    case 5:
        FragColor = vec4(0.0, 0.0, 1.0, 0.0);
        break;

    }
}
