#ifndef OBJOBJECT_H
#define OBJOBJECT_H

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#else
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

class OBJObject
{
private:
    glm::mat4 toWorld;
    glm::vec3 position;
    glm::vec3 originalPos;
    glm::vec4 dirLightPos;
    glm::vec3 pointPos;
    bool pointPosSet = false;
    glm::vec4 spotPos;
    bool spotPosSet = false;
    bool posSet;
    bool centerScale;
    
    float angle;
    float dlAngle;
    char rotation;
    float scaleFactor;
    float pointSize;
    float coneAngle;
    float coneSharpness;
    
public:
    // These variables are needed for the shader program
    GLuint VBO, VAO, EBO, VBO_Normal, EBO_Normal;
    GLuint uProjection, uModel, uView, uColor, uViewPos;
    
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> fVertices;
    std::vector<glm::vec3> fNormals;
    std::vector<unsigned int> eboData;
    
	OBJObject(const char* filepath);
	void parse(const char* filepath);
    void draw(GLuint shaderProgram, bool normalColor, char material, bool dirLight, bool pointLight, bool spotLight, bool sphere, bool cone);
    
    void spin(float deg, char axis);
    
    void centerScaleObject();
    void translateX(int key);
    void translateY(int key);
    void translateZ(char key);
    void translateXY(float x, float y);
    void scalePoint(char key);
    void scale(char key);
    void resetPosition();
    void reset();
    void trackBallRotation(glm::vec3 v1, glm::vec3 v2);
    void lightRotation(GLuint shaderProgram);
    void changeSpotWidth(GLuint shaderProgram, char w);
    void changeSpotEdge(GLuint shaderProgram, char e);
};

#endif
