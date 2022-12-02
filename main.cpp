/*
Student Information
 Names: WONG Chi Kwan Cyrus, LAU Ho Man Elvis
 Student IDs: 1155159006, 1155157519
*/

#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"

#include "Shader.h"
#include "Texture.h"

#include <algorithm>
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


float x_delta = 1.0f;//magnitude of spacecraft movement in x-axis
float x_press_num = 0.0f;//indicate number of corresponding movement
float z_delta = 1.0f;//magnitude of spacecraft movement in z-axis
float z_press_num = 0.0f;//indicate number of corresponding movement

glm::vec3 craftPos = glm::vec3(0.0f, 0.0f, 0.0f);

float rotation = 0.0f;// rotation angle for planet and local vehicle
float rotation2 = 0.0f;//rotation angle for spacecraft
double prevTime = glfwGetTime();//get time interval for rotation and other events

float x_delta2 = 0.5f;//magnitude of local vehicle movement in x-axis

struct UFO {
    glm::vec3 position;
    enum Status{intact=0, damaged=1, destroyed=2} status;
    float movement_magnitude = 0.5f;
    float x_movement_range[2];
};

std::vector<UFO> ufo_container;

float intensity = 1.0f;//intesity of directional light

bool space = false;


std::vector<glm::mat4> rocketPos;
std::vector<glm::mat4>::iterator it;

glm::vec3 PlanetPos = glm::vec3(0.0f, 0.0f, 100.0f);

Shader shader;

glm::vec3 cameraPos;//camera position
const int n = 6;//number of objects
GLuint vao[n];
GLuint vbo[n];
GLuint ebo[n];

Texture texture[7];

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

//Load different obects
Model spacecraft = loadOBJ("resources/object/spacecraft.obj");
Model rock = loadOBJ("resources/object/rock.obj");
Model planet = loadOBJ("resources/object/planet.obj");
Model craft = loadOBJ("resources/object/craft.obj");
Model skybox = loadOBJ("resources/skybox/skybox.obj");
Model rocket = loadOBJ("resources/rocket/rocket.obj");

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
    //bind spacecraft
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

    //vertex normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, normal) // array buffer offset
    );

    glGenBuffers(1, &ebo[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, spacecraft.indices.size() * sizeof(unsigned int), &spacecraft.indices[0], GL_STATIC_DRAW);
    
    //bind rock
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

    //vertex normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, normal) // array buffer offset
    );

    glGenBuffers(1, &ebo[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rock.indices.size() * sizeof(unsigned int), &rock.indices[0], GL_STATIC_DRAW);
    
    //bind planet
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

    //vertex normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, normal) // array buffer offset
    );

    glGenBuffers(1, &ebo[2]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, planet.indices.size() * sizeof(unsigned int), &planet.indices[0], GL_STATIC_DRAW);
    
    //bind local vehicle
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

    //vertex normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, normal) // array buffer offset
    );

    glGenBuffers(1, &ebo[3]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, craft.indices.size() * sizeof(unsigned int), &craft.indices[0], GL_STATIC_DRAW);

    //bind skybox
    glGenVertexArrays(1, &vao[4]);
    glBindVertexArray(vao[4]);

    glGenBuffers(1, &vbo[4]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
    glBufferData(GL_ARRAY_BUFFER, skybox.vertices.size() * sizeof(Vertex), &skybox.vertices[0], GL_STATIC_DRAW);

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

    //vertex normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, normal) // array buffer offset
    );

    glGenBuffers(1, &ebo[4]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[4]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, skybox.indices.size() * sizeof(unsigned int), &skybox.indices[0], GL_STATIC_DRAW);


    //bind skybox
    glGenVertexArrays(1, &vao[4]);
    glBindVertexArray(vao[4]);

    glGenBuffers(1, &vbo[4]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
    glBufferData(GL_ARRAY_BUFFER, skybox.vertices.size() * sizeof(Vertex), &skybox.vertices[0], GL_STATIC_DRAW);

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

    //vertex normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, normal) // array buffer offset
    );

    glGenBuffers(1, &ebo[4]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[4]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, skybox.indices.size() * sizeof(unsigned int), &skybox.indices[0], GL_STATIC_DRAW);

    //bind rocket
    glGenVertexArrays(1, &vao[5]);
    glBindVertexArray(vao[5]);

    glGenBuffers(1, &vbo[5]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
    glBufferData(GL_ARRAY_BUFFER, rocket.vertices.size() * sizeof(Vertex), &rocket.vertices[0], GL_STATIC_DRAW);

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

    //vertex normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, normal) // array buffer offset
    );

    glGenBuffers(1, &ebo[5]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[5]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rocket.indices.size() * sizeof(unsigned int), &rocket.indices[0], GL_STATIC_DRAW);
    
    //Load textures
    texture[0].setupTexture("resources/texture/spacecraftTexture.bmp");
    texture[1].setupTexture("resources/texture/rockTexture.bmp");
    texture[2].setupTexture("resources/texture/earthTexture.bmp");
    texture[3].setupTexture("resources/texture/vehicleTexture.bmp");
    texture[4].setupTexture("resources/skybox/all_face_textures.png");
    texture[5].setupTexture("resources/texture/earthNormal.bmp");
    texture[6].setupTexture("resources/rocket/rocket.jpg");
    texture[7].setupTexture("resources/texture/vehicleTextureRed.png");
    
}

const int num = 200;//number of rocks in the ring
float scaleMax = 0.7f;//upper limit for the size of the rock
float scaleMin = 0.1f;//lower limit for the size of the rock
float scale[num];//stored size of different rock
float yMax = 8.0f;//upper limit for the y-coor of the rock
float yMin = 5.0f;//lower limit for the y-coor of the rock
float y[num];//stored y-coor of different rock
float circleMax = 135.0f;//upper limit for the size of the ring
float circleMin = 65.0f;//lower limit for the size of the ring
float z[num];//stored z-coor of different rock
float x[num];//stored x-coor of different rock


void initializedGL(void) //run only once
{
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW not OK." << std::endl;
    }

    get_OpenGL_info();
    sendDataToOpenGL();

    // Set up ufo basic info
    UFO aUFO;
    aUFO.position = glm::vec3(0.0f, 0.0f, 20.0f);
    aUFO.status = UFO::intact;
    aUFO.x_movement_range[0] = -15.0f;
    aUFO.x_movement_range[1] = 15.0f;
    aUFO.movement_magnitude = 0.3f;
    ufo_container.push_back(aUFO);
    
    //set up camera position
    cameraPos = glm::vec3(0.0f, 3.5f, -6.5f);
    //TODO: set up the vertex shader and fragment shader
    shader.setupShader("VertexShaderCode.glsl", "FragmentShaderCode.glsl");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    //randomize the postion and scale of the rocks
    for (int i = 0; i < num; i++) {
        scale[i] = (scaleMax - scaleMin) * rand() / (RAND_MAX + 1.0) + scaleMin;
        y[i] = (yMax - yMin) * rand() / (RAND_MAX + 1.0) + yMin;
        z[i] = (circleMax - circleMin) * rand() / (RAND_MAX + 1.0) + circleMin;
        x[i] = sqrt(1225 - (z[i] - 100) * (z[i] - 100));
        if (i % 2 == 0)
            x[i] *= -1;
    };
}

float xzDistance(glm::vec3 pos1, glm::vec3 pos2) {
    return pow((pos1.x - pos2.x),2) + pow((pos1.z - pos2.z),2);
}
/*
    Given the locations of rockets, the locations of UFOs,
    we can have a function to calculate whether the rocket is collided with any of the UFO.
    This can be realized by calculating the distance of UFO and rocket. If true, in other words,
    if it is lower than a certain threshold, on the first hit, UFO should turn into red. (Change the texture mapping).
    On the second hit, it should disappear. (Remove its location from array)
    TODO: Implement above
 */

bool isLegalPosition(glm::vec3 objNextPos) {
    // skybox
    if ((objNextPos.x <= -200 || objNextPos.x >= 200) ||
        (objNextPos.y <= -200 || objNextPos.y >= 200) ||
        (objNextPos.z <= -200 || objNextPos.z >= 200))
    {
        std::cout << "DEBUG: Too close to skybox" << std::endl;
        return false;
    }
    
    // planet
    if (xzDistance(objNextPos, PlanetPos) < 200) {
        std::cout << "DEBUG: Too close to planet" << std::endl;
        return false;
    }
    
    // UFO
    for (std::vector<UFO>::iterator iter = ufo_container.begin();
         iter != ufo_container.end(); iter++) {
        if (xzDistance(iter->position, objNextPos) < 50) {
            std::cout << "DEBUG: Too close to UFO" << std::endl;
            // change the status of UFO
            if (iter->status == UFO::intact) {
                iter->status = UFO::damaged;
            } else if (iter->status == UFO::damaged) {
                iter->status = UFO::destroyed;
            }
            
            return false;
        }
    }
   
    
    return true;
}



void paintGL(void)  //always run
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f); //specify the background color, this is just an example
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //TODO:
    shader.use();
    //Set lighting information, such as position and color of lighting source
    //lighting color
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 diffuseColor = lightColor * glm::vec3(0.8f);
    glm::vec3 ambientColor = lightColor * glm::vec3(0.05f);

    //Directional light info
    shader.setVec3("dirLight.direction", glm::vec3(0.0f, -1.0f, 0.0f));
    shader.setVec3("dirLight.ambient", ambientColor);
    shader.setVec3("dirLight.diffuse", diffuseColor);
    shader.setVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setFloat("dirLight.intensity", intensity);

    //Point light info
    shader.setVec3("pointLight.position", glm::vec3(15.0f, 7.0f, 35.0f));
    shader.setVec3("pointLight.ambient", ambientColor);
    shader.setVec3("pointLight.diffuse", diffuseColor);
    shader.setVec3("pointLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setFloat("pointLight.constant", 0.1f);
    shader.setFloat("pointLight.linear", 0.0014f);
    shader.setFloat("pointLight.quadratic", 0.00032f);

    //Set transformation matrix
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);

    //set perspective
    projectionMatrix = glm::perspective(glm::radians(75.0f), 1.0f, 0.1f, 500.0f);
    shader.setMat4("projectionMatrix", projectionMatrix);

    //for every 0.05s trigger some events
    double crntTime = glfwGetTime();
    if (crntTime - prevTime >= 0.05) {
        rotation += 1.0f;//increase angle -> self rotation
        prevTime = crntTime;//set incoming time interval
        
        //set movement range for local vehicle
        for (std::vector<UFO>::iterator iter = ufo_container.begin(); iter != ufo_container.end(); iter++) {
            if (iter->position.x >= iter->x_movement_range[0] && iter->position.x <= iter->x_movement_range[1]) {
                
                iter->position.x += iter->movement_magnitude;//move forward on the track
            } else {
                iter->movement_magnitude *= -1.0f;
                iter->position.x += iter->movement_magnitude;//move backward on the track
            }
        }
        
        
        //continuous movement for spacecraft is button is holding down
        if (keyboardCtl.LEFT)
            x_press_num += 1.0f;
        if (keyboardCtl.RIGHT)
            x_press_num -= 1.0f;
        if (keyboardCtl.UP)
            z_press_num += 1.0f;
        if (keyboardCtl.DOWN)
            z_press_num -= 1.0f;
        double xpos, ypos;//position of mouse
        glfwGetCursorPos(window, &xpos, &ypos);
        rotation2 = (xpos-400) / 800 * 360;//rotation angle for spacecraft
    }
    
    for (it = rocketPos.begin();it != rocketPos.end();it++) {
        glm::mat4 modelMatrixR = *it;
        modelMatrixR = glm::translate(modelMatrixR, glm::vec3(0.0f, 0.0f, 0.5f * 800.0f));
        std::replace(rocketPos.begin(), rocketPos.end(), *it, modelMatrixR);
    }
    

    //set up stationary camera related to spacectaft
    viewMatrix = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    viewMatrix = glm::rotate(viewMatrix, glm::radians(rotation2), glm::vec3(0.0f, 1.0f, 0.0f));
    viewMatrix = glm::translate(viewMatrix, glm::vec3(-x_delta * x_press_num , 0.0f, -z_delta * z_press_num));
    //sent info to shader
    shader.setVec3("eyePositionWorld", cameraPos);
    shader.setMat4("viewMatrix", viewMatrix);
    shader.setInt("normalMapping_flag", 0);//disable normal mapping as default
    shader.setInt("light_flag", 1);//enable light as default

    

    //set model matrix and draw spacecraft
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x_delta * x_press_num, 0.0f, z_delta * z_press_num));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-rotation2), glm::vec3(0.0f, 1.0f, 0.0f));
    
    if (space) {
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f / 800.0f));
        rocketPos.push_back(modelMatrix);
        space = false;
        modelMatrix = glm::scale(modelMatrix, glm::vec3(800.0f));
    }
    

    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f / 200.0f));
    shader.setMat4("modelMatrix", modelMatrix);
    texture[0].bind(0);
    shader.setInt("myTextureSampler0", 0);
    glBindVertexArray(vao[0]);
    glDrawElements(GL_TRIANGLES, (int)spacecraft.indices.size(), GL_UNSIGNED_INT, 0);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(200.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation2), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-x_delta * x_press_num, 0.0f, -z_delta * z_press_num));
    
    /*
     ==============
         Rockets
     ==============
     */
    for (it = rocketPos.begin();it != rocketPos.end();it++) {
        // Remove if the rocket is out of skybox
        glm::vec3 rocketLocVec = glm::vec3((*it)[3]);
        
        // Handle illegal position for rocket
        if (!isLegalPosition(rocketLocVec)) {
            rocketPos.erase(it);
            break;
        }
        
        texture[6].bind(0);
        shader.setInt("myTextureSampler0", 0);
        glBindVertexArray(vao[5]);
        shader.setMat4("modelMatrix", *it);
        glDrawElements(GL_TRIANGLES, (int)rocket.indices.size(), GL_UNSIGNED_INT, 0);
    }
    
    //set model matrix and draw rock
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
        glDrawElements(GL_TRIANGLES, (int)rock.indices.size(), GL_UNSIGNED_INT, 0);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1/scale[i]));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 6.5f, 100.0f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-x[i], -y[i], -z[i]));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(-rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -6.5f, -100.0f));
    
    /*
     ===============
         Planet
     ===============
     */
    //enable normal mapping
    shader.setInt("normalMapping_flag", 1);
    //set model matrix and draw planet
    modelMatrix = glm::translate(modelMatrix, PlanetPos);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(10.0f));
    shader.setMat4("modelMatrix", modelMatrix);
    texture[5].bind(0);
    shader.setInt("myTextureSampler1", 0);
    texture[2].bind(0);
    shader.setInt("myTextureSampler0", 0);
    glBindVertexArray(vao[2]);
    glDrawElements(GL_TRIANGLES, (int)planet.indices.size(), GL_UNSIGNED_INT, 0);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -100.0f));

    /*
     =========
        UFO
     =========
     */
    //disable normal mapping
    shader.setInt("normalMapping_flag", 0);
    //set model matrix and draw local vehicle
    for (std::vector<UFO>::iterator iter = ufo_container.begin();
         iter != ufo_container.end(); iter++) {
        
        if (iter->status == UFO::intact) {
            texture[3].bind(0);
        } else if (iter->status == UFO::damaged) {
            texture[7].bind(0);
        } else if (iter->status == UFO::destroyed) {
            continue;
        }
        
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
        modelMatrix = glm::translate(modelMatrix, iter->position);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("modelMatrix", modelMatrix);
        shader.setInt("myTextureSampler0", 0);
        glBindVertexArray(vao[3]);
        glDrawElements(GL_TRIANGLES, (int)craft.indices.size(), GL_UNSIGNED_INT, 0);
        
        modelMatrix = glm::rotate(modelMatrix, glm::radians(-rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::translate(modelMatrix,  iter->position * -1.0f);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f));
       
        

    }

   

    //disable light
    shader.setInt("light_flag", 0);
    //set model matrix and draw skybox
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(4.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -30.0f, 0.0f));
    shader.setMat4("modelMatrix", modelMatrix);
    texture[4].bind(0);
    shader.setInt("myTextureSampler0", 0);
    glBindVertexArray(vao[4]);
    glDrawElements(GL_TRIANGLES, (int)skybox.indices.size(), GL_UNSIGNED_INT, 0);
    
   
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
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        keyboardCtl.LEFT = true;//left is holding down
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
        keyboardCtl.LEFT = false;//left is released
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        keyboardCtl.RIGHT = true;//right is holding down
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
        keyboardCtl.RIGHT = false;//right is released
    }
    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        keyboardCtl.UP = true;//up is holding down
    }
    if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
        keyboardCtl.UP = false;//up is released
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        keyboardCtl.DOWN = true;//down is holding down
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
        keyboardCtl.DOWN = false;//down is released
    }
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {//increase light intensity when it is below 1.0
        if (intensity < 1.0f)
            intensity += 0.1f;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {//decrease light intensity when it is above 0.0
        if (intensity > 0.0f)
            intensity -= 0.1f;
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        if (!space) {
            space = true;
        }
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        for (std::vector<UFO>::iterator iter = ufo_container.begin();
             iter != ufo_container.end(); iter++) {
            iter->status = UFO::intact;
            iter->position = glm::vec3(0.0f, 0.0f, 20.0f);
        }
    }
//    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
//        UFO newUFO;
//        newUFO.status = UFO::intact;
//
//        float x_coord = ((rand() / (RAND_MAX + 1.0f)) * 400.0f) - 200.0f;
//        float y_coord = 0.0f;
//        float z_coord = ((rand() / (RAND_MAX + 1.0f)) * 400.0f) - 200.0f;
//        newUFO.position = glm::vec3(x_coord, y_coord, z_coord);
//
//        float tmp = (rand() / (RAND_MAX + 1.0f)) * 20.0f;
//        newUFO.x_movement_range[0] = (-1 * tmp) + x_coord;
//        newUFO.x_movement_range[1] = tmp + x_coord;
//
//        ufo_container.push_back(newUFO);
//    }
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
