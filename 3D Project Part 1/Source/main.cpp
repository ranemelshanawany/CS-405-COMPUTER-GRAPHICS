#include <iostream>
#include <vector>

#include "GLM/glm.hpp"
#include "GLM/common.hpp"
#include "GLM/gtc/type_ptr.hpp"
#include "GLM/gtc/random.hpp"
#include "GLAD/glad.h"
#include "GLFW/glfw3.h"
#include "opengl_utilities.h"
#include "mesh_generation.h"

/* Keep the global state inside this struct */
static struct {
	glm::dvec2 mouse_position;
	glm::ivec2 screen_dimensions = glm::ivec2(960, 960);
	GLfloat key;
} Globals;

/* GLFW Callback functions */
static void ErrorCallback(int error, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

static void CursorPositionCallback(GLFWwindow* window, double x, double y)
{
	Globals.mouse_position.x = x;
	Globals.mouse_position.y = y;
}

static void WindowSizeCallback(GLFWwindow* window, int width, int height)
{
	Globals.screen_dimensions.x = width;
	Globals.screen_dimensions.y = height;

	glViewport(0, 0, width, height);
}

static void keyPressedCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Globals.key = key;
}

int main(int argc, char* argv[])
{
	/* Set GLFW error callback */
	glfwSetErrorCallback(ErrorCallback);

	/* Initialize the library */
	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	/* Create a windowed mode window and its OpenGL context */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	GLFWwindow* window = glfwCreateWindow(
		Globals.screen_dimensions.x, Globals.screen_dimensions.y,
		"Ranem Elshanawany", NULL, NULL
	);
	if (!window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	/* Move window to a certain position [do not change] */
	glfwSetWindowPos(window, 10, 50);
	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	/* Enable VSync */
	glfwSwapInterval(1);

	/* Load OpenGL extensions with GLAD */
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Set GLFW Callbacks */
	glfwSetCursorPosCallback(window, CursorPositionCallback);
	glfwSetWindowSizeCallback(window, WindowSizeCallback);
	glfwSetKeyCallback(window, keyPressedCallback);

	/* Configure OpenGL */
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);

	/* Creating Meshes */
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<GLuint> indicies;
	GenerateParametricShapeFrom2D(positions, normals, indicies, ParametricHalfCircle, 16, 16, false);
	VAO sphereVAO(positions, normals, indicies);

	positions.clear();
	normals.clear();
	indicies.clear();
	GenerateParametricShapeFrom2D(positions, normals, indicies, ParametricCircle, 16, 16, false);
	VAO torusVAO(positions, normals, indicies);

	positions.clear();
	normals.clear();
	indicies.clear();
	GenerateParametricShapeFrom2D(positions, normals, indicies, ParametricHalfSquiggle, 160, 160, true);
	VAO sqiggleVAO(positions, normals, indicies);

	positions.clear();
	normals.clear();
	indicies.clear();
	GenerateParametricShapeFrom2D(positions, normals, indicies, ParametricSpikes, 160, 160, true);
	VAO sqiggle2VAO(positions, normals, indicies);

	positions.clear();
	normals.clear();
	indicies.clear();
	GenerateParametricShapeFrom2D(positions, normals, indicies, ParametricSpikes, 160, 160, true);
	VAO flowerVAO(positions, normals, indicies);


	/* Creating Programs */
	GLuint wireframe = CreateProgramFromSources(
		R"delimiter(
#version 330 core
	
layout(location = 0) in vec3 a_position; //0 must be same as glVertexAttribPointer(0,
layout(location = 1) in vec3 a_normal; 

uniform mat4 u_transform;
	
out vec3 vertex_position;
out vec3 vertex_normal;

void main()
{	
	gl_Position = u_transform * vec4(a_position, 1);
	vertex_normal = vec3(u_transform* vec4(a_normal,0));
	vertex_position = vec3(gl_Position);
}
	
)delimiter",

R"delimiter(
#version 330 core

uniform vec2 u_mouse_position;
	
in vec3 vertex_position;
in vec3 vertex_normal;
	
out vec4 out_color;

void main()
{
	out_color = vec4(1, 1, 1, 1);
}
	
)delimiter");

	if (wireframe == NULL)
	{
		glfwTerminate();
		return -1;
	}

	GLuint normal = CreateProgramFromSources(
		R"delimiter(
#version 330 core
	
layout(location = 0) in vec3 a_position; //0 must be same as glVertexAttribPointer(0,
layout(location = 1) in vec3 a_normal; 

uniform mat4 u_transform;
	
out vec3 vertex_position;
out vec3 vertex_normal;

void main()
{	
	gl_Position = u_transform * vec4(a_position, 1);
	vertex_normal = vec3(u_transform* vec4(a_normal,0));
	vertex_position = vec3(gl_Position);
}
	
)delimiter",

R"delimiter(
#version 330 core

uniform vec2 u_mouse_position;
	
in vec3 vertex_position;
in vec3 vertex_normal;
	
out vec4 out_color;

void main()
{
	vec3 surface_position = vertex_position;
	vec3 surface_color = vertex_normal;

	vec3 color = vec3(0);
	float ambient = 1;
	vec3 ambient_color = vertex_normal;

	color += ambient * ambient_color;

	vec3 light_direction = vec3(1, 2.5, -1);
	vec3 light_color = vec3(1,1,1);

	float diffuse =  max(0, dot(light_direction, vertex_normal));
	color += 1 *diffuse * light_color * surface_color;


	out_color = vec4(color, 1);
}
	
)delimiter");

	if (normal == NULL)
	{
		glfwTerminate();
		return -1;
	}

	GLuint grey = CreateProgramFromSources(
		R"delimiter(
#version 330 core
	
layout(location = 0) in vec3 a_position; 
layout(location = 1) in vec3 a_normal; 

uniform mat4 u_transform;
	
out vec3 vertex_position;
out vec3 vertex_normal;

void main()
{	
	gl_Position = u_transform * vec4(a_position, 1);
	vertex_normal = vec3(u_transform* vec4(a_normal,0));
	vertex_position = vec3(gl_Position);
}
	
)delimiter",

R"delimiter(
#version 330 core

uniform vec2 u_mouse_position;
	
in vec3 vertex_position;
in vec3 vertex_normal;
	
out vec4 out_color;

void main()
{
	vec3 color = vec3(0);

	vec3 surface_position = vertex_position;
	vec3 surface_color = vec3(0.5, 0.5, 0.5);
	vec3 surface_normal = normalize(vertex_normal);

	vec3 ambient_color = vec3(0.5, 0.5, 0.5);

	color += ambient_color * surface_color;

	vec3 light_direction = normalize(vec3(-1, -1, 1));
	vec3 to_light = -normalize(light_direction);
	vec3 light_color = vec3(0.4, 0.4, 0.4);

	float diffuse_intensity = max(0, dot(to_light, surface_normal));
	color += diffuse_intensity * light_color * surface_color;

	vec3 view_dir = normalize(vec3(0, 0, -1));
	vec3 halfway_dir = normalize(view_dir + to_light);
	float shininess = 64;
	float specular_intensity = max(0, dot(halfway_dir, surface_normal));
	color += pow(specular_intensity, shininess) * light_color;

	out_color = vec4(color, 1);
}
	
)delimiter");

	if (grey == NULL)
	{
		glfwTerminate();
		return -1;
	}

	GLuint color = CreateProgramFromSources(
		R"VERTEX(
#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

uniform mat4 u_transform;

out vec3 vertex_position;
out vec3 vertex_normal;

void main()
{
	gl_Position = u_transform * vec4(a_position, 1);
	vertex_normal = vec3(u_transform * vec4(a_normal, 0));
	vertex_position = vec3(gl_Position);
}
		)VERTEX",

		R"FRAGMENT(
#version 330 core

uniform vec2 u_mouse_position;
uniform vec3 u_color;
uniform vec3 u_shininess;

in vec3 vertex_position;
in vec3 vertex_normal;

out vec4 out_color;

void main()
{
	vec3 color = vec3(0);

	vec3 surface_color = u_color;
	vec3 surface_position = vertex_position;
	vec3 surface_normal = normalize(vertex_normal);

	vec3 ambient_color = vec3(0.5, 0.5 ,0.5);
	color += ambient_color * surface_color;

	vec3 light_direction = normalize(vec3(-1, -1, 1));
	vec3 to_light = -normalize(light_direction);
	vec3 light_color = vec3(0.4, 0.4, 0.4);

	float diffuse_intensity = max(0, dot(to_light, surface_normal));
	color += diffuse_intensity * light_color * surface_color;

	vec3 view_dir = normalize(vec3(0, 0, -1));	
	vec3 halfway_dir = normalize(view_dir + to_light);
	float specular_intensity = max(0, dot(halfway_dir, surface_normal));
	float shiny = u_shininess.x;
	color += pow(specular_intensity, shiny) * light_color;

	vec3 light_direction2 = normalize(vec3(-u_mouse_position, 2));
	vec3 to_light2 = -normalize(light_direction2);
	vec3 light_color2 =  vec3(0.5);

	float diffuse_intensity2 = max(0, dot(to_light2, surface_normal));
	color += diffuse_intensity2 * light_color2 * surface_color;

	vec3 halfway_dir2 = normalize(view_dir + to_light2);
	float specular_intensity2 = max(0, dot(halfway_dir2, surface_normal));
	shiny = u_shininess.x;
	color += pow(specular_intensity2, shiny) * light_color2;

	out_color = vec4(color, 1);
}
		)FRAGMENT");

	GLuint creative = CreateProgramFromSources(
		R"VERTEX(
#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

uniform mat4 u_transform;

out vec3 vertex_position;
out vec3 vertex_normal;

void main()
{
	gl_Position = u_transform * vec4(a_position, 1);
	vertex_normal = vec3(u_transform * vec4(a_normal, 0));
	vertex_position = vec3(gl_Position);
}
		)VERTEX",

		R"FRAGMENT(
#version 330 core

uniform vec2 u_mouse_position;
uniform vec3 u_color;
uniform vec3 u_shininess;

in vec3 vertex_position;
in vec3 vertex_normal;

out vec4 out_color;

void main()
{
	vec3 color = vec3(0);

	vec3 surface_color = u_color;
	vec3 surface_position = vertex_position;
	vec3 surface_normal = normalize(vertex_normal);

	vec3 ambient_color = vec3(255)/255.;
	color += 0.0* ambient_color * surface_color;

	vec3 light_direction = normalize(vec3(1, 1, 1));
	vec3 to_light = normalize(-1 * light_direction);
	vec3 light_color = vec3(143, 3, 87)/255.;

	float diffuse_intensity = max(0, dot(to_light, surface_normal));
	color += diffuse_intensity * light_color * surface_color;

	vec3 light_direction2 = normalize(vec3(-1, 1, 1));
	vec3 to_light2 = normalize(-1 * light_direction2);
	vec3 light_color2 = vec3(0,0,1);

	float diffuse_intensity2 = max(0, dot(to_light2, surface_normal));
	color += diffuse_intensity2 * light_color2 * surface_color;

	out_color = vec4(color, 1);
}
		)FRAGMENT");

	if (creative == NULL)
	{
		glfwTerminate();
		return -1;
	}
	Globals.key = GLFW_KEY_Q;
	glm::dvec2 chasing_pos = glm::dvec2(0);
	glm::dvec2 chasing_pos_list[36];
	for (int i = 0; i < 36; i++)
	{
		chasing_pos_list[i] = glm::dvec2(0);//glm::ballRand(0.5);
	}
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (Globals.key == GLFW_KEY_Q)
		{
			glClearColor(0,0,0,1);
			glUseProgram(wireframe);

			auto u_transform_location = glGetUniformLocation(wireframe, "u_transform");
			
			glm::mat4 transform(1.0);
			transform = glm::scale(transform, glm::vec3(0.46));
			transform = glm::translate(transform, glm::vec3(-1.1, 1, 0));
			transform = glm::rotate(transform, float(glfwGetTime() * glm::radians(10.)), glm::vec3(1, 1, 0));
			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform));

			glBindVertexArray(sphereVAO.id);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawElements(GL_TRIANGLES, sphereVAO.element_array_count, GL_UNSIGNED_INT, NULL);

			glm::mat4 transform2(1.0);
			transform2 = glm::scale(transform2, glm::vec3(0.46));
			transform2 = glm::translate(transform2, glm::vec3(1, 1, 0));
			transform2 = glm::rotate(transform2, glm::radians(float(glfwGetTime() * 10)), glm::vec3(1, 1, 0));

			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform2));

			glBindVertexArray(torusVAO.id);
			glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, NULL);


			glm::mat4 transform3(1.0);
			transform3 = glm::scale(transform3, glm::vec3(0.3));
			transform3 = glm::translate(transform3, glm::vec3(1.5, -1.5, 0));
			transform3 = glm::rotate(transform3, glm::radians(float(glfwGetTime() * 10)), glm::vec3(1, 1, 0));

			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform3));

			glBindVertexArray(sqiggleVAO.id);
			glDrawElements(GL_TRIANGLES, sqiggleVAO.element_array_count, GL_UNSIGNED_INT, NULL);

			glm::mat4 transform4(1.0);
			transform4 = glm::scale(transform4, glm::vec3(0.4));
			transform4 = glm::translate(transform4, glm::vec3(-1.2, -1.2, 0));
			transform4 = glm::rotate(transform4, glm::radians(float(glfwGetTime() * 10)), glm::vec3(1, 1, 0));

			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform4));

			glBindVertexArray(sqiggle2VAO.id);
			glDrawElements(GL_TRIANGLES, sqiggle2VAO.element_array_count, GL_UNSIGNED_INT, NULL);
		}
		else if (Globals.key == GLFW_KEY_W)
		{
			glClearColor(0, 0, 0, 1);
			glUseProgram(normal);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			auto u_transform_location = glGetUniformLocation(wireframe, "u_transform");

			glm::mat4 transform(1.0);
			transform = glm::scale(transform, glm::vec3(0.46));
			transform = glm::translate(transform, glm::vec3(-1.1, 1, 0));
			transform = glm::rotate(transform, float(glfwGetTime() * glm::radians(10.)), glm::vec3(1, 1, 0));
			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform));

			glBindVertexArray(sphereVAO.id);
			glDrawElements(GL_TRIANGLES, sphereVAO.element_array_count, GL_UNSIGNED_INT, NULL);

			glm::mat4 transform2(1.0);
			transform2 = glm::scale(transform2, glm::vec3(0.46));
			transform2 = glm::translate(transform2, glm::vec3(1, 1, 0));
			transform2 = glm::rotate(transform2, glm::radians(float(glfwGetTime() * 10)), glm::vec3(1, 1, 0));

			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform2));

			glBindVertexArray(torusVAO.id);
			glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, NULL);


			glm::mat4 transform3(1.0);
			transform3 = glm::scale(transform3, glm::vec3(0.3));
			transform3 = glm::translate(transform3, glm::vec3(1.5, -1.5, 0));
			transform3 = glm::rotate(transform3, glm::radians(float(glfwGetTime() * 10)), glm::vec3(1, 1, 0));

			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform3));

			glBindVertexArray(sqiggleVAO.id);
			glDrawElements(GL_TRIANGLES, sqiggleVAO.element_array_count, GL_UNSIGNED_INT, NULL);

			glm::mat4 transform4(1.0);
			transform4 = glm::scale(transform4, glm::vec3(0.4));
			transform4 = glm::translate(transform4, glm::vec3(-1.2, -1.2, 0));
			transform4 = glm::rotate(transform4, glm::radians(float(glfwGetTime() * 10)), glm::vec3(1, 1, 0));

			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform4));

			glBindVertexArray(sqiggle2VAO.id);
			glDrawElements(GL_TRIANGLES, sqiggle2VAO.element_array_count, GL_UNSIGNED_INT, NULL);
		}
		else if (Globals.key == GLFW_KEY_E)
		{
			glClearColor(0, 0, 0, 1);
			glUseProgram(grey);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			auto u_transform_location = glGetUniformLocation(wireframe, "u_transform");

			glm::mat4 transform(1.0);
			transform = glm::scale(transform, glm::vec3(0.46));
			transform = glm::translate(transform, glm::vec3(-1.1, 1, 0));
			transform = glm::rotate(transform, float(glfwGetTime() * glm::radians(10.)), glm::vec3(1, 1, 0));
			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform));

			glBindVertexArray(sphereVAO.id);
			glDrawElements(GL_TRIANGLES, sphereVAO.element_array_count, GL_UNSIGNED_INT, NULL);

			glm::mat4 transform2(1.0);
			transform2 = glm::scale(transform2, glm::vec3(0.46));
			transform2 = glm::translate(transform2, glm::vec3(1, 1, 0));
			transform2 = glm::rotate(transform2, glm::radians(float(glfwGetTime() * 10)), glm::vec3(1, 1, 0));

			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform2));

			glBindVertexArray(torusVAO.id);
			glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, NULL);


			glm::mat4 transform3(1.0);
			transform3 = glm::scale(transform3, glm::vec3(0.3));
			transform3 = glm::translate(transform3, glm::vec3(1.5, -1.5, 0));
			transform3 = glm::rotate(transform3, glm::radians(float(glfwGetTime() * 10)), glm::vec3(1, 1, 0));

			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform3));

			glBindVertexArray(sqiggleVAO.id);
			glDrawElements(GL_TRIANGLES, sqiggleVAO.element_array_count, GL_UNSIGNED_INT, NULL);

			glm::mat4 transform4(1.0);
			transform4 = glm::scale(transform4, glm::vec3(0.4));
			transform4 = glm::translate(transform4, glm::vec3(-1.2, -1.2, 0));
			transform4 = glm::rotate(transform4, glm::radians(float(glfwGetTime() * 10)), glm::vec3(1, 1, 0));

			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform4));

			glBindVertexArray(sqiggle2VAO.id);
			glDrawElements(GL_TRIANGLES, sqiggle2VAO.element_array_count, GL_UNSIGNED_INT, NULL);
		}
		else if (Globals.key == GLFW_KEY_R)
		{
			glClearColor(0, 0, 0, 1);
			glUseProgram(color);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			auto u_transform_location = glGetUniformLocation(color, "u_transform");
			auto mouse_location = glGetUniformLocation(color, "u_mouse_position");
			auto color_location = glGetUniformLocation(color, "u_color");
			auto shininess_location = glGetUniformLocation(color, "u_shininess");

			auto normalized_mouse = Globals.mouse_position / glm::dvec2(Globals.screen_dimensions);
			normalized_mouse.y = 1. - normalized_mouse.y;
			normalized_mouse = normalized_mouse * 2. - 1.;

			glUniform2fv(mouse_location, 1, glm::value_ptr(glm::vec2(normalized_mouse)));

			glm::mat4 transform(1.0);
			transform = glm::scale(transform, glm::vec3(0.46));
			transform = glm::translate(transform, glm::vec3(-1.1, 1, 0));
			transform = glm::rotate(transform, float(glfwGetTime() * glm::radians(10.)), glm::vec3(1, 1, 0));
			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform));
			glUniform3fv(color_location, 1, glm::value_ptr(glm::vec3(0.5, 0.5, 0.5)));
			glUniform3fv(shininess_location, 1, glm::value_ptr(glm::vec3(128, 0, 0)));

			glBindVertexArray(sphereVAO.id);
			glDrawElements(GL_TRIANGLES, sphereVAO.element_array_count, GL_UNSIGNED_INT, NULL);

			glm::mat4 transform2(1.0);
			transform2 = glm::scale(transform2, glm::vec3(0.46));
			transform2 = glm::translate(transform2, glm::vec3(1, 1, 0));
			transform2 = glm::rotate(transform2, glm::radians(float(glfwGetTime() * 10)), glm::vec3(1, 1, 0));
			glUniform2fv(mouse_location, 1, glm::value_ptr(glm::vec2(normalized_mouse)));
			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform2));
			glUniform3fv(color_location, 1, glm::value_ptr(glm::vec3(1, 0, 0)));
			glUniform3fv(shininess_location, 1, glm::value_ptr(glm::vec3(32, 0, 0)));

			glBindVertexArray(torusVAO.id);
			glDrawElements(GL_TRIANGLES, torusVAO.element_array_count, GL_UNSIGNED_INT, NULL);


			glm::mat4 transform3(1.0);
			transform3 = glm::scale(transform3, glm::vec3(0.3));
			transform3 = glm::translate(transform3, glm::vec3(1.5, -1.5, 0));
			transform3 = glm::rotate(transform3, glm::radians(float(glfwGetTime() * 10)), glm::vec3(1, 1, 0));
			glUniform2fv(mouse_location, 1, glm::value_ptr(glm::vec2(normalized_mouse)));

			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform3));
			glUniform3fv(color_location, 1, glm::value_ptr(glm::vec3(0, 0, 1)));
			glUniform3fv(shininess_location, 1, glm::value_ptr(glm::vec3(64, 0, 0)));

			glBindVertexArray(sqiggleVAO.id);
			glDrawElements(GL_TRIANGLES, sqiggleVAO.element_array_count, GL_UNSIGNED_INT, NULL);

			glm::mat4 transform4(1.0);
			transform4 = glm::scale(transform4, glm::vec3(0.4));
			transform4 = glm::translate(transform4, glm::vec3(-1.2, -1.2, 0));
			transform4 = glm::rotate(transform4, glm::radians(float(glfwGetTime() * 10)), glm::vec3(1, 1, 0));
			glUniform2fv(mouse_location, 1, glm::value_ptr(glm::vec2(normalized_mouse)));

			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform4));
			glUniform3fv(color_location, 1, glm::value_ptr(glm::vec3(0, 1, 0)));
			glUniform3fv(shininess_location, 1, glm::value_ptr(glm::vec3(300, 0, 0)));

			glBindVertexArray(sqiggle2VAO.id);
			glDrawElements(GL_TRIANGLES, sqiggle2VAO.element_array_count, GL_UNSIGNED_INT, NULL);

		}
		else if (Globals.key == GLFW_KEY_T)
		{
			glClearColor(0, 0, 0, 1);
			glUseProgram(color);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			auto mouse_location = glGetUniformLocation(color, "u_mouse_position");
			auto color_location = glGetUniformLocation(color, "u_color");
			auto u_transform_location = glGetUniformLocation(color, "u_transform");
			auto shininess_location = glGetUniformLocation(color, "u_shininess");

			glm::dvec2 normalized_mouse = Globals.mouse_position / glm::dvec2(Globals.screen_dimensions);
			normalized_mouse.y = 1. - normalized_mouse.y;
			normalized_mouse.x = normalized_mouse.x * 2. - 1.;
			normalized_mouse.y = normalized_mouse.y * 2. - 1.;

			glm::mat4 transform2(1.0);
			chasing_pos = glm::mix(normalized_mouse, chasing_pos, 0.99);
			transform2 = glm::translate(transform2, glm::vec3(chasing_pos, 1));
			transform2 = glm::scale(transform2, glm::vec3(0.3));
			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform2));
			glUniform2fv(mouse_location, 1, glm::value_ptr(glm::vec2(normalized_mouse)));
			glUniform3fv(color_location, 1, glm::value_ptr(glm::vec3(0.5, 0.5, 0.5)));
			glUniform3fv(shininess_location, 1, glm::value_ptr(glm::vec3(100, 0, 0)));
			glBindVertexArray(sphereVAO.id);
			glDrawElements(GL_TRIANGLES, sphereVAO.element_array_count, GL_UNSIGNED_INT, NULL);

			glm::mat4 transform(1.0);
			transform = glm::translate(transform, glm::vec3(normalized_mouse, 1));
			transform = glm::scale(transform, glm::vec3(0.3));			
			glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform));
			glUniform2fv(mouse_location, 1, glm::value_ptr(glm::vec2(normalized_mouse)));
			glUniform3fv(shininess_location, 1, glm::value_ptr(glm::vec3(100, 0, 0)));
			GLfloat distance = glm::pow((glm::pow((chasing_pos.x - normalized_mouse.x),2) + glm::pow((chasing_pos.y - normalized_mouse.y),2)),0.5);
			if (distance > 0.3*2)
			{
				glUniform3fv(color_location, 1, glm::value_ptr(glm::vec3(0,1,0)));
			}
			else
			{
				glUniform3fv(color_location, 1, glm::value_ptr(glm::vec3(1, 0, 0)));
			}
	
			glDrawElements(GL_TRIANGLES, sphereVAO.element_array_count, GL_UNSIGNED_INT, NULL);

		}
		else if (Globals.key == GLFW_KEY_Y)
		{
			glClearColor(0, 0, 0, 1);
			glUseProgram(creative);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			auto mouse_location = glGetUniformLocation(creative, "u_mouse_position");
			auto color_location = glGetUniformLocation(creative, "u_color");
			auto u_transform_location = glGetUniformLocation(creative, "u_transform");

			glm::dvec2 normalized_mouse = Globals.mouse_position / glm::dvec2(Globals.screen_dimensions);
			normalized_mouse.y = 1. - normalized_mouse.y;
			normalized_mouse.x = normalized_mouse.x * 2. - 1.;
			normalized_mouse.y = normalized_mouse.y * 2. - 1.;
			glm::dvec2 badMouse = -normalized_mouse;

			for (int i = 0; i < 18; i++)
			{
				chasing_pos_list[i] = glm::mix(normalized_mouse, chasing_pos_list[i], 0.99-(i*0.003+0.001));
				glm::mat4 transform(1.0);
				transform = glm::translate(transform, glm::vec3(chasing_pos_list[i], 1));
				transform = glm::scale(transform, glm::vec3(0.17));
				transform = glm::rotate(transform, float(glm::radians(90.)), glm::vec3(1, 0, 0));
				transform = glm::rotate(transform, float(glfwGetTime() * glm::radians(30.)), glm::vec3(0, 1, 0));
				glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform));
				glUniform2fv(mouse_location, 1, glm::value_ptr(glm::vec2(normalized_mouse)));
				glUniform3fv(color_location, 1, glm::value_ptr(glm::vec3(1)));
				glBindVertexArray(flowerVAO.id);
				glDrawElements(GL_TRIANGLES, flowerVAO.element_array_count, GL_UNSIGNED_INT, NULL);

				chasing_pos_list[i+18] = glm::mix(badMouse, chasing_pos_list[i+18], 0.99 - (i*0.003 + 0.001));
				glm::mat4 transform2(1.0);
				transform2 = glm::translate(transform2, glm::vec3(chasing_pos_list[i+18], 1));
				transform2 = glm::scale(transform2, glm::vec3(0.17));
				transform2 = glm::rotate(transform2, float(glm::radians(90.)), glm::vec3(1, 0, 0));
				transform2 = glm::rotate(transform2, float(glfwGetTime() * glm::radians(30.)), glm::vec3(0, 1, 0));
				glUniformMatrix4fv(u_transform_location, 1, GL_FALSE, glm::value_ptr(transform2));
				glUniform2fv(mouse_location, 1, glm::value_ptr(glm::vec2(badMouse)));
				glUniform3fv(color_location, 1, glm::value_ptr(glm::vec3(1, 0, 0)));
				glDrawElements(GL_TRIANGLES, flowerVAO.element_array_count, GL_UNSIGNED_INT, NULL);
			}

		}
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}