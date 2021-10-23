#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <shader.h>
#include <cmath>
#include <iostream>
#include <vector>

using namespace  std;

using namespace std;
// structure to hold the info necessary to render an object
struct SceneObject {
    unsigned int VAO;           // vertex array object handle
    unsigned int vertexCount;   // number of vertices in the object
    float r, g, b;              // for object color
    float x, y;                 // for position offset
};

// declaration of the function you will implement in voronoi 1.1
SceneObject instantiateCone(float r, float g, float b, float offsetX, float offsetY);
// helper functions
float angleToRad(float angle);
// mouse, keyboard and screen reshape glfw callbacks
void button_input_callback(GLFWwindow* window, int button, int action, int mods);
void key_input_callback(GLFWwindow* window, int button, int other,int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

// global variables we will use to store our objects, shaders, and active shader
std::vector<SceneObject> sceneObjects;
std::vector<Shader> shaderPrograms;
Shader* activeShader;


int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment - Voronoi Diagram", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // setup frame buffer size callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // setup input callbacks
    glfwSetMouseButtonCallback(window, button_input_callback); // NEW!
    glfwSetKeyCallback(window, key_input_callback); // NEW!

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // NEW!
    // build and compile the shader programs
    shaderPrograms.push_back(Shader("shaders/shader.vert", "shaders/color.frag"));
    shaderPrograms.push_back(Shader("shaders/shader.vert", "shaders/distance.frag"));
    shaderPrograms.push_back(Shader("shaders/shader.vert", "shaders/distance_color.frag"));
    activeShader = &shaderPrograms[0];

    // NEW!
    // set up the z-buffer
    glDepthRange(1,-1); // make the NDC a right handed coordinate system, with the camera pointing towards -z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // background color
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // notice that now we are clearing two buffers, the color and the z-buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render the cones
        glUseProgram(activeShader->ID);

        // TODO voronoi 1.3
        for(SceneObject cone : sceneObjects){
            //Offset uniform
            int vertexOffsetLocation = glGetUniformLocation(activeShader->ID, "offset");
            glUniform2f(vertexOffsetLocation, cone.x, cone.y);

            //Color uniform
            int fragmentColorLocation = glGetUniformLocation(activeShader->ID, "color");
            if(fragmentColorLocation != -1)
                glUniform3f(fragmentColorLocation, cone.r, cone.g, cone.b);

            //Bind VAO
            glBindVertexArray(cone.VAO);

            //Draw cone
            glDrawArrays(GL_TRIANGLES, 0, cone.vertexCount);
        }


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// creates a cone triangle mesh, uploads it to openGL and returns the VAO associated to the mesh
SceneObject instantiateCone(float r, float g, float b, float offsetX, float offsetY){
    // TODO voronoi 1.1

    // Create an instance of a SceneObject,
    SceneObject sceneObject{};

    // you will need to store offsetX, offsetY, r, g and b in the object.
    sceneObject.r = r;
    sceneObject.g = g;
    sceneObject.b = b;
    sceneObject.x = offsetX;
    sceneObject.y = offsetY;

    // Build the geometry into an std::vector<float> or float array.
    vector<float> vertices;

    // Generate vertices for the cone
    int polygonSize = 256;
    int radius = 3.f;
    for(int i = 0; i < polygonSize; i++) {
        // Center
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);

        float angle = (float) i  / polygonSize * 360;

        vertices.push_back(cos(angleToRad(angle))*radius);
        vertices.push_back(sin(angleToRad(angle))*radius);
        vertices.push_back(0.0f);

        angle = (float) (i+1) / polygonSize * 360;
        if (angle == 360)
            angle = 0;

        vertices.push_back(cos(angleToRad(angle))*radius);
        vertices.push_back(sin(angleToRad(angle))*radius);
        vertices.push_back(0.0f);
    }

    // Store the number of vertices in the mesh in the scene object.
    sceneObject.vertexCount = vertices.size();

    // Declare and generate a VAO and VBO (and an EBO if you decide the work with indices).
    unsigned int VAO;
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    // Bind and set the VAO and VBO (and optionally a EBO) in the correct order.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
    glBindVertexArray(VAO);

    // Set the position attribute pointers in the shader.
    int posSize = 3;
    int posAttributeLocation = glGetAttribLocation(activeShader->ID, "aPos");
    glEnableVertexAttribArray(posAttributeLocation);
    glVertexAttribPointer(posAttributeLocation, posSize, GL_FLOAT, GL_FALSE, 0, 0);

    // Store the VAO handle in the scene object.
    sceneObject.VAO = VAO;

    // 'return' the scene object for the cone instance you just created.
    return sceneObject;
}

// glfw: called whenever a mouse button is pressed
void button_input_callback(GLFWwindow* window, int button, int action, int mods){
    // TODO voronoi 1.2
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        double mouseXPos, mouseYPos;
        int width, height;
        float offsetX, offsetY;
        glfwGetCursorPos(window, &mouseXPos, &mouseYPos);
        glfwGetWindowSize(window, &width, &height);
        offsetX = mouseXPos / width * 2 -1.f;
        offsetY = (mouseYPos / height * 2 -1.f) * -1;
        SceneObject cone = instantiateCone((float) rand()/RAND_MAX, (float) rand()/RAND_MAX, (float) rand()/RAND_MAX, offsetX, offsetY);
        sceneObjects.push_back(cone);
    }
}

// glfw: called whenever a keyboard key is pressed
void key_input_callback(GLFWwindow* window, int button, int other,int action, int mods){
    // TODO voronoi 1.4
    if(action == GLFW_PRESS){
        switch (button) {
            case GLFW_KEY_1:
                activeShader = &shaderPrograms[0];
                break;
            case GLFW_KEY_2:
                activeShader = &shaderPrograms[1];
                break;
            case GLFW_KEY_3:
                activeShader = &shaderPrograms[2];
                break;
        }
    }

}

float angleToRad(float angle){
    return angle*M_PI/180;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}