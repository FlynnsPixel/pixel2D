#ifndef FONT_H
#define FONT_H

#include "PXL_FontUtils.h"
#include "PXL_TextureSheet.h"

typedef struct FT_FaceRec_* FT_Face;

#define MIN(v, v2) v < v2 ? v : v2
#define MAX(v, v2) v > v2 ? v : v2

class PXL_Font {

	public:
		/**
		\*brief: loads the font
		\*param [path]: the path and file name for the font to load
		**/
		PXL_Font(std::string path, int c_max_font_size = 72);
		/**
		\*brief: font deconstructor
		**/
		~PXL_Font();

		PXL_TextureSheet* glyph_sheet; /**> Texture sheet containing all glyphs in this font **/
		std::string name;
		int num_glyphs;
		int width;
		int height;

		const PXL_Rect* get_glyph_rects() {
			return &glyph_rects[0];
		}

		int get_glyph_index(int char_code);
		int get_max_font_size() { return max_font_size; }
		int get_max_char_width() { return max_char_width; }
		int get_max_char_height() { return max_char_height; }

		/**
		\*brief: frees all data from the font
		**/
		void free();

	private:
		//font info
		bool font_loaded;
		FT_Face f;
		PXL_Rect* glyph_rects;
		int max_font_size;
		int max_char_width = 0;
		int max_char_height = 0;
};

/**
\*brief: loads and creates a font from the specified path
\*param [path]: the path and file name for the font to load
**/
extern PXL_Font* PXL_create_font(std::string path, int c_max_font_size = 72);

#endif