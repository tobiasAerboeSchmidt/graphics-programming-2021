#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>

#include <iostream>

#include <vector>
#include <chrono>

#include "shader.h"
#include "glmutils.h"

#include "primitives.h"

// application global variables
float lastX, lastY;                             // used to compute delta movement of the mouse
const unsigned int particlesCount = 10000;    // # of particles
const unsigned int particleSize = 3;            // particle attributes
const unsigned int sizeOfFloat = 4;             // bytes in a float
unsigned int particleId = 0;                    // keep track of last particle to be updated
float boxSize = 30; // Box size of 30m as defined in the paper

// Create offsets and offset deltas for each simulation
const unsigned int numberOfSimulations = 10;
std::vector<float> gravityOffsets(numberOfSimulations);
std::vector<float> xWindOffsets(numberOfSimulations);
std::vector<float> zWindOffsets(numberOfSimulations);
std::vector<float> gravityOffsetDeltas(numberOfSimulations);
std::vector<float> xWindOffsetDeltas(numberOfSimulations);
std::vector<float> zWindOffsetDeltas(numberOfSimulations);



struct SceneObject{
    unsigned int VAO;
    unsigned int vertexCount;
    void drawSceneObject() const{
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES,  vertexCount, GL_UNSIGNED_INT, 0);
    }
};

struct ParticlesObject{
    unsigned int VAO;
    unsigned int vertexCount;
    void drawParticlesObject() const{
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, vertexCount);
    }
};

// function declarations
// ---------------------
unsigned int createParticleVertexArray();
void initializeOffsetsAndOffsetDeltas();
unsigned int createArrayBuffer(const std::vector<float> &array);
unsigned int createElementArrayBuffer(const std::vector<unsigned int> &array);
unsigned int createVertexArray(const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<unsigned int> &indices);
float randBetween(float min, float max);
void setup();
void drawObjects();
void drawParticles();

// glfw and input functions
// ------------------------
void cursorInRange(float screenX, float screenY, int screenW, int screenH, float min, float max, float &x, float &y);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void cursor_input_callback(GLFWwindow* window, double posX, double posY);
void drawCube(glm::mat4 model);

// screen settings
// ---------------
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

// P

// global variables used for rendering
// -----------------------------------
SceneObject cube;
SceneObject floorObj;
ParticlesObject particlesObject;
Shader* shaderProgram;
Shader* particleShaderProgram;

// global variables used for control
// ---------------------------------
float currentTime;
glm::vec3 camForward(.0f, .0f, -1.0f);
glm::vec3 camPosition(.0f, 1.6f, 0.0f);
float linearSpeed = 0.15f, rotationGain = 30.0f;



int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Weather effects", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_input_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // setup mesh objects
    // ---------------------------------------
    setup();

    // set up the z-buffer
    // Notice that the depth range is now set to glDepthRange(-1,1), that is, a left handed coordinate system.
    // That is because the default openGL's NDC is in a left handed coordinate system (even though the default
    // glm and legacy openGL camera implementations expect the world to be in a right handed coordinate system);
    // so let's conform to that
    glDepthRange(-1,1); // make the NDC a LEFT handed coordinate system, with the camera pointing towards +z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // render loop
    // -----------
    // render every loopInterval seconds
    float loopInterval = 0.02f;
    auto begin = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(window))
    {
        // update current time
        auto frameStart = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> appTime = frameStart - begin;
        currentTime = appTime.count();

        processInput(window);

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

        // notice that we also need to clear the depth buffer (aka z-buffer) every new frame
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw objects
        // ------------
        shaderProgram->use();
        drawObjects();

        // Draw particles
        // --------------

        particleShaderProgram->use();
        particleShaderProgram->setVec3("camPosition", camPosition);
        particleShaderProgram->setVec3("forwardPosition", camPosition);
        particleShaderProgram->setFloat("boxSize", boxSize);

        drawParticles();

        glfwSwapBuffers(window);
        glfwPollEvents();

        // control render loop frequency
        std::chrono::duration<float> elapsed = std::chrono::high_resolution_clock::now()-frameStart;
        while (loopInterval > elapsed.count()) {
            elapsed = std::chrono::high_resolution_clock::now() - frameStart;
        }
    }

    delete shaderProgram;
    delete particleShaderProgram;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void drawObjects(){

    glm::mat4 scale = glm::scale(1.f, 1.f, 1.f);

    // NEW!
    // update the camera pose and projection, and compose the two into the viewProjection with a matrix multiplication
    // projection * view = world_to_view -> view_to_perspective_projection
    // or if we want ot match the multiplication order (projection * view), we could read
    // perspective_projection_from_view <- view_from_world
    glm::mat4 projection = glm::perspectiveFov(70.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, .01f, 100.0f);
    glm::mat4 view = glm::lookAt(camPosition, camPosition + camForward, glm::vec3(0,1,0));
    glm::mat4 viewProjection = projection * view;

    // draw floor (the floor was built so that it does not need to be transformed)
    shaderProgram->setMat4("model", viewProjection);
    floorObj.drawSceneObject();

    // draw a cube
    drawCube(viewProjection * glm::translate(0.0f, 1.f, -4.0f) * glm::rotateY(glm::half_pi<float>()) * scale);
}

void drawParticles(){

    glm::mat4 projectionMatrix = glm::perspectiveFov(70.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, .01f, 100.0f);
    glm::mat4 viewMatrix = glm::lookAt(camPosition, camPosition + camForward, glm::vec3(0,1,0));
    glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

    // draw floor (the floor was built so that it does not need to be transformed)
    particleShaderProgram->setMat4("model", viewProjectionMatrix);

    for(int i = 0; i < numberOfSimulations; i++){
        // Update and calculate offset
        gravityOffsets[i] += gravityOffsetDeltas[i];
        xWindOffsets[i] += xWindOffsetDeltas[i];
        zWindOffsets[i] += zWindOffsetDeltas[i];
        glm::vec3 combinedOffset = glm::vec3(xWindOffsets[i], -gravityOffsets[i], zWindOffsets[i]);
        combinedOffset -= camPosition + camForward + (boxSize/2);
        combinedOffset = glm::mod(combinedOffset, boxSize);

        particleShaderProgram->setVec3("combinedOffset", combinedOffset);

        particlesObject.drawParticlesObject();
    }
}


void drawCube(glm::mat4 model){
    // draw object
    shaderProgram->setMat4("model", model);
    cube.drawSceneObject();
}

float randBetween(float min, float max){
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = max - min;
    float r = random * diff;
    return min + r;
}


void setup(){
    // initialize object shader
    //-------------------
    shaderProgram = new Shader("shaders/objectShader.vert", "shaders/objectShader.frag");

    // load floor mesh into openGL
    floorObj.VAO = createVertexArray(floorVertices, floorColors, floorIndices);
    floorObj.vertexCount = floorIndices.size();

    // load cube mesh into openGL
    cube.VAO = createVertexArray(cubeVertices, cubeColors, cubeIndices);
    cube.vertexCount = cubeIndices.size();

    // initialize object shader
    //-------------------
    particleShaderProgram = new Shader("shaders/particleShader.vert", "shaders/particleShader.frag");
    particlesObject.VAO = createParticleVertexArray();
    initializeOffsetsAndOffsetDeltas();
}

void initializeOffsetsAndOffsetDeltas(){
    //Initialize offset for each simulation
    for(int i = 0; i < numberOfSimulations; i++){
        // Each simulation should begin with an offset of 0
        xWindOffsets[i] = 0.0f;
        zWindOffsets[i] = 0.0f;
        gravityOffsets[i] = 0.0f;
        // Each simulation should change with a random amount
        xWindOffsetDeltas[i] = randBetween(.05, .1);
        xWindOffsetDeltas[i] = randBetween(.05, .1);
        gravityOffsetDeltas[i] = randBetween(.1, .3);
    }
}

unsigned int createParticleVertexArray() {
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // initialize particle buffer, set all values to 0
    std::vector<float> data(particlesCount * particleSize);
    for (unsigned int i = 0; i < data.size(); i += particleSize){
        float x = randBetween(-15, 15);
        float y = randBetween(-15, 15);
        float z = randBetween(-15, 15);
        data[i] = x;
        data[i+1] = y;
        data[i+2] = z;
    }
    // Update how many vertices should be drawn
    particlesObject.vertexCount = data.size();

    // allocate at openGL controlled memory
    glBufferData(GL_ARRAY_BUFFER, particlesCount * particleSize * sizeOfFloat, &data[0], GL_DYNAMIC_DRAW);

    // Bind position attribute
    int posSize = 3; // each position has x,y,z
    GLuint vertexPositionLocation = glGetAttribLocation(particleShaderProgram->ID, "pos");
    glEnableVertexAttribArray(vertexPositionLocation);
    glVertexAttribPointer(vertexPositionLocation, posSize, GL_FLOAT, GL_FALSE, 0, 0);

    return VAO;
}


unsigned int createVertexArray(const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<unsigned int> &indices){
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    // bind vertex array object
    glBindVertexArray(VAO);

    // set vertex shader attribute "pos"
    createArrayBuffer(positions); // creates and bind  the VBO
    int posAttributeLocation = glGetAttribLocation(shaderProgram->ID, "pos");
    glEnableVertexAttribArray(posAttributeLocation);
    glVertexAttribPointer(posAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // set vertex shader attribute "color"
    createArrayBuffer(colors); // creates and bind the VBO
    int colorAttributeLocation = glGetAttribLocation(shaderProgram->ID, "color");
    glEnableVertexAttribArray(colorAttributeLocation);
    glVertexAttribPointer(colorAttributeLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // creates and bind the EBO
    createElementArrayBuffer(indices);

    return VAO;
}


unsigned int createArrayBuffer(const std::vector<float> &array){
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, array.size() * sizeof(GLfloat), &array[0], GL_STATIC_DRAW);


    return VBO;
}


unsigned int createElementArrayBuffer(const std::vector<unsigned int> &array){
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, array.size() * sizeof(unsigned int), &array[0], GL_STATIC_DRAW);

    return EBO;
}

// NEW!
// instead of using the NDC to transform from screen space you can now define the range using the
// min and max parameters
void cursorInRange(float screenX, float screenY, int screenW, int screenH, float min, float max, float &x, float &y){
    float sum = max - min;
    float xInRange = (float) screenX / (float) screenW * sum - sum/2.0f;
    float yInRange = (float) screenY / (float) screenH * sum - sum/2.0f;
    x = xInRange;
    y = -yInRange; // flip screen space y axis
}

void cursor_input_callback(GLFWwindow* window, double posX, double posY){

    int screenW, screenH;
    glfwGetWindowSize(window, &screenW, &screenH);
    glm::vec2 cursorPosition(0.0f);
    cursorInRange(posX, posY, screenW, screenH, -1.0f, 1.0f, cursorPosition.x, cursorPosition.y);

    // initialize with first value so that there is no jump at startup
    static glm::vec2 lastCursorPosition = cursorPosition;

    // compute the cursor position change
    glm::vec2 positionDiff = cursorPosition - lastCursorPosition;

    static float rotationAroundVertical = 0;
    static float rotationAroundLateral = 0;

    // require a minimum threshold to rotate
    if (glm::dot(positionDiff, positionDiff) > 1e-5f){
        camForward = glm::vec3 (0,0,-1);
        // rotate the forward vector around the Y axis, notices that w is set to 0 since it is a vector
        rotationAroundVertical += glm::radians(-positionDiff.x * rotationGain);
        camForward = glm::rotateY(rotationAroundVertical) * glm::vec4(camForward, 0.0f);

        // rotate the forward vector around the lateral axis
        rotationAroundLateral +=  glm::radians(positionDiff.y * rotationGain);
        // we need to clamp the range of the rotation, otherwise forward and Y axes get parallel
        rotationAroundLateral = glm::clamp(rotationAroundLateral, -glm::half_pi<float>() * 0.9f, glm::half_pi<float>() * 0.9f);

        glm::vec3 lateralAxis = glm::cross(camForward, glm::vec3(0, 1,0));

        camForward = glm::rotate(rotationAroundLateral, lateralAxis) * glm::vec4(camForward, 0);

        lastCursorPosition = cursorPosition;
    }

}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glm::vec3 forward = glm::normalize(glm::vec3(camForward.x, 0, camForward.z));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camPosition += forward * linearSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camPosition -= glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)) * linearSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camPosition -= forward * linearSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camPosition += glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)) * linearSpeed;

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}