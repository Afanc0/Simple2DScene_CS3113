/**
* Author: Gianfranco Romani
* Assignment: Simple 2D Scene
* Date due: 2023-09-30, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define STB_IMAGE_IMPLEMENTATION
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.0f,
BG_BLUE = 0.75f,
BG_GREEN = 0.85f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const char TRIANGLE_SPRITE[] = "block.png";
const char SQUARE_SPRITE[] = "tria.png";

GLuint triangle_texture_id,
square_texture_id;

const float MILLISECONDS_IN_SECOND = 1000.0;
const float DEGREES_PER_SECOND = 90.0f;

SDL_Window* g_display_window;

bool g_game_is_running = true;

ShaderProgram g_program;
glm::mat4 g_view_matrix,
g_model_matrix,
g_projection_matrix,
g_trans_matrix;

float g_triangle_x = 0.0f;
float g_triangle_rotate = 0.0f;

float g_square_x = 0.5f;
float g_square_rotate = 0.5f;

float g_previous_ticks = 0.0f;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0; // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0; // this value MUST be zero

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Hello, Delta Time!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_program.Load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.
    g_trans_matrix = g_model_matrix;

    g_program.SetProjectionMatrix(g_projection_matrix);
    g_program.SetViewMatrix(g_view_matrix);
    // Notice we haven't set our model matrix yet!

    glUseProgram(g_program.programID);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    triangle_texture_id = load_texture(TRIANGLE_SPRITE);
    square_texture_id = load_texture(SQUARE_SPRITE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_game_is_running = false;
        }
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;

    g_triangle_x += 1.0f * delta_time;
    g_model_matrix = glm::mat4(1.0f);

    /* Translate */
    g_model_matrix = glm::translate(g_model_matrix, glm::vec3(g_triangle_x, 0.0f, 0.0f));


    g_square_x += 1.0f * delta_time;
    g_square_rotate += DEGREES_PER_SECOND * delta_time; // 90-degrees per second
    g_model_matrix = glm::mat4(1.0f);

    /* Rotate */
    g_model_matrix = glm::rotate(g_model_matrix, glm::radians(g_square_rotate), glm::vec3(0.0f, 0.0f, 1.0f));

}

void draw_square(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_program.SetModelMatrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); 
}

void draw_triangle(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_program.SetModelMatrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 3); 
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float s_vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float s_texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_program.positionAttribute, 2, GL_FLOAT, false, 0, s_vertices);
    glEnableVertexAttribArray(g_program.positionAttribute);

    glVertexAttribPointer(g_program.texCoordAttribute, 2, GL_FLOAT, false, 0, s_texture_coordinates);
    glEnableVertexAttribArray(g_program.texCoordAttribute);

    draw_square(g_model_matrix, square_texture_id);

    float t_vertices[] = {
        0.0f, -0.25f, 0.5f, -0.25f, 0.25f, 0.25f,  // triangle 1
    };

    float t_texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.0f,     // triangle 1
    };

    glVertexAttribPointer(g_program.positionAttribute, 2, GL_FLOAT, false, 0, t_vertices);
    glEnableVertexAttribArray(g_program.positionAttribute);

    glVertexAttribPointer(g_program.texCoordAttribute, 2, GL_FLOAT, false, 0, t_texture_coordinates);
    glEnableVertexAttribArray(g_program.texCoordAttribute);

    draw_triangle(g_model_matrix, triangle_texture_id);

    glDisableVertexAttribArray(g_program.positionAttribute);
    glDisableVertexAttribArray(g_program.texCoordAttribute);

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }

int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}