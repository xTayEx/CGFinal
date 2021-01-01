#pragma once
#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

namespace Kapsule {
	using namespace std;
	// A fps camera
	enum Movement { FORWARD, LEFT, RIGHT, BACKWARD, UP, DOWN };
	class Camera {
	public:
		glm::vec3 eye;
		glm::vec3 up;
		glm::vec3 front;
		glm::vec3 right;

		float yaw;
		float pitch;

		float moveSpeed;

		float sensitivity;

		Camera(glm::vec3 eye, 
			   glm::vec3 up,
			   float yaw = -90.0f,
			   float pitch = 0.0f,
			   float moveSpeed = 6.0f)
		{
			front = glm::vec3(0.0f, 0.0f, 1.0f);
			this->eye = eye;
			this->up = up;
			this->yaw = yaw;
			this->pitch = pitch;
			this->moveSpeed = moveSpeed;
			this->right = glm::normalize(glm::cross(front, up));
			this->sensitivity = 0.1;
		}

		glm::mat4 getViewMatrix()
		{
			return glm::lookAt(eye, eye + front, up);
		}

		void processKeybooard(Movement dir, float deltaTime)
		{
			glm::vec3 frontDir(front.x, 0.0f, front.z);
			float deltaDistance = moveSpeed * deltaTime;
			switch (dir) {
			case Kapsule::FORWARD:
				eye += frontDir * deltaDistance;
				break;
			case Kapsule::LEFT:
				eye -= right * deltaDistance;
				//cerr << "eye: " << eye.x << ", " << eye.y << ", " << eye.z << "\n";
				break;
			case Kapsule::RIGHT:
				eye += right * deltaDistance;
				//cerr << "eye: " << eye.x << ", " << eye.y << ", " << eye.z << "\n";
				break;
			case Kapsule::BACKWARD:
				eye -= frontDir * deltaDistance;
				break;
			case Kapsule::UP:
				eye += up * deltaDistance;
				break;
			case Kapsule::DOWN:
				eye -= up * deltaDistance;
				break;
			default:
				break;
			}
		}

		void processMouse(float x, float y)
		{
			x *= sensitivity;
			y *= sensitivity;
			this->yaw -= x * 2;
			this->pitch += y;

			this->pitch = min(89.0f, this->pitch);
			this->pitch = max(-30.0f, this->pitch);

			updateVectors();
		}

		void setEyePosition(glm::vec3 pos)
		{
			eye = pos;
		}

	private:
		void updateVectors()
		{
			glm::vec3 newFront;
			newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			newFront.y = sin(glm::radians(pitch));
			newFront.z = -sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			this->front = newFront;

			this->right = glm::normalize(glm::cross(this->front, glm::vec3(0.0f, 1.0f, 0.0f)));
			//cerr << "this->right: " << this->right.x << ", " << this->right.y << ", " << this->right.z << "\n";
			this->up = glm::normalize(glm::cross(this->right, this->front));
		}
	};
}
#endif // __CAMERA_H__
