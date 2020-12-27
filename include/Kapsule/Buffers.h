#pragma once
#ifndef __BUFFERS_H__
#define __BUFFERS_H__

#include <glad/glad.h>
#include <iostream>
#include <functional>
#include <array>
#include <vector>
#include <Kapsule/Handle.h>

using std::function;
using std::array;
using std::vector;
using std::cin;
using std::cout;

namespace Kapsule {
	enum class USAGE {
		STATIC_DRAW = GL_STATIC_DRAW, 
		DYNAMIC_DRAW = GL_DYNAMIC_DRAW
	};
	class VertexBuffer {
	public:
		GLuint id;
		
		void create()
		{
			glGenBuffers(1, &id);
		}
		
		void bind()
		{
			glBindBuffer(GL_ARRAY_BUFFER, id);
		}
		
		template<typename T>
		void loadData(const vector<T>& data, USAGE usage)
		{
			glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), &data[0], static_cast<GLenum>(usage));
		}

		template<typename T>
		void loadData(const T data[], size_t size, USAGE usage)
		{
			glBufferData(GL_ARRAY_BUFFER, size, data, static_cast<GLenum>(usage));
		}

		void loadData(size_t size, USAGE usage)
		{
			glBufferData(GL_ARRAY_BUFFER, size, NULL, static_cast<GLenum>(usage));
		}

		template<typename T>
		void loadSubData(const vector<T>& data, size_t offset)
		{
			glBufferSubData(GL_ARRAY_BUFFER, offset, data.size() * sizeof(T), &data[0]);
		}

		template<typename T>
		void loadSubData(const T data[], size_t size, size_t offset)
		{
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
		}

		void destroy()
		{
			glDeleteBuffers(1, &id);
		}

		operator GLuint()
		{
			return id;
		}

		bool operator<(const VertexBuffer& b) const
		{
			return id < b.id;
		}
	};

	class ElementBuffer {
	public:
		GLuint id;

		void create()
		{
			glGenBuffers(1, &id);
		}

		void bind()
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
		}

		template<typename T>
		void loadData(const vector<T>& data, USAGE usage)
		{
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(T), &data[0], static_cast<GLenum>(usage));
		}

		template<typename T>
		void loadData(const T data[], size_t size, USAGE usage)
		{
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, static_cast<GLenum>(usage));
		}

		void loadData(size_t size, USAGE usage)
		{
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, NULL, static_cast<GLenum>(usage));
		}

		template<typename T>
		void loadSubData(const vector<T>& data, size_t size, size_t offset)
		{
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data);
		}

		void destroy()
		{
			glDeleteBuffers(1, &id);
		}

		operator GLuint()
		{
			return id;
		}

		bool operator<(const ElementBuffer& b) const
		{
			return id < b.id;
		}
	};

	class FrameBuffer {
	public:
		GLuint id;
		void create()
		{
			glGenFramebuffers(1, &id);
		}

		void bind()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, id);
		}

		void unbind()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		
		void createDepthBuffer(int width, int height)
		{
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glBindFramebuffer(GL_FRAMEBUFFER, texture);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	private:
		GLuint texture;
	};
}

#endif // !__BUFFERS_H__
