/*
Student Information
Student ID:
Student Name:
*/

#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"

#include "Shader.h"
#include "Texture.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

// screen setting
GLFWwindow* window;
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

struct keyboardcontroller
{
	bool UP = false;
	bool DOWN = false;
	bool LEFT = false;
	bool RIGHT = false;
};

keyboardcontroller keyboardCtl;

float y_delta = 1.0f;//number of degree change in y-axis
float y_press_num = 0.0f;//number of changes in y-axis
float x_delta = 1.0f;//number of degree change in y-axis
float x_press_num = 0.0f;//number of changes in y-axis
float z_delta = 1.0f;//number of degree change in y-axis
float z_press_num = 0.0f;//number of changes in y-axis

float rotation = 0.0f;
float rotation2 = 0.0f;
double prevTime = glfwGetTime();
float x_delta2 = 0.3f;//number of degree change in y-axis
float hori = 0.0f;

Shader shader;

glm::vec3 camPos;//camera position
const int n = 5;
GLuint vao[n];
GLuint vbo[n];
GLuint ebo[n];

Texture texture[6];

// struct for storing the obj file
struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct Model {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

Model loadOBJ(const char* objPath)
{
	// function to load the obj file
	// Note: this simple function cannot load all obj files.

	struct V {
		// struct for identify if a vertex has showed up
		unsigned int index_position, index_uv, index_normal;
		bool operator == (const V& v) const {
			return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
		}
		bool operator < (const V& v) const {
			return (index_position < v.index_position) ||
				(index_position == v.index_position && index_uv < v.index_uv) ||
				(index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
		}
	};

	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::map<V, unsigned int> temp_vertices;

	Model model;
	unsigned int num_vertices = 0;

	std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

	std::ifstream file;
	file.open(objPath);

	// Check for Error
	if (file.fail()) {
		std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
		exit(1);
	}
	while (!file.eof()) {
		// process the object file
		char lineHeader[128];
		file >> lineHeader;

		if (strcmp(lineHeader, "v") == 0) {
			// geometric vertices
			glm::vec3 position;
			file >> position.x >> position.y >> position.z;
			if (objPath == "resources/object/spacecraft.obj") {
				position.y = position.y - 220.0f;
				position.z = position.z - 120.0f;
			}
			temp_positions.push_back(position);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			// texture coordinates
			glm::vec2 uv;
			file >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			// vertex normals
			glm::vec3 normal;
			file >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			// Face elements
			V vertices[3];
			for (int i = 0; i < 3; i++) {
				char ch;
				file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
			}

			// Check if there are more than three vertices in one face.
			/*std::string redundency;
			std::getline(file, redundency);
			if (redundency.length() >= 5) {
				std::cerr << "There may exist some errors while load the obj file. Error content: [" << redundency << " ]" << std::endl;
				std::cerr << "Please note that we only support the faces drawing with triangles. There are more than three vertices in one face." << std::endl;
				std::cerr << "Your obj file can't be read properly by our simple parser :-( Try exporting with other options." << std::endl;
				exit(1);
			}*/

			for (int i = 0; i < 3; i++) {
				if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
					// the vertex never shows before
					Vertex vertex;
					vertex.position = temp_positions[vertices[i].index_position - 1];
					vertex.uv = temp_uvs[vertices[i].index_uv - 1];
					vertex.normal = temp_normals[vertices[i].index_normal - 1];

					model.vertices.push_back(vertex);
					model.indices.push_back(num_vertices);
					temp_vertices[vertices[i]] = num_vertices;
					num_vertices += 1;
				}
				else {
					// reuse the existing vertex
					unsigned int index = temp_vertices[vertices[i]];
					model.indices.push_back(index);
				}
			} // for
		} // else if
		else {
			// it's not a vertex, texture coordinate, normal or face
			char stupidBuffer[1024];
			file.getline(stupidBuffer, 1024);
		}
	}
	file.close();

	std::cout << "There are " << num_vertices << " vertices in the obj file.\n" << std::endl;
	return model;
}

Model spacecraft = loadOBJ("resources/object/spacecraft.obj");
Model rock = loadOBJ("resources/object/rock.obj");
Model planet = loadOBJ("resources/object/planet.obj");
Model craft = loadOBJ("resources/object/craft.obj");

void get_OpenGL_info()
{
	// OpenGL information
	const GLubyte* name = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* glversion = glGetString(GL_VERSION);
	std::cout << "OpenGL company: " << name << std::endl;
	std::cout << "Renderer name: " << renderer << std::endl;
	std::cout << "OpenGL version: " << glversion << std::endl;
}

void sendDataToOpenGL()
{
	//TODO
	//Load objects and bind to VAO and VBO
	glGenVertexArrays(1, &vao[0]);
	glBindVertexArray(vao[0]);

	glGenBuffers(1, &vbo[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, spacecraft.vertices.size() * sizeof(Vertex), &spacecraft.vertices[0], GL_STATIC_DRAW);

	//vertex position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);

	//vertex uv
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);

	glGenBuffers(1, &ebo[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, spacecraft.indices.size() * sizeof(unsigned int), &spacecraft.indices[0], GL_STATIC_DRAW);
	///////////////////////////////////////////////////////////////////////////////////
	glGenVertexArrays(1, &vao[1]);
	glBindVertexArray(vao[1]);

	glGenBuffers(1, &vbo[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, rock.vertices.size() * sizeof(Vertex), &rock.vertices[0], GL_STATIC_DRAW);

	//vertex position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);

	//vertex uv
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);

	glGenBuffers(1, &ebo[1]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, rock.indices.size() * sizeof(unsigned int), &rock.indices[0], GL_STATIC_DRAW);
	///////////////////////////////////////////////////////////////////////////////////
	glGenVertexArrays(1, &vao[2]);
	glBindVertexArray(vao[2]);

	glGenBuffers(1, &vbo[2]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, planet.vertices.size() * sizeof(Vertex), &planet.vertices[0], GL_STATIC_DRAW);

	//vertex position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);

	//vertex uv
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);

	glGenBuffers(1, &ebo[2]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, planet.indices.size() * sizeof(unsigned int), &planet.indices[0], GL_STATIC_DRAW);
	///////////////////////////////////////////////////////////////////////////////////
	glGenVertexArrays(1, &vao[3]);
	glBindVertexArray(vao[3]);

	glGenBuffers(1, &vbo[3]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, craft.vertices.size() * sizeof(Vertex), &craft.vertices[0], GL_STATIC_DRAW);

	//vertex position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);

	//vertex uv
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);

	glGenBuffers(1, &ebo[3]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, craft.indices.size() * sizeof(unsigned int), &craft.indices[0], GL_STATIC_DRAW);



	//Load textures
	texture[0].setupTexture("resources/texture/spacecraftTexture.bmp");
	texture[1].setupTexture("resources/texture/rockTexture.bmp");
	texture[2].setupTexture("resources/texture/earthTexture.bmp");
	texture[3].setupTexture("resources/texture/vehicleTexture.bmp");

}

const int num = 200;
float scaleMax = 0.7f;
float scaleMin = 0.1f;
float scale[num];
float yMax = 8.0f;
float yMin = 5.0f;
float y[num];
float circleMax = 135.0f;
float circleMin = 65.0f;
float z[num];
float x[num];


void initializedGL(void) //run only once
{
	if (glewInit() != GLEW_OK) {
		std::cout << "GLEW not OK." << std::endl;
	}

	get_OpenGL_info();
	sendDataToOpenGL();

	//set up camera position
	//camPos = glm::vec3(0.0f, 50.0f, 0.0f);
	camPos = glm::vec3(0.0f, 3.5f, -6.5f);
	//TODO: set up the vertex shader and fragment shader
	shader.setupShader("VertexShaderCode.glsl", "FragmentShaderCode.glsl");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


	for (int i = 0; i < num; i++) {
		scale[i] = (scaleMax - scaleMin) * rand() / (RAND_MAX + 1.0) + scaleMin;
		y[i] = (yMax - yMin) * rand() / (RAND_MAX + 1.0) + yMin;
		z[i] = (circleMax - circleMin) * rand() / (RAND_MAX + 1.0) + circleMin;
		x[i] = sqrt(1225 - (z[i] - 100) * (z[i] - 100));
		if (i % 2 == 0)
			x[i] *= -1;
	};
}

void paintGL(void)  //always run
{
	glClearColor(1.0f, 1.0f, 1.0f, 0.5f); //specify the background color, this is just an example
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//TODO:
	shader.use();
	//Set lighting information, such as position and color of lighting source
	//Set transformation matrix
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 viewMatrix = glm::mat4(1.0f);
	glm::mat4 projectionMatrix = glm::mat4(1.0f);

	projectionMatrix = glm::perspective(glm::radians(75.0f), 1.0f, 0.1f, 300.0f);

	shader.setMat4("projectionMatrix", projectionMatrix);

	double crntTime = glfwGetTime();
	if (crntTime - prevTime >= 0.05) {
		rotation += 1.0f;
		prevTime = crntTime;
		if (hori <= 15.0f && hori >= -15.0f) {
			hori += x_delta2;
		}
		else {
			x_delta2 *= -1.0f;
			hori += x_delta2;
		}
		if (keyboardCtl.LEFT)
			x_press_num += 1.0f;
		if (keyboardCtl.RIGHT)
			x_press_num -= 1.0f;
		if (keyboardCtl.UP)
			z_press_num += 1.0f;
		if (keyboardCtl.DOWN)
			z_press_num -= 1.0f;
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		rotation2 = (xpos-400) / 800 * 360;
	}

	//viewMatrix = glm::lookAt(camPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	viewMatrix = glm::lookAt(camPos, glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	viewMatrix = glm::rotate(viewMatrix, glm::radians(rotation2), glm::vec3(0.0f, 1.0f, 0.0f));
	viewMatrix = glm::translate(viewMatrix, glm::vec3(-x_delta * x_press_num , -y_delta * y_press_num, -z_delta * z_press_num));
	shader.setMat4("viewMatrix", viewMatrix);

	modelMatrix = glm::translate(modelMatrix, glm::vec3(x_delta * x_press_num, y_delta * y_press_num, z_delta * z_press_num));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(-rotation2), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f / 200.0f));
	shader.setMat4("modelMatrix", modelMatrix);
	texture[0].bind(0);
	shader.setInt("myTextureSampler0", 0);
	glBindVertexArray(vao[0]);
	glDrawElements(GL_TRIANGLES, spacecraft.indices.size(), GL_UNSIGNED_INT, 0);
	modelMatrix = glm::scale(modelMatrix, glm::vec3(200.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation2), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-x_delta * x_press_num, -y_delta * y_press_num, -z_delta * z_press_num));

	texture[1].bind(0);
	shader.setInt("myTextureSampler0", 0);
	glBindVertexArray(vao[1]);

	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 6.5f, 100.0f));
	for (int i = 0; i < num; i++) {
		modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(x[i], y[i], z[i]));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -6.5f, -100.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(scale[i]));
		shader.setMat4("modelMatrix", modelMatrix);
		glDrawElements(GL_TRIANGLES, rock.indices.size(), GL_UNSIGNED_INT, 0);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1/scale[i]));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 6.5f, 100.0f));
		modelMatrix = glm::translate(modelMatrix, glm::vec3(-x[i], -y[i], -z[i]));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(-rotation), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -6.5f, -100.0f));



	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 100.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(10.0f));
	shader.setMat4("modelMatrix", modelMatrix);
	texture[2].bind(0);
	shader.setInt("myTextureSampler0", 0);
	glBindVertexArray(vao[2]);
	glDrawElements(GL_TRIANGLES, planet.indices.size(), GL_UNSIGNED_INT, 0);
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(-rotation), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -100.0f));


	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
	modelMatrix = glm::translate(modelMatrix, glm::vec3(hori, 0.0f, 20.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
	shader.setMat4("modelMatrix", modelMatrix);
	texture[3].bind(0);
	shader.setInt("myTextureSampler0", 0);
	glBindVertexArray(vao[3]);
	glDrawElements(GL_TRIANGLES, craft.indices.size(), GL_UNSIGNED_INT, 0);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
	// Sets the cursor position callback for the current window
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Sets the scoll callback for the current window.
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		keyboardCtl.LEFT = true;
	}
	if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
		keyboardCtl.LEFT = false;
	}
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		keyboardCtl.RIGHT = true;
	}
	if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
		keyboardCtl.RIGHT = false;
	}
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		keyboardCtl.UP = true;
	}
	if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
		keyboardCtl.UP = false;
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		keyboardCtl.DOWN = true;
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
		keyboardCtl.DOWN = false;
	}
}

int main(int argc, char* argv[])
{

	/* Initialize the glfw */
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	/* glfw: configure; necessary for MAC */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 2", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/*register callback functions*/
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);                                                                  //    
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	initializedGL();

	while (!glfwWindowShouldClose(window)) {
		/* Render here */
		paintGL();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}





