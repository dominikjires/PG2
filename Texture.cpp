// NE�AHAT!!!
#include <opencv2\opencv.hpp>
#include "texture.hpp"

GLuint textureInit(const char* filepath)
{
	cv::Mat image = cv::imread(filepath, cv::IMREAD_UNCHANGED);
	if (image.empty())
	{
		std::cerr << "no texture: " << filepath << std::endl;
		exit(1);
	}
	GLuint texture = tex_gen(image);

	return texture;
}

GLuint tex_gen(cv::Mat& image)
{
	if (image.empty()) {
		throw std::exception("Image empty?\n");
	}

	// select proper format for image source and destination
	GLint  img_internalformat;
	GLenum img_format;

	if (!GLEW_ARB_texture_compression) {
		throw std::exception("Compressed textures not supported??\n");
	}

	switch (image.channels()) {
	case 1:
		img_internalformat = GL_COMPRESSED_RED;
		img_format = GL_RED;
		break;
	case 3:
		img_internalformat = GL_COMPRESSED_RGB;
		img_format = GL_BGR;
		break;
	case 4:
		// if channels() == RGBA, we have a Alpha channel, aka semitransparent texture
		img_internalformat = GL_COMPRESSED_RGBA;
		img_format = GL_BGRA;
		break;
	default:
		throw std::exception("Unsupported # of channels\n");
	}
	bool transparent = (image.channels() == 4);

	// generate texture name
	GLuint texture;
	glGenTextures(1, &texture);

	// bind texture as active
	glBindTexture(GL_TEXTURE_2D, texture);

	// get list of supported formats
	GLint num_compressed_format;
	glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &num_compressed_format);
	if (num_compressed_format > 0)
	{
		GLint compressed, internalformat, compressed_size;

		std::cout << "COMPRESSION supported, tot. available formats: " << num_compressed_format << std::endl;

		// try to load compressed texture
		//glHint(GL_TEXTURE_COMPRESSION_HINT, GL_FASTEST);
		glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);

		glTexImage2D(GL_TEXTURE_2D, 0, img_internalformat, image.cols, image.rows, 0, img_format, GL_UNSIGNED_BYTE, image.data);

		// Is it now really compressed? Did we succeed?
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed);
		// if the compression has been successful
		if (compressed == GL_TRUE)
		{
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalformat);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressed_size);
			std::cout << "ORIGINAL: " << image.total() * image.elemSize() << " COMPRESSED: " << compressed_size << " INTERNAL FORMAT: " << internalformat << std::endl;
		}
	}
	else {
		throw std::exception("Compression supported, but no formats available?\n");
	}

	//
	// Set texture properties
	//

	// TILED texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	// Texture filters - pick one

	// nearest neighbor - ugly & fast 
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// bilinear - nicer & slower
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// MIPMAP filtering + automatic MIPMAP generation - nicest, needs more memory. Notice: MIPMAP is only for image minifying.
	glGenerateMipmap(GL_TEXTURE_2D);  //Generate mipmaps now.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // bilinear magnifying
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //trilinear minifying

	return texture;
}
