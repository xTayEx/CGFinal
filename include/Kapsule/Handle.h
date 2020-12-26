#pragma once
#ifndef __HANDLE_H__
#define __HANDLE_H__

#include <memory>
#include <functional>
#include <iostream>
#include <glad/glad.h>

using std::shared_ptr;
using std::cerr;
using std::cout;

namespace Kapsule {
	class Handle {
	public:
		Handle() : objPtr(nullptr) {}
		Handle(GLuint obj, std::function<void(GLuint*)> delFunc)
			: objPtr(new GLuint(obj), delFunc)
		{

		}
		void release()
		{
			objPtr.reset();
		}
		operator GLuint() const
		{
			if (objPtr == nullptr) {
				return -1;
			} else {
				return *objPtr.get();
			}
		}
	private:
		shared_ptr<GLuint> objPtr;
	};
}

#endif // !__HANDLE_H__
