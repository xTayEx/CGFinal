#pragma once
#ifndef __SHADER_H__
#define __SHADER_H__

#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <glm/glm.hpp>

#include "Utility.h"

namespace Kapsule {

	using namespace std;
	class Shader {
	public:
		unsigned int ShaderID;
		Shader(const char* vertexShaderPath, const char* fragmentShaderPath, bool showInfoLog=true)
		{
			this->showInfoLog = showInfoLog;
			string vertexShaderSource, fragmentShaderSource;

			vertexShaderSource = readFile(vertexShaderPath);
			fragmentShaderSource = readFile(fragmentShaderPath);
			
			const char* vertexShaderSourceCstr = vertexShaderSource.c_str();
			const char* fragmentShaderSourceCstr = fragmentShaderSource.c_str();

			unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertexShader, 1, &vertexShaderSourceCstr, NULL);
			glCompileShader(vertexShader);
			checkCompileErrors(vertexShader, "vertexShader");

			unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragmentShader, 1, &fragmentShaderSourceCstr, NULL);
			glCompileShader(fragmentShader);
			checkCompileErrors(fragmentShader, "fragmentShader");

			this->ShaderID = glCreateProgram();
			glAttachShader(ShaderID, vertexShader);
			glAttachShader(ShaderID, fragmentShader);
			glLinkProgram(ShaderID);
			checkCompileErrors(ShaderID, "program");

			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);
		}
		
		// set uniform
		bool getShowInfoLog()
		{
			return showInfoLog;
		}
		
		void use() const
		{
			glUseProgram(ShaderID);
		}

		void setInt(const string& name, int value) const
		{
			glUniform1i(glGetUniformLocation(ShaderID, name.c_str()), value);
		}

		void setBool(const string& name, bool value) const
		{
			glUniform1i(glGetUniformLocation(ShaderID, name.c_str()), value);
		}

		void setFloat(const string& name, bool value) const
		{
			glUniform1f(glGetUniformLocation(ShaderID, name.c_str()), value);
		}

		void setVec2(const string& name, glm::vec2 value) const
		{
			glUniform2fv(glGetUniformLocation(ShaderID, name.c_str()), 1, &value[0]);
		}

		void setVec2(const string& name, float x, float y) const
		{
			glUniform2f(glGetUniformLocation(ShaderID, name.c_str()), x, y);
		}

		void setVec3(const string& name, glm::vec3 value) const
		{
			glUniform3fv(glGetUniformLocation(ShaderID, name.c_str()), 1, &value[0]);
		}

		void setVec3(const string& name, float x, float y, float z) const
		{
			glUniform3f(glGetUniformLocation(ShaderID, name.c_str()), x, y, z);
		}

		void setVec4(const string& name, glm::vec3 value) const
		{
			glUniform4fv(glGetUniformLocation(ShaderID, name.c_str()), 1, &value[0]);
		}

		void setVec4(const string& name, float x, float y, float z, float w) const
		{
			glUniform4f(glGetUniformLocation(ShaderID, name.c_str()), x, y, z, w);
		}

		void setMat2(const string& name, glm::mat2& value) const
		{
			glUniformMatrix2fv(glGetUniformLocation(ShaderID, name.c_str()), 1, GL_FALSE, &value[0][0]);
		}

		void setMat3(const string& name, glm::mat3& value) const
		{
			glUniformMatrix3fv(glGetUniformLocation(ShaderID, name.c_str()), 1, GL_FALSE, &value[0][0]);
		}
		
		void setMat4(const string& name, glm::mat4& value) const
		{
			glUniformMatrix4fv(glGetUniformLocation(ShaderID, name.c_str()), 1, GL_FALSE, &value[0][0]);
		}

		GLint getAttribute(const string& name)
		{
			return glGetAttribLocation(ShaderID, name.c_str());
		}
	private:
		bool showInfoLog;
		void checkCompileErrors(unsigned int shader, string type)
		{
			int success;
			char infoLog[1000];
			if (type != "program") {
				glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
				if (!success) {
					glGetShaderInfoLog(shader, 1000, NULL, infoLog);
					cerr << "[ERROR] Failed to compile shader.\n";
					cerr << "[ERROR] Shader type is " << type << "\n";
					cerr << "[ERROR] Position is " << __FILE__ << ":" << __LINE__ << "(file:line)\n";
					if (showInfoLog) {
						cout << infoLog << "\n";
					}
				}
			} else {
				glGetProgramiv(shader, GL_LINK_STATUS, &success);
				if (!success) {
					glGetProgramInfoLog(shader, 1000, NULL, infoLog);
					cerr << "[ERROR] Failed to link programs.\n";
					cerr << "[ERROR] Position is " << __FILE__ << ":" << __LINE__ << "(file:line)\n";
					if (showInfoLog) {
						cout << infoLog << "\n";
					}
				}
			}
		}
	};

}
#endif 

