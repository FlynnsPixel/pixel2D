#ifndef _PREBUILT_SHADERS_H
#define _PREBUILT_SHADERS_H

#include <string>
#include <iostream>

#define GLSL(src) "#version 130\nprecision mediump float;\n" #src

namespace pxl { namespace graphics {

    /**
	    -------------- default vertex shader -------------
	    author: Richman Stewart
	    simple vertex shader that sets the position
	    to the specified matrix and position while
	    passing the vertex colour and tex coords
	    to the fragment shader
    **/
    extern const char* basic_vertex_shader_str = GLSL(
	    //[START_VERTEX]

	    attribute mediump vec3 a_position;
	    attribute mediump vec2 a_tex_coord;
        attribute lowp vec4 a_colour;

        uniform mat4 matrix;

	    varying vec4 v_colour;
        varying vec2 tex_coord;
        flat out float z_depth;

	    void main() {
		    v_colour = a_colour;
            tex_coord = a_tex_coord;
            z_depth = a_position.z;
            gl_Position = matrix * vec4(a_position.x, a_position.y, 0, 1);
	    }

	    //[END_VERTEX]
    );

    /**
	    ------------ default fragment shader ------------
	    author: Richman Stewart
	    simple default fragment shader that multiplies
	    the vertex colour with a texel
    **/
    extern const char* default_shader_str = GLSL(
        //[START_FRAGMENT]

        uniform sampler2D t0;

	    varying vec4 v_colour;
        varying vec2 tex_coord;
        flat in float z_depth;

	    void main() {
		    gl_FragColor = v_colour * texture2D(t0, tex_coord);
            gl_FragDepth = z_depth;
	    }

	    //[END_FRAGMENT]
    );

    /**

	    ------------ one pass bloom shader ------------

	    author: Richman Stewart

	    applies a gaussian blur horizontally and vertically
	    and applies it on top of the original texture

	    ------------------ use ------------------------

	    outline_size - defines the spread x and y
	    outline_intensity - bloom intensity

    **/
    extern const char* bloom_shader_str = GLSL(
        //[START_FRAGMENT]

        varying vec4 v_colour;
        varying vec2 tex_coord;

	    uniform sampler2D t0;
	    uniform float outline_spread;
	    uniform float outline_intensity;

	    void main() {
		    ivec2 size = textureSize(t0, 0);

		    float uv_x = tex_coord.x * size.x;
		    float uv_y = tex_coord.y * size.y;

		    vec4 sum = vec4(0.0);
		    for (int n = 0; n < 9; ++n) {
			    uv_y = (tex_coord.y * size.y) + (outline_spread * float(n - 4));
			    vec4 h_sum = vec4(0.0);
			    h_sum += texelFetch(t0, ivec2(uv_x - (4.0 * outline_spread), uv_y), 0);
			    h_sum += texelFetch(t0, ivec2(uv_x - (3.0 * outline_spread), uv_y), 0);
			    h_sum += texelFetch(t0, ivec2(uv_x - (2.0 * outline_spread), uv_y), 0);
			    h_sum += texelFetch(t0, ivec2(uv_x - outline_spread, uv_y), 0);
			    h_sum += texelFetch(t0, ivec2(uv_x, uv_y), 0);
			    h_sum += texelFetch(t0, ivec2(uv_x + outline_spread, uv_y), 0);
			    h_sum += texelFetch(t0, ivec2(uv_x + (2.0 * outline_spread), uv_y), 0);
			    h_sum += texelFetch(t0, ivec2(uv_x + (3.0 * outline_spread), uv_y), 0);
			    h_sum += texelFetch(t0, ivec2(uv_x + (4.0 * outline_spread), uv_y), 0);
			    sum += h_sum / 9.0;
		    }

            gl_FragColor = v_colour * (texture(t0, tex_coord) + ((sum / 9.0) * outline_intensity));
	    }

	    //[END_FRAGMENT]
    );

    /**

	    ------------ one pass blur shader ------------

		    author: Richman Stewart

		    applies a gaussian blur horizontally and vertically

	    ------------------ use ------------------------

		    blur_size - blur spread amount

    **/
    extern const char* blur_shader_str = GLSL(
        //[START_FRAGMENT]

        varying vec4 v_colour;
        varying vec2 tex_coord;

	    uniform sampler2D t0;
	    uniform vec2 blur_size;

	    void main() {
		    ivec2 size = textureSize(t0, 0);

		    float uv_x = tex_coord.x * size.x;
		    float uv_y = tex_coord.y * size.y;

		    vec4 sum = vec4(0.0);

		    sum += texelFetch(t0, ivec2(uv_x - (4.0 * blur_size.x), uv_y - (4.0 * blur_size.y)), 0);
		    sum += texelFetch(t0, ivec2(uv_x - (3.0 * blur_size.x), uv_y - (3.0 * blur_size.y)), 0);
		    sum += texelFetch(t0, ivec2(uv_x - (2.0 * blur_size.x), uv_y - (2.0 * blur_size.y)), 0);
		    sum += texelFetch(t0, ivec2(uv_x - blur_size.x, uv_y - blur_size.y), 0);
		    sum += texelFetch(t0, ivec2(uv_x, uv_y), 0);
		    sum += texelFetch(t0, ivec2(uv_x + blur_size.x, uv_y + blur_size.y), 0);
		    sum += texelFetch(t0, ivec2(uv_x + (2.0 * blur_size.x), uv_y + (2.0 * blur_size.y)), 0);
		    sum += texelFetch(t0, ivec2(uv_x + (3.0 * blur_size.x), uv_y + (3.0 * blur_size.y)), 0);
		    sum += texelFetch(t0, ivec2(uv_x + (4.0 * blur_size.x), uv_y + (4.0 * blur_size.y)), 0);

            gl_FragColor = v_colour * (sum / 9.0);
	    }

	    //[END_FRAGMENT]
    );

    /**
	
	    ------------ grayscale fragment shader ------------

	    author: Richman Stewart

	    changes each colour to grayscale using NTSC
	    conversion weights

    **/
    extern const char* grayscale_shader_str = GLSL(
        //[START_FRAGMENT]

        varying vec4 v_colour;
        varying vec2 tex_coord;

	    uniform sampler2D t0;

	    void main() {
            gl_FragColor = texture(t0, tex_coord);
            float g = dot(gl_FragColor.rgb, vec3(0.299, 0.587, 0.114));
            gl_FragColor = vec4(g, g, g, v_colour.a * gl_FragColor.a);
	    }

	    //[END_FRAGMENT]
    );

    /**
	
	    ------------ one pass glow shader ------------

	    author: Richman Stewart

	    applies a gaussian glow horizontally and vertically
	    behind the original texture

	    ------------------ use ------------------------

	    outline_size - defines the spread x and y
	    outline_colour - the colour of the glow
	    outline_intensity - glow intensity

    **/
    extern const char* glow_shader_str = GLSL(
        //[START_FRAGMENT]

        varying vec4 v_colour;
        varying vec2 tex_coord;

	    uniform sampler2D t0;
	    uniform float outline_size;
	    uniform vec3 outline_colour;
	    uniform float outline_intensity;
	    uniform float outline_threshold;

	    void main() {
            gl_FragColor = texture(t0, tex_coord);
            if (gl_FragColor.a <= outline_threshold) {
			    ivec2 size = textureSize(t0, 0);
	
			    float uv_x = tex_coord.x * size.x;
			    float uv_y = tex_coord.y * size.y;

			    float sum = 0.0;
			    for (int n = 0; n < 9; ++n) {
				    uv_y = (tex_coord.y * size.y) + (outline_size * float(n - 4.5));
				    float h_sum = 0.0;
				    h_sum += texelFetch(t0, ivec2(uv_x - (4.0 * outline_size), uv_y), 0).a;
				    h_sum += texelFetch(t0, ivec2(uv_x - (3.0 * outline_size), uv_y), 0).a;
				    h_sum += texelFetch(t0, ivec2(uv_x - (2.0 * outline_size), uv_y), 0).a;
				    h_sum += texelFetch(t0, ivec2(uv_x - outline_size, uv_y), 0).a;
				    h_sum += texelFetch(t0, ivec2(uv_x, uv_y), 0).a;
				    h_sum += texelFetch(t0, ivec2(uv_x + outline_size, uv_y), 0).a;
				    h_sum += texelFetch(t0, ivec2(uv_x + (2.0 * outline_size), uv_y), 0).a;
				    h_sum += texelFetch(t0, ivec2(uv_x + (3.0 * outline_size), uv_y), 0).a;
				    h_sum += texelFetch(t0, ivec2(uv_x + (4.0 * outline_size), uv_y), 0).a;
				    sum += h_sum / 9.0;
			    }

                gl_FragColor = vec4(outline_colour, (sum / 9.0) * outline_intensity);
		    }
	    }

	    //[END_FRAGMENT]
    );

    /**
	
	    ------------ one pass outline shader ------------

	    author: Richman Stewart

	    applies a gaussian blur horizontally and vertically
	    behind the original texture and makes it black

	    ------------------ use ------------------------

	    outline_thickness - outline spread amount
	    outline_colour - colour of the outline

    **/
    extern const char* outline_shader_str = GLSL(
        //[START_FRAGMENT]

        varying vec4 v_colour;
        varying vec2 tex_coords;

	    uniform sampler2D t0;
	    uniform float outline_thickness = 1;
	    uniform vec4 outline_colour = vec4(0, 0, 0, 1);
	    uniform float outline_threshold = .5;

	    void main() {
            gl_FragColor = texture(t0, tex_coords);

            if (gl_FragColor.a <= outline_threshold) {
			    ivec2 size = textureSize(t0, 0);

			    float uv_x = tex_coords.x * size.x;
			    float uv_y = tex_coords.y * size.y;

			    float sum = 0.0;
			    for (int n = 0; n < 9; ++n) {
				    uv_y = (tex_coords.y * size.y) + (outline_thickness * float(n - 4.5));
				    float h_sum = 0.0;
				    h_sum += texelFetch(t0, ivec2(uv_x - (4.0 * outline_thickness), uv_y), 0).a;
				    h_sum += texelFetch(t0, ivec2(uv_x - (3.0 * outline_thickness), uv_y), 0).a;
				    h_sum += texelFetch(t0, ivec2(uv_x - (2.0 * outline_thickness), uv_y), 0).a;
				    h_sum += texelFetch(t0, ivec2(uv_x - outline_thickness, uv_y), 0).a;
				    h_sum += texelFetch(t0, ivec2(uv_x, uv_y), 0).a;
				    h_sum += texelFetch(t0, ivec2(uv_x + outline_thickness, uv_y), 0).a;
				    h_sum += texelFetch(t0, ivec2(uv_x + (2.0 * outline_thickness), uv_y), 0).a;
				    h_sum += texelFetch(t0, ivec2(uv_x + (3.0 * outline_thickness), uv_y), 0).a;
				    h_sum += texelFetch(t0, ivec2(uv_x + (4.0 * outline_thickness), uv_y), 0).a;
				    sum += h_sum / 9.0;
			    }

			    if (sum / 9.0 >= 0.0001) {
                    gl_FragColor = outline_colour;
			    }
		    }
	    }

	    //[END_FRAGMENT]
    );

    /**

	    ------------ point light fragment shader ------------

	    author: Richman Stewart

	    repeats the original texture x, y amount of times

	    ---------------------- use -----------------------------

	    repeat - the amount of times to repeat
	    the texture horizontally and vertically

    **/
    extern const char* point_light_shader_str = GLSL(
        //[START_FRAGMENT]

        varying vec4 v_colour;
        varying vec2 tex_coord;

	    uniform sampler2D t0;
	    uniform float points[504];
	    uniform int points_length;
	    uniform float max_alpha = 1;
	    const int point_size = 7;

	    void main() {
            gl_FragColor = vec4(0, 0, 0, 0);

		    vec2 pos = tex_coord * textureSize(t0, 0);
		    float size;
		    float intensity;
		    for (int n = 0; n < points_length; n += point_size) {
			    size = points[n + 2];
			    intensity = points[n + 3];
			    float dist = sqrt(pow(pos.x - points[n], 2) + pow(pos.y - points[n + 1], 2));
			    if (dist <= size) {
				    float a = intensity - (dist / (size / intensity));
                    gl_FragColor.r += a * points[n + 4];
                    gl_FragColor.g += a * points[n + 5];
                    gl_FragColor.b += a * points[n + 6];
                    gl_FragColor.a += a * gl_FragColor.rgb;
			    }
		    }
            gl_FragColor.a = clamp(gl_FragColor.a, 0, max_alpha);
	    }

	    //[END_FRAGMENT]
    );

    /**

	    ------------ texture repeat fragment shader ------------

	    author: Richman Stewart

	    repeats the original texture x, y amount of times

	    ---------------------- use -----------------------------

	    repeat - the amount of times to repeat
	    the texture horizontally and vertically

    **/
    extern const char* repeat_shader_str = GLSL(
        //[START_FRAGMENT]

        varying vec4 v_colour;
        varying vec2 tex_coord;

	    uniform sampler2D t0;
	    uniform vec2 repeat = vec2(2.0, 2.0);

	    void main() {
		    ivec2 size = textureSize(t0, 0);
            gl_FragColor = v_colour * texelFetch(t0, ivec2(mod(tex_coord.xy * repeat.xy * size.xy, size.xy)), 0);
	    }

	    //[END_FRAGMENT]
    );

    /**

	    ------------ texture repeat fragment shader ------------

	    author: Richman Stewart

	    repeats the original texture x, y amount of times

	    ---------------------- use -----------------------------

	    repeat - the amount of times to repeat
	    the texture horizontally and vertically

    **/
    extern const char* text_shader_str = GLSL(
	    //[START_FRAGMENT]

	    uniform sampler2D t0;
	
	    varying vec4 v_colour;
        varying vec2 tex_coord;
        flat in float z_depth;

	    void main() {
            gl_FragColor = vec4(v_colour.rgb, texture2D(t0, tex_coord).a);
            gl_FragDepth = z_depth;
	    }

	    //[END_FRAGMENT]
    );
}};

#endif