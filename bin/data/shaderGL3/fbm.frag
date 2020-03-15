// fragment shader

#version 150

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;

in vec4 position;
in vec3 normal;
in vec4 color;
in vec2 texcoord;

out vec4 outputColor;

// custom

uniform vec2 noiseOffset, planeSize;
uniform int fbmOctaves, fbmWarp;

uniform float seed, contrast,
              gradientNoise, gradientAmnt, gradientScale, 
              simplexNoise, simplexAmnt, simplexScale,
              voronoiNoise, voronoiAmnt, voronoiScale,
              fbmHurst, fbmFrequency, fbmRotation, warpRotation;

#define PI 3.14159265359

/**************************************************************/

vec2 ghash (vec2 x) {
  const vec2 k = vec2(0.3183099, 0.3678794);
  x = x * k + k.yx + seed;
  return fract(16.0 * k * fract(x.x * x.y * (x.x + x.y)));
}  

// GRADIENT NOISE
float gnoise2D(vec2 st) {
  st *= gradientScale;

  vec2 ipos = floor(st);
  vec2 fpos = fract(st);

  float a = dot(ghash(ipos + vec2(0., 0.)),fpos - vec2(0., 0.));
  float b = dot(ghash(ipos + vec2(1., 0.)), fpos - vec2(1., 0.));
  float c = dot(ghash(ipos + vec2(0., 1.)), fpos - vec2(0., 1.));
  float d = dot(ghash(ipos + vec2(1., 1.)), fpos - vec2(1., 1.));

  vec2 u = smoothstep(0., 1., fpos);

  return mix(
    mix(a, b, u.x),
    mix(c, d, u.x),
    u.y
  ) * gradientAmnt * gradientNoise;
}

// SIMPLEX NOISE
float snoise2D (vec2 st) {
  st *= simplexScale;

  const float K1 = 0.366025404;                 // (sqrt(3)-1)/2;
  const float K2 = 0.211324865;                 // (3-sqrt(3))/6;

	vec2  i = floor( st + (st.x + st.y) * K1 );   // integer part
  vec2  a = st - i + (i.x + i.y) * K2;          // fract part

  float m = step(a.y, a.x);                   // which triangle we are in
  vec2 o = vec2(m,1.0-m);                     // origin selector
  
  vec2  b = a - o + K2;                       // distance between a and the origin
  vec2  c = a - 1.0 + 2.0 * K2;               // distance between a and other vertex

  vec3  h = max( 0.5 - vec3(                  // kernel
    dot(a,a),                               // same as magnitude squared
    dot(b,b),                               // same as magnitude squared
    dot(c,c)                                // same as magnitude squared
  ), 0.0 );

  vec3  n = h*h*h*h*vec3(
    dot(a, ghash( i + 0.)),                 // gradient computation
    dot(b, ghash( i + o)),                  // gradient computation
    dot(c, ghash( i + 1.0))                 // gradient computation
  );

  return dot( n, vec3(70.) ) * simplexAmnt * simplexNoise;              // scaling up the noise
}

// VORONOI NOISE
float voronoi2D(in vec2 st) {
  st *= voronoiScale;

	float minDist = 100.; // we impose a large minDist

  vec2 id = floor(st);  // the number of the cell we are in
  vec2 gv = fract(st) - .5; // adjusted space inside the cell, from 0 to 1 to -.5 to .5

  for (float y = -1.; y <= 1.; y++) {     // this double loop searches for the neighbouring points
    for (float x = -1.; x <= 1.; x++) {   // in the 9 cell around our ID cell
      vec2 offset = vec2(x, y);           // which cell we are  in

      vec2 n = ghash(vec2(id + offset));  // random value for that specific cell
      vec2 p = offset + sin(n) * .5;      //  the random value is transformed in a position vector inside the                                    //   cell we are in

      float d = length(gv-p);  // square distance between p and our center point gv
      d *= d;

      if (d < minDist) {
        minDist = d;    // minDist is updated
      }
    }
  }

  return minDist * voronoiAmnt * voronoiNoise;
}

// FRACTIONAL BROWNIAN MOTION
float fbm (in vec2 st) {
  float value = 0.0;
  float amplitude = fbmHurst;
  float frequency = 0.;
    
  mat2 fbmRotMat = mat2(
    cos(fbmRotation * PI), sin(fbmRotation * PI),
    -sin(fbmRotation * PI), cos(fbmRotation * PI)
  );

  for (int i = 0; i < fbmOctaves; i++) {
    value += amplitude * (snoise2D(st) + gnoise2D(st) + voronoi2D(st));
    value = tanh(value);
    st *= fbmFrequency;
    st = fbmRotMat * st;
    amplitude *= fbmHurst;
  }
  return value;
}


void main()
{
  vec2 st = gl_FragCoord.xy / planeSize;

  // DOMAIN WARPING
  float k_1 = 0.;
  float k_2 = 0.;
  float n_1 = 0.;
  float n_2 = 0.;

  mat2 warpRotMat = mat2(
    cos(warpRotation * PI), sin(warpRotation * PI),
    -sin(warpRotation * PI), cos(warpRotation * PI)
  );

  for (int i = 0; i < fbmWarp; i++) {
    n_1 = fbm(st - noiseOffset + k_1);
    k_1 = n_1;
    n_2 = fbm( st + noiseOffset + k_2);
    k_2 = n_2;
    st = warpRotMat * st;
  }

  float n_1_norm = tanh(contrast * n_1);
  float n_2_norm = tanh(contrast * n_2);

  vec3 n = vec3(n_1_norm, n_2_norm, 0.);

  outputColor = vec4(n, 1.);
}
