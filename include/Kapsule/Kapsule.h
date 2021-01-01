/*
 * A simple OpenGL wrapper created by Yuliang Li(2018151004)
 * =========================================================
 * Kapsule::Buffer: wrapper for VBO and EBO
 * Kapsule::VertexArray: wrapper for VAO
 * Kapsule::Shader: wrapper for shader program
 * Kapsule::Mesh: data structure for loading and rendering mesh of models
 * Kapsule::Model: a class for loading and renering .obj models
 * Kapsule::LayerModel: an n-ary tree based data structure for creating and rendering layer model
 * Kapsule::Camera: a simple camera class
 * Kapsule::Snapshot: some functions for capture screenshots
 * Kapsule::Skybox: a class for loading and rendering skybox
 * Kapsule::Utility: some tools
 * 2021.1.1
 */
#pragma once
#ifndef __KAPSULE_H__
#define __KAPSULE_H__

#include <Kapsule/Buffers.h>
#include <Kapsule/VertexArray.h>
#include <Kapsule/Shader.h>
#include <Kapsule/Mesh.h>
#include <Kapsule/Model.h>
#include <Kapsule/Camera.h>
#include <Kapsule/Utility.h>
#include <Kapsule/LayerModel.h>
#include <Kapsule/Snapshot.h>
#include <Kapsule/Skybox.h>

#endif // !__KAPSULE_H__
