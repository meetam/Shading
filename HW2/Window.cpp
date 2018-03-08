#include "Window.h"
#include "OBJObject.h"

const char* window_title = "GLFW Starter Project";
Cube * cube;
OBJObject * dragon;
OBJObject * bunny;
OBJObject * bear;
OBJObject * sphere;
OBJObject * cone;
bool dragonMode = false;
bool bunnyMode = true;
bool bearMode = false;
bool normalColor = true;
bool dirLightMode = false;
bool pointLightMode = false;
bool spotLightMode = false;
GLint shaderProgram;
GLint lightShaderProgram;

// On some systems you need to change this to the absolute path
#define VERTEX_SHADER_PATH "/Users/Meeta/Desktop/CSE 167/HW2/HW2/shader.vert"
#define FRAGMENT_SHADER_PATH "/Users/Meeta/Desktop/CSE 167/HW2/HW2/shader.frag"
#define LIGHT_VERTEX_SHADER_PATH "/Users/Meeta/Desktop/CSE 167/HW2/HW2/lightShader.vert"
#define LIGHT_FRAGMENT_SHADER_PATH "/Users/Meeta/Desktop/CSE 167/HW2/HW2/lightShader.frag"

// Default camera parameters
glm::vec3 cam_pos(0.0f, 0.0f, 20.0f);		// e  | Position of camera
glm::vec3 cam_look_at(0.0f, 0.0f, 0.0f);	// d  | This is where the camera looks at
glm::vec3 cam_up(0.0f, 1.0f, 0.0f);			// up | What orientation "up" is

int Window::width;
int Window::height;

glm::mat4 Window::P;
glm::mat4 Window::V;

void Window::initialize_objects()
{
	cube = new Cube();
    dragon = new OBJObject("/Users/Meeta/Desktop/CSE 167/HW2/HW2/dragon.obj");
    bunny = new OBJObject("/Users/Meeta/Desktop/CSE 167/HW2/HW2/bunny.obj");
    bear = new OBJObject("/Users/Meeta/Desktop/CSE 167/HW2/HW2/bear.obj");
    sphere = new OBJObject("/Users/Meeta/Desktop/CSE 167/HW2/HW2/sphere.obj");
    cone = new OBJObject("/Users/Meeta/Desktop/CSE 167/HW2/HW2/cone.obj");
	// Load the shader program. Make sure you have the correct filepath up top
	shaderProgram = LoadShaders(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);
    lightShaderProgram = LoadShaders(LIGHT_VERTEX_SHADER_PATH, LIGHT_FRAGMENT_SHADER_PATH);
}

// Treat this as a destructor function. Delete dynamically allocated memory here.
void Window::clean_up()
{
	delete(cube);
	glDeleteProgram(shaderProgram);
}

GLFWwindow* Window::create_window(int width, int height)
{
	// Initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return NULL;
	}

	// 4x antialiasing
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__ // Because Apple hates comforming to standards
	// Ensure that minimum OpenGL version is 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Enable forward compatibility and allow a modern OpenGL context
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Create the GLFW window
	GLFWwindow* window = glfwCreateWindow(width, height, window_title, NULL, NULL);

	// Check if the window could not be created
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		fprintf(stderr, "Either GLFW is not installed or your graphics card does not support modern OpenGL.\n");
		glfwTerminate();
		return NULL;
	}

	// Make the context of the window
	glfwMakeContextCurrent(window);

	// Set swap interval to 1
	glfwSwapInterval(1);

	// Get the width and height of the framebuffer to properly resize the window
	glfwGetFramebufferSize(window, &width, &height);
	// Call the resize callback to make sure things get drawn immediately
	Window::resize_callback(window, width, height);

	return window;
}

void Window::resize_callback(GLFWwindow* window, int width, int height)
{
#ifdef __APPLE__
	glfwGetFramebufferSize(window, &width, &height); // In case your Mac has a retina display
#endif
	Window::width = width;
	Window::height = height;
	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);

	if (height > 0)
	{
		P = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
		V = glm::lookAt(cam_pos, cam_look_at, cam_up);
	}
}

void Window::idle_callback()
{
	// Call the update function the cube
	//cube->update();
    
    if (dirLightMode)
    {
        if (dragonMode)
            dragon->lightRotation(shaderProgram);
        if (bunnyMode)
            bunny->lightRotation(shaderProgram);
        if (bearMode)
            bear->lightRotation(shaderProgram);
    }
}

void Window::display_callback(GLFWwindow* window)
{
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use the shader of programID
	glUseProgram(shaderProgram);
	
	// Render the cube
	//cube->draw(shaderProgram);
    if (dragonMode)
        dragon->draw(shaderProgram, normalColor, 's', dirLightMode, pointLightMode, spotLightMode, false, false);
    else if (bunnyMode)
        bunny->draw(shaderProgram, normalColor, 'd', dirLightMode, pointLightMode, spotLightMode, false, false);
    else if (bearMode)
        bear->draw(shaderProgram, normalColor, 'b', dirLightMode, pointLightMode, spotLightMode, false, false);
    
    glUseProgram(lightShaderProgram);
    if (pointLightMode)
        sphere->draw(lightShaderProgram, normalColor, 'n', false, false, false, true, false);
    else if (spotLightMode)
        cone->draw(lightShaderProgram, normalColor, 'n', false, false, false, false, true);

	// Gets events, including input such as keyboard and mouse or window resizing
	glfwPollEvents();
	// Swap buffers
	glfwSwapBuffers(window);
}

bool rDown = false;
bool lDown = false;

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Check for a key press
	if (action == GLFW_PRESS)
	{
		// Check if escape was pressed
		if (key == GLFW_KEY_ESCAPE)
		{
			// Close the window. This causes the program to also terminate.
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
        
        if (key == GLFW_KEY_F1) //Switch to dragon
        {
            bunnyMode = false;
            bearMode = false;
            dragonMode = true;
        }
        
        if (key == GLFW_KEY_F2) //Switch to bunny
        {
            dragonMode = false;
            bearMode = false;
            bunnyMode = true;
        }
        
        if (key == GLFW_KEY_F3) //Switch to bear
        {
            bunnyMode = false;
            dragonMode = false;
            bearMode = true;
        }
        
        if (key == GLFW_KEY_S) //Scale
        {
            char s = 's';
            if (mods == GLFW_MOD_SHIFT)
                s = 'S';
            if (bunnyMode)
                bunny->scale(s);
            if (dragonMode)
                dragon->scale(s);
            if (bearMode)
                bear->scale(s);
        }
        
        if (key == GLFW_KEY_W) //Spot light wider/narrower
        {
            char w = 'w';
            if (mods == GLFW_MOD_SHIFT)
                w = 'W';
            if (bunnyMode)
                bunny->changeSpotWidth(shaderProgram, w);
            if (dragonMode)
                dragon->changeSpotWidth(shaderProgram, w);
            if (bearMode)
                bear->changeSpotWidth(shaderProgram, w);
        }
        
        if (key == GLFW_KEY_E) //Spot edge sharper/blurrier
        {
            char e = 'e';
            if (mods == GLFW_MOD_SHIFT)
                e = 'E';
            if (bunnyMode)
                bunny->changeSpotEdge(shaderProgram, e);
            if (dragonMode)
                dragon->changeSpotEdge(shaderProgram, e);
            if (bearMode)
                bear->changeSpotEdge(shaderProgram, e);
        }
        
        if (key == GLFW_KEY_N) //Switch coloring mode
        {
            normalColor = !normalColor;
        }
        
        if (key == GLFW_KEY_0) //Scene 0
        {
            dirLightMode = false;
            spotLightMode = false;
            pointLightMode = false;
        }
        
        if (key == GLFW_KEY_1) //Scene 1
        {
            dirLightMode = true;
            spotLightMode = false;
            pointLightMode = false;
        }
        
        if (key == GLFW_KEY_2) //Scene 2
        {
            dirLightMode = true;
            spotLightMode = false;
            pointLightMode = true;
        }
        
        if (key == GLFW_KEY_3) //Scene 3
        {
            dirLightMode = true;
            spotLightMode = true;
            pointLightMode = false;
        }
        
        if (key == GLFW_KEY_4) //Toggle directional light on/off
        {
            dirLightMode = !dirLightMode;
        }
	}
    
    // Check for control button
    if (key == GLFW_KEY_LEFT_CONTROL)
    {
        if (action == GLFW_PRESS)
            rDown = true;
        if (action == GLFW_RELEASE)
            rDown = false;
    }
}

double xPos1;
double yPos1;
void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            lDown = true;
            glfwGetCursorPos(window, &xPos1, &yPos1);
        }
        if (action == GLFW_RELEASE)
        {
            lDown = false;
        }
    }
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset < 0)
    {
        if (dragonMode)
            dragon->translateZ('Z');
        if (bunnyMode)
            bunny->translateZ('Z');
        if (bearMode)
            bear->translateZ('Z');
    }
    
    else if (yoffset > 0)
    {
        if (dragonMode)
            dragon->translateZ('z');
        if (bunnyMode)
            bunny->translateZ('z');
        if (bearMode)
            bear->translateZ('z');
    }
}

void Window::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    float xPos = xpos;
    float yPos = ypos;
    
    if (rDown)
    {
        xPos = xPos - (float)width / 2.0f;
        yPos = (float)height / 2.0f - yPos;
        xPos /= 24.0f;
        yPos /= 24.0f;
        
        if (dragonMode)
            dragon->translateXY(xPos, yPos);
        if (bunnyMode)
            bunny->translateXY(xPos, yPos);
        if (bearMode)
            bear->translateXY(xPos, yPos);
    }
    
    if (lDown)
    {
        std::cout <<"2D 1: " << xPos1 << " " << yPos1 << std::endl;
        std::cout << "2D 2: " << xPos << " " << yPos << std::endl;
        glm::vec3 v1;
        
        v1.x = (2.0f * (float)xPos1 - width) / width;
        v1.y = (height - 2.0f * (float)yPos1) / height;
        v1.z = 0.0f;
        //std::cout << v1.x << " " << v1.y << " " << v1.z << std::endl;
        float d = v1.length();//glm::length(v1);
        d = (d < 1.0f) ? d : 1.0f;
        v1.z = sqrtf(1.001f - d*d);
        v1 = glm::normalize(v1);
        std::cout <<"3D 1: " << v1.x << " " << v1.y << " " << v1.z << std::endl;
        glm::vec3 v2;
        v2.x = (2.0f * xPos - width) / width;
        v2.y = (height - 2.0f * yPos) / height;
        v2.z = 0.0f;
        //std::cout << v2.x << " " << v2.y << " " << v2.z << std::endl;
        d = v2.length();//glm::length(v2);
        d = (d < 1.0f) ? d : 1.0f;
        v2.z = sqrtf(1.001f - d*d);
        v2 = glm::normalize(v2);
        std::cout <<"3D 2: " << v2.x << " " << v2.y << " " << v2.z << std::endl;
        if (dragonMode)
            dragon->trackBallRotation(v1, v2);
        if (bunnyMode)
            bunny->trackBallRotation(v1, v2);
        if (bearMode)
            bear->trackBallRotation(v1, v2);
    }
}
