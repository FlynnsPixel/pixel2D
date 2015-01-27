//[START_VERTEX]
#version 140

/**
-------------- texture coords vertex shader -------------

    author: Richman Stewart

    simple vertex shader that sets the position
    to the specified matrix and position while
    passing the vertex colour and tex coords
    to the fragment shader

**/

in vec2 a_position;
in vec2 a_tex_coord;
in vec4 a_colour;

uniform mat4 matrix;

out vec4 v_colour;
out vec2 tex_coord;

void main() {
   v_colour = a_colour;
   tex_coord = a_tex_coord;
   gl_Position = matrix * vec4(a_position, 0, 1);
}

//[END_VERTEX]

//[START_FRAGMENT]
#version 140

/**
------------ texture repeat fragment shader ------------

    author: Richman Stewart

    repeats the original texture x, y amount of times

---------------------- use -----------------------------

    repeat - the amount of times to repeat
    the texture horizontally and vertically

**/

in vec4 v_colour;
in vec2 tex_coord;
out vec4 pixel;

uniform sampler2D t0;
uniform float points[504];
uniform int points_length;
const int point_size = 7;

void main() {
	pixel = vec4(1, 1, 1, 0);
	
  	vec2 pos = tex_coord * textureSize(t0, 0);
  	float size;
  	float intensity;
  	if (pos.x >= 1020) {
  		pixel = vec4(0, 0, 0, 1);
  	}
  	for (int n = 0; n < points_length; n += point_size) {
  		size = points[n + 2];
  		intensity = points[n + 3];
  		float dist = sqrt(pow(pos.x - points[n], 2) + pow(pos.y - points[n + 1], 2));
  		if (dist <= size) {
  			float a = intensity - (dist / (size / intensity));
  			pixel.r -= a * points[n + 4];
  			pixel.g -= a * points[n + 5];
  			pixel.b -= a * points[n + 6];
  			pixel.a += a;
  		}
  	}
}

//[END_FRAGMENT]