#include "OBJObject.h"
#include "Window.h"
#include <iostream>

OBJObject::OBJObject(const char *filepath) 
{
    posSet = false;
    centerScale = false;
    angle = 0.0f;
    dlAngle = 0.0f;
    pointSize = 1.0f;
    scaleFactor = 1.0f;

    this->coneAngle = 60.0f;
    this->coneSharpness = 1.0f;
    
    dirLightPos = glm::vec4 (1.0f, 0.0f, 0.0f, 1.0f);
    spotPos = glm::vec4 (0.0f, 6.0f, 0.0f, 1.0f);
    pointPos = glm::vec3 (0.0f, 6.0f, 0.0f);
    
	toWorld = glm::mat4(1.0f);
	parse(filepath);
    
    // Create array object and buffers. Remember to delete your buffers when the object is destroyed!
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &VBO_Normal);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &EBO_Normal);
    
    // Bind the Vertex Array Object (VAO) first, then bind the associated buffers to it.
    // Consider the VAO as a container for all your buffers.
    glBindVertexArray(VAO);
    
    // Now bind a VBO to it as a GL_ARRAY_BUFFER. The GL_ARRAY_BUFFER is an array containing relevant data to what you want to draw, such as vertices, normals, colors, etc.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    
    std::vector<int> fVerts;
    std::vector<int> fNorms;
    //std::
    for (int i = 0; i < fVertices.size(); i++)
    {
        fVerts.push_back(fVertices[i].x);
        fVerts.push_back(fVertices[i].y);
        fVerts.push_back(fVertices[i].z);
        fNorms.push_back(fNormals[i].x);
        fNorms.push_back(fNormals[i].y);
        fNorms.push_back(fNormals[i].z);
    }
    
    // glBufferData populates the most recently bound buffer with data starting at the 3rd argument and ending after the 2nd argument number of indices. How does OpenGL know how long an index spans? Go to glVertexAttribPointer.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    
    // Enable the usage of layout location 0 (check the vertex shader to see what this is)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Normal);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    
    // Enable the usage of layout location 0 (check the vertex shader to see what this is)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    // We've sent the vertex data over to OpenGL, but there's still something missing.
    // In what order should it draw those vertices? That's why we'll need a GL_ELEMENT_ARRAY_BUFFER for this.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, fVerts.size() * sizeof(int), fVerts.data(), GL_STATIC_DRAW);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Normal);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, fNorms.size() * sizeof(int), fNorms.data(), GL_STATIC_DRAW);
    
    // Unbind the currently bound buffer so that we don't accidentally make unwanted changes to it.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Unbind the VAO now so we don't accidentally tamper with it.
    // NOTE: You must NEVER unbind the element array buffer associated with a VAO!
    glBindVertexArray(0);
}

// Parses the OBJ file, populating the face indices, vertices, and normals vectors with the OBJ Object data
void OBJObject::parse(const char *filepath)
{
    FILE *fp; //File pointer
    float x, y, z; //Certex coordinates
    float r, g, b; //Vertex color
    int c1, c2; //Characters read from file
    
    fp = fopen(filepath, "r");
    if (!fp)
    {
        std::cerr << "Error loading file." << std::endl;
        exit(-1);
    }
    
    while (true)
    {
        c1 = fgetc(fp);
        if (c1 == EOF)
        {
            break;
        }
        c2 = fgetc(fp);
        
        if (c1 == 'v' && c2 == ' ') //Vertex
        {
            fscanf(fp, "%f %f %f %f %f %f\r\n", &x, &y, &z, &r, &g, &b);
            glm::vec3 v(x, y, z);
            vertices.push_back(v);
        }
        
        else if (c1 == 'v' && c2 == 'n') //Vertex normal
        {
            fscanf(fp, "%f %f %f\r\n", &x, &y, &z);
            glm::vec3 n(x, y, z);
            normals.push_back(n);
        }
        
        else if (c1 == 'f' && c2 == ' ') //Face vertices
        {
            fscanf(fp, "%f//", &x);
            fscanf(fp, "%f ", &r);
            fscanf(fp, "%f//", &y);
            fscanf(fp, "%f ", &g);
            fscanf(fp, "%f//", &z);
            fscanf(fp, "%f \r\n", &b);
            glm::vec3 v(x - 1, y - 1, z - 1);
            glm::vec3 n(r - 1, g - 1, b - 1);
            fVertices.push_back(v);
            fNormals.push_back(n);
        }
        
        else
        {
            fscanf(fp, "\r\n");
        }
    }
    
    fclose(fp);
}

void OBJObject::draw(GLuint shaderProgram, bool normalColor, char material, bool dirLight, bool pointLight, bool spotLight, bool sphere, bool cone)
{
	originalPos.x, originalPos.y, originalPos.z = 0.0f;
    if (!posSet)
    {
        position = originalPos;
        posSet = true;
    }
    
    // Calculate the combination of the model and view (camera inverse) matrices
    glm::mat4 modelview = Window::V * toWorld;
    // We need to calculate this because modern OpenGL does not keep track of any matrix other than the viewport (D)
    // Consequently, we need to forward the projection, view, and model matrices to the shader programs
    // Get the location of the uniform variables "projection" and "modelview"
    uProjection = glGetUniformLocation(shaderProgram, "projection");
    uModel = glGetUniformLocation(shaderProgram, "model");
    uView = glGetUniformLocation(shaderProgram, "view");
    uColor = glGetUniformLocation(shaderProgram, "color");
    uViewPos = glGetUniformLocation(shaderProgram, "viewPos");
    
    // Now send these values to the shader program
    glUniformMatrix4fv(uProjection, 1, GL_FALSE, &Window::P[0][0]);
    glUniformMatrix4fv(uModel, 1, GL_FALSE, &toWorld[0][0]);
    glUniformMatrix4fv(uView, 1, GL_FALSE, &Window::V[0][0]);
    glUniform3f(uViewPos, 0.0f, 0.0f, 20.0f);
    
    // Now draw the cube. We simply need to bind the VAO associated with it.
    glBindVertexArray(VAO);
    // Tell OpenGL to draw with triangles, using 36 indices, the type of the indices, and the offset to start from
    glDrawElements(GL_TRIANGLES, sizeof(glm::vec3) * vertices.size(), GL_UNSIGNED_INT, 0);
    // Unbind the VAO when we're done so we don't accidentally draw extra stuff or tamper with its bound buffers
    glBindVertexArray(0);
    
    if (!centerScale)
        centerScaleObject();

    //glUseProgram(shaderProgram);
    
    glUniform1i(glGetUniformLocation(shaderProgram, "normalColor"), normalColor);
    
    if (cone)
    {
        glUniform3f(glGetUniformLocation(shaderProgram, "material.specular"), 0.1f, 0.1f, 1.0f);
         glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuse"), 1.0f, 1.0f, 1.0f);
         glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), 0, 0.5f, 0);
         glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), 1.0f);
        
        if (!spotPosSet)
        {
            position = spotPos;
            this->toWorld = glm::translate(glm::mat4(1.0f), position) * toWorld;
            scale('s');
            scale('s');
            spotPosSet = true;
        }
    }
    
    else if (sphere)
    {
        glUniform3f(glGetUniformLocation(shaderProgram, "material.specular"), 0, 0, 1.0f);
         glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuse"), 1.0f, 1.0f, 1.0f);
         glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), 0, 0.5f, 0);
         glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), 1.0f);
        
        if (!pointPosSet)
        {
            position = pointPos;
            this->toWorld = glm::translate(glm::mat4(1.0f), position) * toWorld;
            scale('s');
            scale('s');
            pointPosSet = true;
        }
    }
    
    else if (material == 's')
    {
        glUniform3f(glGetUniformLocation(shaderProgram, "material.specular"), .3f, .3f, .3f);
        glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuse"), 0.0, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), .192f, .192f, .192f);
        glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), 0.088f);
    }
    
    else if (material == 'd')
    {
        glUniform3f(glGetUniformLocation(shaderProgram, "material.specular"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuse"), 0.7f, 0.7f, 0.7);
        glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), .05f, .05f, .05f);
        glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), 0.0f);
    }
    
    else if (material == 'b')
    {
        glUniform3f(glGetUniformLocation(shaderProgram, "material.specular"), 0.33f, 0.33f, 0.33f);
        glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuse"), 0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), .192f, .192f, .192f);
        glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), 0.5f);
    }
    
    if (!cone && !sphere)
    {
        //Directional Light
        glUniform1i(glGetUniformLocation(shaderProgram, "dirLight.on"), dirLight);
        glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.direction"), dirLightPos.x, dirLightPos.y, dirLightPos.z);
        //glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.color"), 0.86f, 0.08f, 0.24f);
        glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.ambient"), 0.3f, 0.24f, 0.14f);
        glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.diffuse"), 0.7f, 0.42f, 0.26f);
        glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.specular"), 0.5f, 0.5f, 0.5f);
        
        //Point Light
        glUniform1i(glGetUniformLocation(shaderProgram, "pointLight.on"), pointLight);
        glm::vec3 pointLightColors = glm::vec3(1.0f, 0.6f, 0.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "pointLight.position"), pointPos.x, pointPos.y - 4.0f, pointPos.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "pointLight.ambient"), 0.3f, 0.24f, 0.14f);
        glUniform3f(glGetUniformLocation(shaderProgram, "pointLight.diffuse"), 0.7f, 0.42f, 0.26f);
        glUniform3f(glGetUniformLocation(shaderProgram, "pointLight.specular"), 0.5f, 0.5f, 0.5f);
        glUniform1f(glGetUniformLocation(shaderProgram, "pointLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(shaderProgram, "pointLight.linear"), 0.09);
        glUniform1f(glGetUniformLocation(shaderProgram, "pointLight.quadratic"), 0.032);
        
        //Spot Light
        glUniform1i(glGetUniformLocation(shaderProgram, "spotLight.on"), spotLight);
        glUniform4f(glGetUniformLocation(shaderProgram, "spotLight.position"), spotPos.x, spotPos.y - 4.0f, spotPos.z, spotPos.w);
        glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.intensities"), 2.0f, 2.0f, 2.0f); //strong white light
        glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.attenuation"), 0.5f);
        glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.ambientCoefficient"), 0.0f);
        glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.coneAngle"), this->coneAngle);
        glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.coneDirection"), 0.0f, 0.0f, this->coneSharpness);
    }
}

void OBJObject::centerScaleObject()
{
    glm::vec3 max(vertices[0].x, vertices[0].y, vertices[0].z);
    glm::vec3 min(vertices[0].x, vertices[0].y, vertices[0].z);
    
    for (int i = 1; i < vertices.size(); i++)
    {
        if (vertices[i].x > max.x)
        {
            max.x = vertices[i].x;
        }
        if (vertices[i].x < min.x)
        {
            min.x = vertices[i].x;
        }
        
        if (vertices[i].y > max.y)
        {
            max.y = vertices[i].y;
        }
        if (vertices[i].y < min.y)
        {
            min.y = vertices[i].y;
        }
        
        if (vertices[i].z > max.z)
        {
            max.z = vertices[i].z;
        }
        if (vertices[i].z < min.z)
        {
            min.z = vertices[i].z;
        }
    }
    
    glm::vec3 center = (max + min) / 2.0f;
    this->toWorld = glm::translate(glm::mat4(1.0f), position * -1.0f) * toWorld; //Translate to origin
    position = center;
    originalPos = center;
    this->toWorld = glm::translate(glm::mat4(1.0f), center) * toWorld;
    
    float xDim = max.x - min.x;
    float yDim = max.y - min.y;
    float zDim = max.z - min.z;
    float longestDim = xDim;
    if (yDim > longestDim)
        longestDim = yDim;
    if (zDim > longestDim)
        longestDim = zDim;
    
    float ratio = 2.0f / longestDim;
    this->toWorld = glm::translate(glm::mat4(1.0f), position * -1.0f) * toWorld;
    this->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(ratio, ratio, ratio)) * toWorld;
    this->toWorld = glm::translate(glm::mat4(1.0f), position) * toWorld;
    
    scaleFactor *= ratio;
    centerScale = true;
}

void OBJObject::spin(float deg, char axis)
{
    this->toWorld = glm::translate(glm::mat4(1.0f), position * -1.0f) * toWorld; //Translate to origin
    angle += deg / 180.0f * glm::pi<float>(); //Update angle
    if (angle > 360.0f)
    {
        angle = angle - 360.0f;
    }
    
    //Creates matrix to rotate
    if (axis == 'x')
    {
        this->toWorld = glm::rotate(glm::mat4(1.0f), deg / 180.0f * glm::pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)) * toWorld;
        rotation = 'x';
    }
    else if (axis == 'y')
    {
        this->toWorld = glm::rotate(glm::mat4(1.0f), deg / 180.0f * glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f)) * toWorld;
        rotation = 'y';
    }
    else
    {
        this->toWorld = glm::rotate(glm::mat4(1.0f), deg / 180.0f * glm::pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f)) * toWorld;
        rotation = 'z';
    }
    this->toWorld = glm::translate(glm::mat4(1.0f), position) * toWorld; //Translate back
}

void OBJObject::translateX(int key)
{
    if (key == GLFW_KEY_D) //Right
    {
        position.x += 1.0f;
        this->toWorld = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * toWorld;
    }
    else //Left
    {
        position.x -= 1.0f;
        this->toWorld = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)) * toWorld;
    }
}

void OBJObject::translateY(int key)
{
    if (key == GLFW_KEY_W) //Up
    {
        position.y += 1.0f;
        this->toWorld = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * toWorld;
    }
    else //Down
    {
        position.y -= 1.0f;
        this->toWorld = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f)) * toWorld;
    }
}

void OBJObject::translateZ(char key)
{
    if (key == 'Z') //Out of screen
    {
        position.z += 1.0f;
        this->toWorld = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * toWorld;
    }
    else //Into screen
    {
        position.z -= 1.0f;
        this->toWorld = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f)) * toWorld;
    }
}

void OBJObject::translateXY(float x, float y)
{
    this->toWorld = glm::translate(glm::mat4(1.0f), glm::vec3(x - position.x, y - position.y, 0.0f)) * toWorld;
    position.x = x;
    position.y = y;
}

void OBJObject::scalePoint(char key) //Scale point size
{
    glMatrixMode(GL_MODELVIEW);
    glMultMatrixf(&(toWorld[0][0]));
    if (key == 'P')
    {
        pointSize *= 1.3f;
    }
    else
    {
        pointSize *= 0.75f;
    }
    
    // Loop through all the vertices of this OBJ Object and change their point size
    for (unsigned int i = 0; i < vertices.size(); ++i)
    {
        glPointSize(pointSize);
    }
}

void OBJObject::scale(char key) //Scale object
{
    if (key == 'S') //Scale up
    {
        this->toWorld = glm::translate(glm::mat4(1.0f), position * -1.0f) * toWorld;
        this->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(1.3f, 1.3f, 1.3f)) * toWorld;
        this->toWorld = glm::translate(glm::mat4(1.0f), position) * toWorld;
        scaleFactor *= 1.3f;
    }
    else //Scale down
    {
        this->toWorld = glm::translate(glm::mat4(1.0f), position * -1.0f) * toWorld;
        this->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(0.75f, 0.75f, 0.75f)) * toWorld;
        this->toWorld = glm::translate(glm::mat4(1.0f), position) * toWorld;
        scaleFactor *= 0.75f;
    }
}

void OBJObject::resetPosition()
{
    glm::vec3 translation = originalPos - position;
    this->toWorld = glm::translate(glm::mat4(1.0f), translation) * toWorld;
    position = originalPos;
}

void OBJObject::reset()
{
    this->toWorld = glm::translate(glm::mat4(1.0f), position * -1.0f) * toWorld; //Translate to origin
    
    //Reset scale
    this->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f/scaleFactor, 1.0f/scaleFactor, 1.0f/scaleFactor)) * toWorld;
    
    //Reset orientation
    if (rotation == 'x')
        this->toWorld = glm::rotate(glm::mat4(1.0f), -1.0f * angle, glm::vec3(1.0f, 0.0f, 0.0f)) * toWorld;
    else if (rotation == 'y')
        this->toWorld = glm::rotate(glm::mat4(1.0f), -1.0f * angle, glm::vec3(0.0f, 1.0f, 0.0f)) * toWorld;
    else if (rotation == 'z')
        this->toWorld = glm::rotate(glm::mat4(1.0f), -1.0f * angle, glm::vec3(0.0f, 0.0f, 1.0f)) * toWorld;
    
    this->toWorld = glm::translate(glm::mat4(1.0f), position) * toWorld; //Translate back
    scaleFactor = 1.0f;
    angle = 0.0f;
}

void OBJObject::trackBallRotation(glm::vec3 v1, glm::vec3 v2)
{
    glm::vec3 begin (v1.x, v1.y, v1.z);
    glm::vec3 end (v2.x, v2.y, v2.z);
    glm::vec3 axis = glm::cross(end, begin);
    float angle = glm::dot(begin, end) / (begin.length() * end.length()); /// 180.0f * glm::pi<float>();
    angle = acos(angle);
    //std::cout << "begin " << begin.x << " " << begin.y << " " << begin.z << std::endl;
    //std::cout << "end " << end.x << " " << end.y << " " << end.z << std::endl;
    //std::cout << "axis " << axis.x << " " << axis.y << " " << axis.z << std::endl;
    
    this->toWorld = glm::translate(glm::mat4(1.0f), position * -1.0f) * toWorld; //Translate to origin
    this->toWorld = glm::rotate(glm::mat4(1.0f), angle / 180.0f * glm::pi<float>(), glm::vec3(axis.x, axis.y, axis.z)) * this->toWorld;
    this->toWorld = glm::translate(glm::mat4(1.0f), position) * toWorld; //Translate back
}

void OBJObject::lightRotation(GLuint shaderProgram)
{
    dlAngle += 1.0f / 180.0f * glm::pi<float>(); //Update angle
    if (dlAngle > 360.0f)
    {
        dlAngle -= 360.0f;
    }
    
    dirLightPos = glm::rotate(glm::mat4(1.0f), 1.0f / 180.0f * glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f)) * dirLightPos;
    glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.direction"), dirLightPos.x, dirLightPos.y, dirLightPos.z);
}

void OBJObject::changeSpotWidth(GLuint shaderProgram, char w)
{
    if (w == 'w')
        coneAngle += 1.0f;
    else
        coneAngle -= 1.0f;
    
    glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.coneAngle"), coneAngle);
}

void OBJObject::changeSpotEdge(GLuint shaderProgram, char e)
{
    if (e == 'e')
        coneSharpness -= 0.5f;
    else
        coneSharpness += 0.5f;
    //glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.coneDirection"), 0.0f, 0.0f, coneSharpness);
}
