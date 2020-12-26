#pragma once
#include <glad/glad.h>

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#ifndef __UTILITY_H__
#define __UTILITY_H__

#define DEBUG { cerr << "For debug purpose. On line: " <<  __LINE__ << "\n" << "error code: " << glGetError() << "\n"; }
#define DEBUGX(x) { cerr << #x << ": " << x << "\n"; }

namespace Kapsule {
	using namespace std;
	string readFile(const string& filePath)
	{
		ifstream inf;
		stringstream ss;
		string result = "";
		inf.open(filePath.c_str());
		inf.exceptions(ifstream::failbit | ifstream::badbit);
		try {
			ss << inf.rdbuf();
			result = ss.str();
		} catch (ifstream::failure e) {
			cerr << "[ERROR] Failed to load file. File path is " << filePath << "\n";
			cerr << "[ERROR] Position is " << __FILE__ << ":" << __LINE__ << "(file:line)\n";
		}
		return result;
	}
	constexpr double pi = 3.141592653589793;
};
#endif // __UTILITY_H__

