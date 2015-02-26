#include "PXL_Batch.h"
#include <iostream>
#include <algorithm>
#include "PXL_Window.h"
#include "system/PXL_Exception.h"

PXL_Batch::PXL_Batch(PXL_BatchSize max_vertices) {
	batch_created = false;
	create_batch(max_vertices);
}

void PXL_Batch::create_batch(PXL_BatchSize max_vertices) {
	max_vertices_amount = max_vertices;
	max_quads_amount = max_vertices_amount / 4;

	free();

	{
		//create the vbo
		glGenBuffers(1, &vertex_buffer_id);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
		glBufferData(GL_ARRAY_BUFFER, max_vertices_amount * sizeof(PXL_VertexPoint), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, NULL);

		vertex_batch_cache = new PXL_VertexBatch[max_quads_amount];
		vertex_data_cache = new PXL_VertexPoint[max_vertices_amount];

		vertices = new VertexContainer[max_quads_amount];
		for (int n = 0; n < max_quads_amount; ++n) {
			vertices[n].batches = std::vector<PXL_VertexBatch>();
			vertices[n].data = std::vector<PXL_VertexPoint>();
		}

		batch_created = true;
	}

	clear_all();

	//set perspective matrix to window coordinates and translate to 0,0 top left
	view_mat.identity();
	perspective_mat.identity();

	perspective_mat.scale(1.0f / PXL_center_window_x, -1.0f / PXL_center_window_y);
	perspective_mat.translate(-1.0f, 1.0f);

	//enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);

	//binds vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);

	//enable vertex attrib pointers when rendering
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	//set vertex shader attrib pointers
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(PXL_VertexPoint), (void*)offsetof(PXL_VertexPoint, pos));
	glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(PXL_VertexPoint), (void*)offsetof(PXL_VertexPoint, uv));
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(PXL_VertexPoint), (void*)offsetof(PXL_VertexPoint, colour));
}

void PXL_Batch::render_all() {
	if (num_added != 0) {
		//if a framebuffer is specified, bind to it, if not bind to the default framebuffer
		if (target_frame_buffer != NULL) {
			target_frame_buffer->bind(PXL_GL_FRAMEBUFFER_WRITE);
		}else {
			glBindFramebuffer(PXL_GL_FRAMEBUFFER_WRITE, 0);
		}

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);

		draw_vbo();

		glBindFramebuffer(PXL_GL_FRAMEBUFFER_WRITE, 0);
	}
	clear_all();
}

void PXL_Batch::clear_all() {
	total_vertices = 0;
	num_added = 0;
	min_vertex_index = UINT_MAX;
	max_vertex_index = 0;
	total_vertices = 0;
	min_vertices_count = 0;
}

void PXL_Batch::use_shader(PXL_ShaderProgram* shader) {
	if (shader == NULL) { shader = PXL_default_shader; }

	if (current_shader != shader) {
		current_shader = shader;

		//use specified program id
		glUseProgram(current_shader->get_program_id());

		//set matrix uniform in the vertex shader for the program
		view_mat.identity();
		glUniformMatrix4fv(current_shader->get_matrix_loc(), 1, true, (view_mat * perspective_mat).get_mat());
	}
}

void PXL_Batch::use_blend_mode(PXL_BlendMode blend_mode) {
	if (current_blend_mode != blend_mode) {
		current_blend_mode = blend_mode;
		if (current_blend_mode == PXL_BLEND) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}else if (current_blend_mode == PXL_NO_BLEND) {
			//glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE);
		}
	}
}

void PXL_Batch::add(const PXL_Texture& texture, PXL_Rect* rect, PXL_Rect* src_rect, float rotation, PXL_Vec2* origin, 
					PXL_Flip flip, int z_depth, PXL_Colour colour, PXL_ShaderProgram* shader, PXL_BlendMode blend_mode) {
	if (verify_texture_add(texture, rect)) {
		z_depth += (max_quads_amount - 1) / 2;
		if (z_depth < 0) {
			PXL_show_exception("Z depth value cannot be below half of the max quad batch size (" + std::to_string(-max_quads_amount / 2) + ")", 
				PXL_ERROR_BATCH_ADD_FAILED);
			z_depth = 0;
		}else if (z_depth >= max_quads_amount) {
			PXL_show_exception("Z depth value cannot be greater than half ot the max quad batch size (" + std::to_string(max_quads_amount / 2) + ")", 
				PXL_ERROR_BATCH_ADD_FAILED);
			z_depth = max_quads_amount - 1;
		}
		
		VertexContainer& c = vertices[z_depth];

		if (c.batch_index >= c.batches.size()) {
			c.batches.push_back(vertex_batch_cache[total_vertices / 4]);
			++c.batch_size;
		}
		PXL_VertexBatch& v_batch = c.batches[c.batch_index];

		v_batch.num_vertices = 4;
		v_batch.texture_id = texture.get_id();
		v_batch.shader = shader;
		v_batch.z_depth = z_depth;
		v_batch.blend_mode = blend_mode;

		min_vertex_index = PXL_min(min_vertex_index, (PXL_uint)z_depth);
		max_vertex_index = PXL_max(max_vertex_index, (PXL_uint)z_depth);

		++num_added;

		if (c.data_index >= c.data.size()) {
			c.data.push_back(vertex_data_cache[total_vertices]);
			c.data.push_back(vertex_data_cache[total_vertices + 1]);
			c.data.push_back(vertex_data_cache[total_vertices + 2]);
			c.data.push_back(vertex_data_cache[total_vertices + 3]);
			c.data_size += 4;
		}
		PXL_VertexPoint* v = &c.data[c.data_index];

		total_vertices += 4;
		++c.batch_index;
		c.data_index += 4;

		/**
		==================================================================================
									Set vertex positions
		==================================================================================
		**/
		//set vertex pos, uvs and colours
		//set origin
		float origin_x = 0; float origin_y = 0;
		if (origin != NULL) { origin_x = origin->x; origin_y = origin->y; }

		bool modified = false;
		if (v_batch.rect.x != rect->x || v_batch.rect.y != rect->y ||
			v_batch.rect.w != rect->w || v_batch.rect.h != rect->h) {
			modified = true;

			memcpy(&v_batch.rect, rect, sizeof(PXL_Rect));
		}
		if (v_batch.rotation != rotation || v_batch.flip != flip ||
			v_batch.origin.x != origin_x || v_batch.origin.y != origin_y) {
			modified = true;

			v_batch.rotation = rotation;
			v_batch.flip = flip;
			v_batch.origin.x = origin_x; v_batch.origin.y = origin_y;
		}
		if (modified) {
			//get positions from rect
			int x = rect->x; int y = rect->y;

			//set scale
			float scale_x = rect->w / texture.get_width(); float scale_y = rect->h / texture.get_height();
			if (flip == PXL_FLIP_HORIZONTAL) {
				scale_x = -scale_x;
				x += rect->w;
				origin_x -= rect->w;
			}else if (flip == PXL_FLIP_VERTICAL) {
				scale_y = -scale_y;
				y += rect->h;
				origin_y -= rect->h;
			}
			int scaled_width = texture.get_width() * scale_x;
			int scaled_height = texture.get_height() * scale_y;

			//apply rotation
			if (rotation != 0) {
				//set rotation to degrees rather than radians
				rotation = rotation / PXL_radian;
				float c = PXL_fast_cos(rotation); float s = PXL_fast_sin(rotation);

				x += origin_x; y += origin_y;
				scaled_width -= origin_x; scaled_height -= origin_y;

				//set vertex position including scale and rotation
				v[0].pos.x = x + ((c * -origin_x) - (s * -origin_y));
				v[0].pos.y = y + ((s * -origin_x) + (c * -origin_y));

				v[1].pos.x = x + ((c * scaled_width) - (s * -origin_y));
				v[1].pos.y = y + ((s * scaled_width) + (c * -origin_y));

				v[2].pos.x = x + ((c * scaled_width) - (s * scaled_height));
				v[2].pos.y = y + ((s * scaled_width) + (c * scaled_height));

				v[3].pos.x = x + ((c * -origin_x) - (s * scaled_height));
				v[3].pos.y = y + ((s * -origin_x) + (c * scaled_height));

				v_batch.rotation = rotation;
			}else {
				//set vertex position including scale
				v[0].pos.x = x;											v[0].pos.y = y;
				v[1].pos.x = x + scaled_width;							v[1].pos.y = y;
				v[2].pos.x = x + scaled_width;							v[2].pos.y = y + scaled_height;
				v[3].pos.x = x;											v[3].pos.y = y + scaled_height;
			}

			c.updated = true;
		}

		/**
		==================================================================================
									Set UV vertex coords
		==================================================================================
		**/
		//attempt to optimise by not setting uv values if they have the same value in the vertex batch as the new values
		modified = false;
		if (src_rect == NULL) {
			if (v_batch.src_rect.x != 0					  || v_batch.src_rect.y != 0 ||
				v_batch.src_rect.w != texture.get_width() || v_batch.src_rect.h != texture.get_height()) {
				modified = true;

				v_batch.src_rect.x = 0; v_batch.src_rect.y = 0;
				v_batch.src_rect.w = texture.get_width(); v_batch.src_rect.h = texture.get_height();
			}
		}else {
			if (v_batch.src_rect.x != src_rect->x || v_batch.src_rect.y != src_rect->y ||
				v_batch.src_rect.w != src_rect->w || v_batch.src_rect.h != src_rect->h) {
				modified = true;

				memcpy(&v_batch.src_rect, src_rect, sizeof(PXL_Rect));
			}
		}

		if (modified) {
			//default un-normalised uv coords
			PXL_ushort uv_x = 0; PXL_ushort uv_y = 0; PXL_ushort uv_w = USHRT_MAX; PXL_ushort uv_h = USHRT_MAX;
			if (src_rect != NULL) {
				//calculate uv x, y, w, h by the src rect
				uv_x = (src_rect->x / texture.get_width()) * USHRT_MAX; uv_y = (src_rect->y / texture.get_height()) * USHRT_MAX;
				uv_w = (src_rect->w / texture.get_width()) * USHRT_MAX; uv_h = (src_rect->h / texture.get_height()) * USHRT_MAX;
			}

			//set uv coordinates
			v[0].uv.x = uv_x;										v[0].uv.y = uv_y;
			v[1].uv.x = uv_x + uv_w;								v[1].uv.y = uv_y;
			v[2].uv.x = uv_x + uv_w;								v[2].uv.y = uv_y + uv_h;
			v[3].uv.x = uv_x;										v[3].uv.y = uv_y + uv_h;

			c.updated = true;
		}

		/**
		==================================================================================
									Set vertex colours
		==================================================================================
		**/
		int i_r = colour.r * 255; int i_g = colour.g * 255; int i_b = colour.b * 255; int i_a = colour.a * 255;

		if (v_batch.colour.r != i_r || v_batch.colour.g != i_g || v_batch.colour.b != i_b || v_batch.colour.a != i_a) {
			//set vertex colours
			for (int n = 0; n < 4; ++n) {
				v[n].colour.r = i_r;
				v[n].colour.g = i_g;
				v[n].colour.b = i_b;
				v[n].colour.a = i_a;
			}
			v_batch.colour.r = i_r;
			v_batch.colour.g = i_g;
			v_batch.colour.b = i_b;
			v_batch.colour.a = i_a;

			c.updated = true;
		}
	}
}

bool PXL_Batch::verify_texture_add(const PXL_Texture& texture, PXL_Rect* rect) {
	if (texture.texture_created) {
		if (rect->x + rect->w > 0 && rect->y + rect->h > 0 && rect->x < PXL_window_width && rect->y < PXL_window_height) {
			if (num_added + 1 >= max_quads_amount) {
				PXL_show_exception("Hit max batch size at " + std::to_string(max_quads_amount) + " max sprites/quads", PXL_ERROR_BATCH_ADD_FAILED);
				return false;
			}

			return true;
		}
	}
	return false;
}

void PXL_Batch::set_target(PXL_FrameBuffer* f) {
	//sets the target frame buffer to be used for rendering
	target_frame_buffer = f;
}

void PXL_Batch::draw_vbo() {
	//if there are no textures to draw or no vertex data then return
	if (num_added == 0) { return; }

	//binds vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);

	//loops through each texture and draws the vertex data with that texture id
	vbo_offset = 0;
	int size = 0;
	bool changed = false;

	//render notes
	//vertex batches vectors
	//vertex data has it's own vectors like vertex batches
	//upload buffer data for each z depth vector
	//when uploading input vector as buffer data
	//cache buffer data by not uploading if it hasn't changed
	//maybe can render non-transparent images with depth buffer first and then disable it and render transparent images

	GLuint prev_id = vertices[min_vertex_index].batches[0].texture_id;
	glBindTexture(GL_TEXTURE_2D, prev_id);
	PXL_BlendMode prev_blend_mode = vertices[min_vertex_index].batches[0].blend_mode;
	use_blend_mode(prev_blend_mode);
	PXL_ShaderProgram* prev_shader = vertices[min_vertex_index].batches[0].shader;
	use_shader(prev_shader);

	std::cout << "---------\n";
	for (int i = min_vertex_index; i <= max_vertex_index; ++i) {
		size_t v_size = vertices[i].batch_index;
		if (v_size == 0) continue;

		size = 0;

		VertexContainer& c = vertices[i];

		if (c.updated || c.data_index == vbo_offset) {
			if (vbo_offset + c.data_index >= max_vertices_amount) vbo_offset = 0;
			glBufferSubData(GL_ARRAY_BUFFER, vbo_offset * sizeof(PXL_VertexPoint), c.data_index * sizeof(PXL_VertexPoint), &c.data[0]);
		}else {
			std::cout << "did not update batch " << i << "\n";
		}

		PXL_VertexBatch* v_batch;
		for (int n = 0; n < v_size + 1; ++n) {
			GLuint last_id = prev_id;

			if (n >= v_size) {
				changed = true;
			}else {
				v_batch = &c.batches[n];
			}

			glBindTexture(GL_TEXTURE_2D, prev_id);
			if (v_batch->texture_id != prev_id) {
				prev_id = v_batch->texture_id;
				changed = true;
			}

			use_shader(prev_shader);
			if (v_batch->shader != prev_shader) {
				prev_shader = v_batch->shader;
				changed = true;
			}

			use_blend_mode(prev_blend_mode);
			if (v_batch->blend_mode != prev_blend_mode) {
				prev_blend_mode = v_batch->blend_mode;
				changed = true;
			}

			if (changed) {
				//draw vertex data from vertex data in buffer
				glDrawArrays(GL_QUADS, vbo_offset, size);

				vbo_offset += size;
				size = 0;

				changed = false;
			}
			size += v_batch->num_vertices;
		}
		//c.batches.clear();
		//c.data.clear();
		c.batch_index = 0;
		c.data_index = 0;
		c.updated = false;
	}
}

void PXL_Batch::free() {
	if (batch_created) {
		glDeleteBuffers(1, &vertex_buffer_id);

		for (int n = 0; n < max_quads_amount; ++n) {
			std::vector<PXL_VertexBatch>().swap(vertices[n].batches);
		}
		for (int n = 0; n < max_quads_amount; ++n) {
			std::vector<PXL_VertexPoint>().swap(vertices[n].data);
		}
		delete[] vertices;

		batch_created = false;
	}
}

PXL_Batch::~PXL_Batch() {
	free();
}