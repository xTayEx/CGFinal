#ifndef __SKYBOX_H__
#define __SKYBOX_H__
#include <string>
#include <vector>
#include <iostream>

#include <glad/glad.h>

#include <Kapsule/VertexArray.h>
#include <Kapsule/Buffers.h>
#include <Kapsule/Utility.h>

//#include <stb_image.h>

namespace Kapsule {
	using namespace std;
	class Skybox {
	public:
		Kapsule::VertexArray skyboxVao;
		Kapsule::VertexBuffer skyboxVbo;
		GLuint skyboxTexID;
		void loadCubemap(vector<string> faces)
		{
			glGenTextures(1, &skyboxTexID);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexID);

			int width, height, nrChannels;
			size_t siz = faces.size();
			for (size_t i = 0; i < siz; i++) {
				unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
				if (data) {
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
								 0,
								 GL_RGB,
								 width,
								 height,
								 0,
								 GL_RGB,
								 GL_UNSIGNED_BYTE,
								 data);
					stbi_image_free(data);
				} else {
					cerr << "[ERROR] Failed to load texture at path " << faces[i] << "\n";
					stbi_image_free(data);
				}
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}

		void setupSkybox(float Size)
		{
			float skyboxVertices[] = {
				// positions          
				-Size,  Size, -Size,
				-Size, -Size, -Size,
				 Size, -Size, -Size,
				 Size, -Size, -Size,
				 Size,  Size, -Size,
				-Size,  Size, -Size,

				-Size, -Size,  Size,
				-Size, -Size, -Size,
				-Size,  Size, -Size,
				-Size,  Size, -Size,
				-Size,  Size,  Size,
				-Size, -Size,  Size,

				 Size, -Size, -Size,
				 Size, -Size,  Size,
				 Size,  Size,  Size,
				 Size,  Size,  Size,
				 Size,  Size, -Size,
				 Size, -Size, -Size,

				-Size, -Size,  Size,
				-Size,  Size,  Size,
				 Size,  Size,  Size,
				 Size,  Size,  Size,
				 Size, -Size,  Size,
				-Size, -Size,  Size,

				-Size,  Size, -Size,
				 Size,  Size, -Size,
				 Size,  Size,  Size,
				 Size,  Size,  Size,
				-Size,  Size,  Size,
				-Size,  Size, -Size,

				-Size, -Size, -Size,
				-Size, -Size,  Size,
				 Size, -Size, -Size,
				 Size, -Size, -Size,
				-Size, -Size,  Size,
				 Size, -Size,  Size
			};
			skyboxVao.create();
			skyboxVao.bind();
			skyboxVbo.create();
			skyboxVbo.bind();
			skyboxVbo.loadData(skyboxVertices, sizeof(skyboxVertices), Kapsule::USAGE::STATIC_DRAW);
			skyboxVao.setAttributePointer(skyboxVbo, 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		}

		void renderSkybox(Shader& skyboxShader, glm::mat4 view, glm::mat4 projection)
		{
			glDepthFunc(GL_LEQUAL);
			skyboxShader.use();
			skyboxShader.setMat4("view", view);
			skyboxShader.setMat4("projection", projection);
			skyboxVao.bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, this->skyboxTexID);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			skyboxVao.unbind();
			glDepthFunc(GL_LESS);
		}
	};
}

#endif // !__SKYBOX_H__
