#pragma once
#ifndef __VERTEX_ARRAY_H__
#define __VERTEX_ARRAY_H__

#include <glad/glad.h>
#include <functional>
#include <set>

#include "Handle.h"
#include "Buffers.h"

using std::function;
using std::set;


namespace Kapsule {
	class VertexArray {
	public:
		GLuint id;
		void create()
		{
			glGenVertexArrays(1, &id);
		}
		void bind()
		{
			glBindVertexArray(id);
		}
		void destroy()
		{
			glDeleteVertexArrays(1, &id);
		}
		void setAttributePointer(VertexBuffer& b, GLint attrib, size_t size, GLenum type, bool normalized, size_t stride, void* offset)
		{
			glBindBuffer(GL_ARRAY_BUFFER, b);
			glEnableVertexAttribArray(attrib);
			glVertexAttribPointer(attrib, size, type, normalized, stride, offset);
			vbos.insert(b);
		}
		void unbind()
		{
			glBindVertexArray(0);
		}
		operator GLuint()
		{
			return id;
		}
	private:
		set<VertexBuffer> vbos;
	};
}

#endif // !__VERTEX_ARRAY_H__
