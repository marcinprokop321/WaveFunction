#include <glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <vector>
#include <Windows.h>
#include "shader_s.h"
typedef struct tile
{
	uint8_t id;
 } tile;
#define tileSize 0.005f
#define widht 400
typedef struct buffer
{
	unsigned int VAO, VBO;
}buffer;
uint8_t* tiles;
uint16_t ran16(uint16_t cap)
{
	uint16_t ret;
	_rdrand16_step(&ret);
	return ret % cap;
}
buffer CreateTile(float size)
{
	float vert[] = { 0,0,0,size, size,0,0,size, size,0,size,size };
	unsigned int vao, vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	return { vao,vbo };
}
void drawTile(buffer id)
{
	glBindVertexArray(id.VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
void render(buffer id, GLFWwindow *win, Shader *sh,int mapSize)
{
	for (int y = 0;y<mapSize;y++)
	{
		for (int x = 0; x < mapSize; x++)
		{
			uint8_t color = tiles[x + (y * widht)];
			glm::mat4 position = glm::mat4(1.0f);
			sh->use();
			position = glm::translate(position, glm::vec3(((float)x * tileSize) - 1.0f, (-1.0f * (((float)y * tileSize) - 1.0f)) - tileSize, 0.0f));
			sh->setMat4("pos", position);
			glm::vec3 colour = { 0.004 * (color % 128) ,0.004 * (color % 128) ,0.004 * (color % 128) };
			sh->setVec3("atex", colour);
			glBindVertexArray(id.VAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}
}
uint8_t waveFunc(uint8_t* map, uint8_t til, int x, int y)
{
	uint8_t sourround[4] = { 0 };
	if (x < (widht - 1))
		sourround[0] = map[int((x + 1) + (y * widht))]%(uint8_t)128;
	if (x > 0)
		sourround[1] = map[int((x - 1) + (y * widht))] % (uint8_t)128;
	if (y < (widht - 1))
		sourround[2] = map[int(x + ((y + 1) * widht))] % (uint8_t)128;
	if (y > 0)
		sourround[3] = map[int(x + ((y - 1) * widht))] % (uint8_t)128;
	if (sourround[0] + sourround[1] + sourround[2] + sourround[3] == 0)
		return { 0 };
	uint8_t xpp = ((sourround[0] + sourround[1] + sourround[2] + sourround[3]))%128;
	xpp += 128;
	return xpp;
}
int translateScreenToMapX(double x)
{
	return x/(1000 / widht) ;
}
int translateScreenToMapY(double y)
{
	return y/(1000/widht);
}
uint8_t getBlock(double x, double y)
{
	int xi = translateScreenToMapX(x);
	int yi = translateScreenToMapY(y);
	if (yi<0 || xi<0 || xi>(widht - 1) || yi>(widht - 1))
		return 0;
	return tiles[xi + (yi * widht)];
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		printf("%d\n", getBlock(x, y));
		//getBlock(x, y);
	}
}
int main(int argc, char** argv)
{
	float chance = 10000;
	srand(time(NULL));
	tiles =(uint8_t*)malloc(widht * widht);
	memset(tiles, 0, widht * widht);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(1000, 1000, "xppp", NULL, NULL);
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	buffer xp = CreateTile(tileSize);
	glClearColor(1, 1, 1, 0);
	Shader shader = Shader("vt.txt", "fr.txt");
	while (!glfwWindowShouldClose(window))
	{
		//main loop
		glfwPollEvents();
		float x = ran16(widht);
		float y = ran16(widht);
		int ran = ran16(chance);
		if (ran > 700 && tiles[int(x + (y * widht))] == 0)
		    uint8_t ran = ran16(128);
		tiles[int(x + (y * widht))] = ran;
		for (int x1 = 0; x1 < widht; x1++)
		{
			for (int y1 = 0; y1 < widht; y1++)
			{
				ran = ran16(chance);
				if (ran > 9000)
				{
					uint8_t* tmp = &tiles[int(x1 + (y1 * widht))];
					if(*tmp==0&&!(*tmp>>7))
						*tmp = waveFunc(tiles, *tmp, x1, y1);
				}
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);
		render(xp, window, &shader,widht);
		glfwSwapBuffers(window);
	}
}
