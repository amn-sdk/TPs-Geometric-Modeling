#version 120

varying vec4 myvertex;
varying vec3 mynormal;

uniform mat4 myview_matrix;
uniform mat3 mynormal_matrix;
uniform int type;
uniform vec4 kd;

void main(void)
{
    vec3 eyepos = vec3(0.0, 0.0, 0.0);

    vec4 _mypos = myview_matrix * myvertex;
    vec3 mypos = _mypos.xyz / _mypos.w;

    vec3 lightpos = vec3(0.0, 0.0, 4.0);
    vec3 lightcolor = vec3(1.0, 1.0, 1.0);

    vec3 L = normalize(lightpos - mypos);
    vec3 V = normalize(eyepos - mypos);
    vec3 N = normalize(mynormal_matrix * mynormal);

    float dotLN = max(dot(L, N), 0.0);
    vec3 color = kd.rgb * lightcolor * dotLN;

    gl_FragColor = vec4(color, 1.0);
}
