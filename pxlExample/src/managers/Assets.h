#ifndef ASSETS_H
#define ASSETS_H

#include <SDL.h>
#include <vector>
#include <PXL_Texture.h>

using namespace std;

class Universe;

class Assets {

	public:
		vector<PXL_Texture*> textures;

		PXL_Texture* cat;

		void initiate();

		/**
		loads an image in the assets folder and returns a texture from it
		**/
		PXL_Texture* load_texture(char* path, bool dispose_surface = true);

		/**
		free memory from all textures loaded
		**/
		void free_textures();

	private:
		Universe* universe;
};

#endif