#pragma once
#ifndef __MODEL_H__
#define __MODEL_H__

#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <Kapsule/VertexArray.h>
#include <Kapsule/Buffers.h>
#include <Kapsule/Shader.h>
#include <Kapsule/Mesh.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

namespace Kapsule {
	using namespace std;
	unsigned int TextureFromFile(const char* path, const string& directory)
	{
		string filename = string(path);
		filename = directory + '/' + filename;

		unsigned int textureID;
		glGenTextures(1, &textureID);
		//DEBUG

		int width, height, nrComponents;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data) {
			GLenum format;
			if (nrComponents == 1) {
				format = GL_RED;
			} else if (nrComponents == 3) {
				format = GL_RGB;
			} else if (nrComponents == 4) {
				format = GL_RGBA;
			}

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		} else {
			cerr << "[ERROR] Failed to load texture at path " << path << "\n";
			stbi_image_free(data);
		}
		return textureID;
	}


	class Model {
	public:
		Model() {}
		Model(char* path);
		void draw(const Shader& shader);
		void drawShadow(const Shader& shadowShader, 
						glm::vec3 lightPos,
                        glm::mat4 viewMatrix,
						glm::mat4 projMatrix,
                        glm::mat4 shadowModelMatrix);
	private:
		vector<Mesh> meshes;
		string directory;
		void loadModel(string const& path);
		void processNode(aiNode* node, const aiScene* scene);
		Mesh processMesh(aiMesh* mesh, const aiScene* scene);
		vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
	};

	Model::Model(char* path)
	{
		loadModel(path);
	}

	void Model::draw(const Shader& shader)
	{
		for (size_t i = 0; i < meshes.size(); i++) {
			meshes[i].draw(shader);
		}
	}
	
	void Model::drawShadow(const Shader& shadowShader,
						   glm::vec3 lightPos,
		                   glm::mat4 viewMatrix,
						   glm::mat4 projMatrix,
		                   glm::mat4 modelMatrix)
	{
		for (size_t i = 0; i < meshes.size(); i++) {
			meshes[i].drawShadow(shadowShader, lightPos, viewMatrix, projMatrix, modelMatrix);
		}
	}

	vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
			//DEBUG
			aiString str;
			mat->GetTexture(type, i ,&str);
			bool skip = false;


			Texture texture;
			texture.textureID = TextureFromFile(str.C_Str(), this->directory);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
		}
		return textures;
	}

	Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
	{
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;
		
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;
			glm::vec3 vector;

			// position
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.position = vector;

			// normal
			if (mesh->HasNormals()) {
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.normal = vector;
			}

			// texture coord
			if (mesh->mTextureCoords[0]) {
				glm::vec2 vec;

				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.texCoords = vec;
			} else {
				vertex.texCoords = glm::vec2(0.0f, 0.0f);
			}
			vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);
			}
		}

		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse_texture");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "specular_texture");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		return Mesh(vertices, indices, textures);
	}

	void Model::processNode(aiNode* node, const aiScene* scene)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);
		}
		return;
	}

	void Model::loadModel(string const& path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			cerr << "[ERROR] Assimp error: " << importer.GetErrorString() << "\n";
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));
		processNode(scene->mRootNode, scene);
	}
}
#endif // !__MODEL_H__
