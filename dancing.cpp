/*

    Copyright (C) 2022 Xilin Jia https://github.com/XilinJia
    Copyright 2011 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Tutorial 16 - Basic Texture Mapping
*/

#include <stdio.h>
#include <string.h>

#include <iostream>

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "ogldev_pipeline.h"
#include "ogldev_math_3d.h"
#include "ogldev_texture.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1200

struct Vertex
{
    Vector3f m_pos;
    Vector2f m_tex;

    Vertex() {}

    Vertex(Vector3f pos, Vector2f tex)
    {
        m_pos = pos;
        m_tex = tex;
    }
};

GLuint VBO;
GLuint IBO;
GLuint gWVPLocation;
GLuint gSampler;
Texture *pTexture = NULL, *pTexture1 = NULL;
Camera *pGameCamera = NULL;
PersProjInfo gPersProjInfo;

const char *pVSFileName = "shader.vs";
const char *pFSFileName = "shader.fs";

static Vertex Vertices[1000];
unsigned int Indices[5000];
int nv = 5, nl = 20;
int iter = 0;

static void CreateVertexBuffer(float Scale)
{
    if (nv > 30)
        nv = 5;

    float angle = 2 * M_PI / nv;
    for (int l = 0; l < nl; l++)
    {
        for (int i = 0; i <= nv; i++)
        {
            Vertices[l * (nv + 1) + i] = Vertex(
                Vector3f(-cos(angle / 2 + i * angle) - 0.1 * sin(0.6f * Scale) * cos(l * 0.5f),
                         -1.0f + 2.0f * l / (nl - 1) - 0.06 * cos(0.6f * Scale),
                         sin(angle / 2 + i * angle)),
                Vector2f(1.0f * i / nv, 1.0f * l / (nl - 1)));
        }
    }
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, nl * (nv + 1) * sizeof(Vertex), Vertices, GL_STATIC_DRAW);

    for (int l = 0; l < nl - 1; l++)
    {
        for (int i = 0; i < nv; i++)
        {
            Indices[6 * l * nv + 6 * i] = l * (nv + 1) + i;
            Indices[6 * l * nv + 6 * i + 1] = l * (nv + 1) + (nv + 1) + i + 1;
            Indices[6 * l * nv + 6 * i + 2] = l * (nv + 1) + (nv + 1) + i;
            Indices[6 * l * nv + 6 * i + 3] = l * (nv + 1) + i;
            Indices[6 * l * nv + 6 * i + 4] = l * (nv + 1) + i + 1;
            Indices[6 * l * nv + 6 * i + 5] = l * (nv + 1) + (nv + 1) + i + 1;
        }
    }
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * (nl - 1) * nv * sizeof(int), Indices, GL_STATIC_DRAW);
}

static void _RenderSceneCB()
{
    pGameCamera->OnRender();

    glClear(GL_COLOR_BUFFER_BIT);

    static float Scale = 0.0f;

    Scale += 0.5f;
    iter++;

    CreateVertexBuffer(Scale);

    Pipeline p;
    p.Rotate(0.0f, Scale, 0.0f);
    p.WorldPos(0.0f, 0.0f, 5.0f);
    p.SetCamera(*pGameCamera);
    p.SetPerspectiveProj(gPersProjInfo);

    glUniformMatrix4fv(gWVPLocation, 1, GL_TRUE, (const GLfloat *)p.GetWVPTrans());

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)12);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    pTexture->Bind(GL_TEXTURE0);
    pTexture1->Bind(GL_TEXTURE1);
    int size;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    glDrawElements(GL_TRIANGLES, size / sizeof(GLushort), GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glutSwapBuffers();

    if (iter % 200 == 0)
    {
        nv++;
        cout << "Vertices: " << nl * (nv + 1) << " Indices: " << 6 * (nl - 1) * nv << "\n";
        float alpha = 0.f;
        if (nv > 10 && nv < 25)
            alpha = (nv - 10) / 15.f;
        else if (nv >= 25)
            alpha = 1.0f;
        glUniform1f(glGetUniformLocation(gSampler, "alpha"), alpha);
    }
}

static void AddShader(GLuint ShaderProgram, const char *pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0)
    {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(1);
    }

    const GLchar *p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0] = strlen(pShaderText);
    glShaderSource(ShaderObj, 1, p, Lengths);
    glCompileShader(ShaderObj);
    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }

    glAttachShader(ShaderProgram, ShaderObj);
}

static void CompileShaders()
{
    GLuint ShaderProgram = glCreateProgram();

    if (ShaderProgram == 0)
    {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    string vs, fs;

    if (!ReadFile(pVSFileName, vs))
    {
        exit(1);
    };

    if (!ReadFile(pFSFileName, fs))
    {
        exit(1);
    };

    AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);
    AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = {0};

    glLinkProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (Success == 0)
    {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success)
    {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ShaderProgram);

    gWVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
    assert(gWVPLocation != 0xFFFFFFFF);
    // gSampler = glGetUniformLocation(ShaderProgram, "gSampler");
    gSampler = glGetUniformLocation(ShaderProgram, "texture1");
    assert(gSampler != 0xFFFFFFFF);
    glUniform1i(gSampler, 0);
    glUniform1i(glGetUniformLocation(gSampler, "texture2"), 1);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Tutorial 16");
    glutGameModeString("1920x12@32");
    glutEnterGameMode();

    glutIdleFunc(_RenderSceneCB);

    pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK)
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    CompileShaders();

    glUniform1i(gSampler, 0);

    pTexture = new Texture(GL_TEXTURE_2D, "two_heads.png");
    if (!pTexture->Load())
    {
        return 1;
    }

    pTexture1 = new Texture(GL_TEXTURE_2D, "rose.png");
    if (!pTexture1->Load())
    {
        return 1;
    }

    gPersProjInfo.FOV = 60.0f;
    gPersProjInfo.Height = WINDOW_HEIGHT;
    gPersProjInfo.Width = WINDOW_WIDTH;
    gPersProjInfo.zNear = 1.0f;
    gPersProjInfo.zFar = 100.0f;

    glutMainLoop();

    return 0;
}
