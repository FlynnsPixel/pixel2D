#include "PXL_ShaderUtils.h"
#include <fstream>
#include "PXL_Batch.h"
#include "PXL_System.h"
#include "PXL_Config.h"

//defines
const char* start_v_header = "//[START_VERTEX]";
const char* end_v_header = "//[END_VERTEX]";
const char* start_f_header = "//[START_FRAGMENT]";
const char* end_f_header = "//[END_FRAGMENT]";

PXL_ShaderProgram* PXL_default_shader;
PXL_ShaderProgram* PXL_bloom_shader;
PXL_ShaderProgram* PXL_repeat_shader;
PXL_ShaderProgram* PXL_grayscale_shader;
PXL_ShaderProgram* PXL_blur_shader;
PXL_ShaderProgram* PXL_outline_shader;
PXL_ShaderProgram* PXL_outer_glow_shader;
PXL_ShaderProgram* PXL_text_shader;

void PXL_shader_init() {
	//setup premade pxl glsl shaders
	PXL_default_shader = PXL_load_glsl_shader(PXL_append_char(PXL_PREBUILT_SHADER_PATH, "default.glsl"));
	PXL_bloom_shader = PXL_load_glsl_shader(PXL_append_char(PXL_PREBUILT_SHADER_PATH, "bloom.glsl"));
	PXL_repeat_shader = PXL_load_glsl_shader(PXL_append_char(PXL_PREBUILT_SHADER_PATH, "repeat.glsl"));
	PXL_grayscale_shader = PXL_load_glsl_shader(PXL_append_char(PXL_PREBUILT_SHADER_PATH, "grayscale.glsl"));
	PXL_blur_shader = PXL_load_glsl_shader(PXL_append_char(PXL_PREBUILT_SHADER_PATH, "blur.glsl"));
	PXL_outline_shader = PXL_load_glsl_shader(PXL_append_char(PXL_PREBUILT_SHADER_PATH, "outline.glsl"));
	PXL_outer_glow_shader = PXL_load_glsl_shader(PXL_append_char(PXL_PREBUILT_SHADER_PATH, "outer_glow.glsl"));
	PXL_text_shader = PXL_load_glsl_shader(PXL_append_char(PXL_PREBUILT_SHADER_PATH, "text.glsl"));
}

void PXL_set_default_shader(PXL_Batch* batch) {
	batch->set_shader(PXL_default_shader);
}

void PXL_set_bloom_shader(PXL_Batch* batch, float spread, float intensity) {
	batch->set_shader(PXL_bloom_shader);
	glUniform1f(glGetUniformLocation(PXL_bloom_shader->get_program_id(), "outline_spread"), spread);
	glUniform1f(glGetUniformLocation(PXL_bloom_shader->get_program_id(), "outline_intensity"), intensity);
}

void PXL_set_repeat_shader(PXL_Batch* batch, float repeat_x, float repeat_y) {
	batch->set_shader(PXL_repeat_shader);
	glUniform2f(glGetUniformLocation(PXL_repeat_shader->get_program_id(), "repeat"), repeat_x, repeat_y);
}

void PXL_set_grayscale_shader(PXL_Batch* batch) {
	batch->set_shader(PXL_grayscale_shader);
}

void PXL_set_blur_shader(PXL_Batch* batch, float spread_x, float spread_y) {
	batch->set_shader(PXL_blur_shader);
	glUniform2f(glGetUniformLocation(PXL_blur_shader->get_program_id(), "outline_size"), spread_x, spread_y);
}

void PXL_set_outline_shader(PXL_Batch* batch, float thickness, float r, float g, float b, float a, float threshold) {
	batch->set_shader(PXL_outline_shader);
	glUniform1f(glGetUniformLocation(PXL_outline_shader->get_program_id(), "outline_thickness"), thickness);
	glUniform4f(glGetUniformLocation(PXL_outline_shader->get_program_id(), "outline_colour"), r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
	glUniform1f(glGetUniformLocation(PXL_outline_shader->get_program_id(), "outline_threshold"), threshold);
}

void PXL_set_outer_glow_shader(PXL_Batch* batch, float size, float r, float g, float b, float intensity, float threshold) {
	batch->set_shader(PXL_outer_glow_shader);
	glUniform1f(glGetUniformLocation(PXL_outline_shader->get_program_id(), "outline_size"), size);
	glUniform3f(glGetUniformLocation(PXL_outline_shader->get_program_id(), "outline_colour"), r / 255.0f, g / 255.0f, b / 255.0f);
	glUniform1f(glGetUniformLocation(PXL_outline_shader->get_program_id(), "outline_threshold"), threshold);
	glUniform1f(glGetUniformLocation(PXL_outline_shader->get_program_id(), "outline_intensity"), intensity);
}

void PXL_set_text_shader(PXL_Batch* batch, float r, float g, float b, float a) {
	batch->set_shader(PXL_text_shader);
	glUniform3f(glGetUniformLocation(PXL_text_shader->get_program_id(), "text_colour"), r / 255.0f, g / 255.0f, b / 255.0f);
}

PXL_ShaderProgram* PXL_load_shader(std::string vertex_file, std::string fragment_file) {
	return new PXL_ShaderProgram(PXL_load_file(vertex_file), PXL_load_file(fragment_file), vertex_file, fragment_file);
}

PXL_ShaderProgram* PXL_load_glsl_shader(std::string glsl_file) {
	std::string s = PXL_load_file(glsl_file);

	int start_v = s.find(start_v_header);
	int end_v = s.find(end_v_header);
	int start_f = s.find(start_f_header);
	int end_f = s.find(end_f_header);

	if (start_v != -1 && end_v != -1 && start_f != -1 && end_f != -1) {
		std::string vertex_file = s.substr(start_v + strlen(start_v_header) + 1, end_v - (start_v + strlen(start_v_header) + 1));
		std::string fragment_file = s.substr(start_f + strlen(start_f_header) + 1, end_f - (start_f + strlen(start_f_header) + 1));
		return new PXL_ShaderProgram(vertex_file, fragment_file, glsl_file + " - vertex", glsl_file + " - fragment");
	}else {
		PXL_show_exception("Headers not found when loading (" + glsl_file + "). Custom PXL_glsl shaders use " + 
							start_v_header + " at the beginning of a vertex shader and " + end_v_header + " at the end. Fragment " + 
							"shaders use " + start_f_header + " and " + end_f_header, true, false);
	}
	return NULL;
}

std::string PXL_load_file(std::string file_name) {
	std::ifstream file(file_name, std::ifstream::in);
	if (file) {
		file.ignore(std::numeric_limits<std::streamsize>::max());
		std::streamsize size = file.gcount();
		file.seekg(0, std::ifstream::beg);

		if (size >= 0) {
			char* buffer = new char[size];
			file.read(buffer, size);

			file.close();
			if (buffer) {
				buffer[size] = '\0';
				return buffer;
			}else {
				PXL_show_exception("(" + file_name + ") could not be read successfully", true, false);
				delete[] buffer;
			}
		}else {
			PXL_show_exception("(" + file_name + ") does not contain any content when read", true, false);
		}
	}else {
		PXL_show_exception("Couldn't load shader file (" + file_name + "). It may not exist", true, false);
	}
	return "";
}