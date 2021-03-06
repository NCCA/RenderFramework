#version 400 core

uniform sampler2D pointTex;
uniform sampler2D normalTex;
uniform sampler2D colourTex;
uniform vec3 lightPos;
uniform float lightI=0.005;

uniform mat4 V;
layout (location=0)out vec4 lightPassTex;

uniform vec3 cam;
uniform vec2 wh;

void main()
{
  vec2 st;
  st.s = gl_FragCoord.x / wh.x;
  st.t = gl_FragCoord.y / wh.y;
  vec4 point=texture(pointTex,st);
  vec4 normal=texture(normalTex,st);
  vec4 colour=texture(colourTex,st);
//  if (point.z > -0.0001) {
//     discard;
//   }
  vec3 lightDir = lightPos - point.xyz ;

  normal = normalize(normal);
  lightDir = normalize(lightDir);
  lightPassTex=vec4(0.0);
  vec3 eyeDir = normalize(cam-point.xyz);
  vec3 vHalfVector = normalize(lightDir.xyz+eyeDir);
  lightPassTex.rgb = max(dot(normal.xyz,lightDir),0) * colour.rgb +
                   pow(max(dot(normal.xyz,vHalfVector),0.0), 100) ;

  lightPassTex=lightPassTex*lightI;
}
