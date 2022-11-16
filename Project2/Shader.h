#pragma once
#include "Dependencies/glew/glew.h"
#include "Dependencies/glm/glm.hpp"

#include <string>
#include <iostream>

class Shader {
public:
	void setupShader(const char* vertexPath, const char* fragmentPath);
	void use() const;

	// a series utilities for setting shader parameters 
	void setMat4(const std::string& name, glm::mat4& value) const;
	void setVec4(const std::string& name, glm::vec4 value) const;
	void setVec3(const std::string& name, glm::vec3 value) const;
	void setVec3(const std::string& name, float v1, float v2, float v3) const;
	void setFloat(const std::string& name, float value) const;
	void setInt(const std::string& name, int value) const;

private:
	unsigned int ID = 0;

	std::string readShaderCode(const char* fileName) const;
	bool checkShaderStatus(GLuint shaderID) const;
	bool checkProgramStatus(GLuint programID) const;
	bool checkStatus(
		GLuint objectID,
		PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
		PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
		GLenum statusType) const;
};
