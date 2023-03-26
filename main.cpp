// Inclure les entêtes standard
#include <stdio.h> 
#include <stdlib.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
// Inclure GLEW. Toujours l'inclure avant gl.h et glfw.h, car c'est un peu magique. 
#include <GL/glew.h>
// Inclure GLFW 
#include <GLFW/glfw3.h>
// Inclure GLM 
#include <glm/glm.hpp> 
#//include "loadShader.cpp"
using namespace glm;
using namespace std;

struct ShaderProgramSource
{
    string VertexSource;
    string FragmentSource;
};

static ShaderProgramSource ParserShader(const string& filePath) {
    fstream stream(filePath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    string line;
    stringstream  ss[2];
    ShaderType type = ShaderType::NONE;
    

    while (getline(stream, line)) {
        if (line.find("shader") != string::npos) {
            if (line.find("vertex") != string::npos) {
                //set mode to vertex
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != string::npos)
            {
                //set mode to fragment
                type = ShaderType::FRAGMENT;
            }
        }
        else {

            ss[(int) type]  << line << "\n";
            
        }
    }
    return { ss[0].str(), ss[1].str() };

}

static unsigned int CompileShader(const string& source, unsigned int type) {
   
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;

    glGetShaderiv(id, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE) {

        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        cout << "Failed to compile "<<(type == GL_VERTEX_SHADER ? "vertec" : "fragment") << " shader" << endl;
        cout << message << endl;
        glDeleteShader(id);
        return 0;

    }

    return id;

}

static unsigned int CreateShader( const string& vertexShader, const string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int  vs = CompileShader (vertexShader, GL_VERTEX_SHADER);
    unsigned int  fs = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;

    return 0;
}

int main() {
    // Initialise GLFW 
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4); // antialiasing 4x 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Nous voulons OpenGL 3.3 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Nous ne voulons pas de support de l'ancien OpenGL

    // Ouvre une fenêtre et crée son contexte OpenGL
    GLFWwindow* window; // (Dans le code joint, cette variable est globale) 
    window = glfwCreateWindow(1024, 768, "Tutorial 01", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialise GLEW 
    glewExperimental = true; // Nécessaire pour le profil core
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Un tableau de trois vecteurs qui représentent trois sommets
    static const GLfloat g_vertex_buffer_data[] = {
       -1.0f, -1.0f, 0.0f,
       1.0f, -1.0f, 0.0f,
       1.0f,  1.0f, 0.0f,
       -1.0f, 1.0f, 0.0f,

    };

    unsigned int indices[] = {
            0,1,2,
            2,3,0
    };

    // Ceci identifiera notre tampon de sommets
    GLuint vertexbuffer;
     
    // Génère un tampon et place l'identifiant dans 'vertexbuffer'
    glGenBuffers(1, &vertexbuffer);

    // Les commandes suivantes vont parler de notre tampon 'vertexbuffer'
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

    // Fournit les sommets à OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    // premier tampon d'attributs : les sommets
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        0,                  // attribut 0. Aucune raison particulière pour 0, mais cela doit correspondre au « layout » dans le shader 
        3,                  // taille
        GL_FLOAT,           // type 
        GL_FALSE,           // normalisé ? 
        0,                  // nombre d'octets séparant deux sommets dans le tampon
        (void*)0            // décalage du tableau de tampon
    );

    unsigned int ibo;
    glGenBuffers(1, &ibo);

    // Les commandes suivantes vont parler de notre tampon 'vertexbuffer'
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    // Fournit les sommets à OpenGL.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // premier tampon d'attributs : les sommets

    // Crée et compile notre programme GLSL à partir des shaders
   

    ShaderProgramSource source = ParserShader("basic.shader");
    cout << "vertex" << endl;
    cout << source.VertexSource << endl;
    cout << "fragment" << endl;
    cout << source.FragmentSource << endl;
     unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
     glUseProgram(shader);
    
   // GLuint programID = LoadShaders("vertex1.glsl", "fragmentShader.glsl");

    // S'assure que l'on puisse capturer la touche échap utilisée plus bas
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);


    do {
        // N'affiche rien, à bientôt dans le deuxième tutoriel !
        // Dessine le triangle ! 
        

        glClear(GL_COLOR_BUFFER_BIT);

       // glUseProgram(programID);
       // glDrawArrays(GL_TRIANGLES, 0, 6); // Démarre à partir du sommet 0; 3 sommets au total -> 1 triangle 
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glfwSwapBuffers(window); 
        glfwPollEvents();

        glDeleteProgram(shader);

    } // Vérifie si la touche échap a été appuyée ou si la fenêtre a été fermée
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);
}
