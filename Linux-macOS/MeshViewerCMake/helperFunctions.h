#pragma once

#define GL_SILENCE_DEPRECATION
#include <GL/glew.h>
#ifdef __APPLE__
#undef  glGenVertexArrays
#define glGenVertexArrays    glGenVertexArraysAPPLE
#undef  glBindVertexArray
#define glBindVertexArray    glBindVertexArrayAPPLE
#undef  glDeleteVertexArrays
#define glDeleteVertexArrays glDeleteVertexArraysAPPLE
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>


void menu(int item);
GLuint initshaders(GLenum type, const char *filename);
GLuint initprogram(GLuint, GLuint);
void display();

GLuint shaderprogram;

// width and height of the window.
int Glut_w = 600, Glut_h = 400;

// Variables and their values for the camera setup.
myPoint3D camera_eye(0, 0, 2);
myVector3D camera_up(0, 1, 0);
myVector3D camera_forward(0, 0, -1);

float fovy = 45.0f;
float zNear = 0.1f;
float zFar = 6000;

int frame = 0, timebase = 0;
float fps = 0;

int button_pressed = 0; // 1 if a button is currently being pressed.
int GLUTmouse[2] = {0, 0};

#define NUM_BUFFERS 10
vector<GLuint> buffers(NUM_BUFFERS, 0);
vector<GLuint> vaos(NUM_BUFFERS, 0);
unsigned int num_triangles;

enum {
  BUFFER_VERTICES = 0,
  BUFFER_NORMALS_PERFACE,
  BUFFER_NORMALS_PERVERTEX,
  BUFFER_VERTICESFORNORMALDRAWING,
  BUFFER_INDICES_TRIANGLES,
  BUFFER_INDICES_EDGES,
  BUFFER_INDICES_VERTICES
};
enum {
  VAO_TRIANGLES_NORMSPERVERTEX = 0,
  VAO_TRIANGLES_NORMSPERFACE,
  VAO_EDGES,
  VAO_VERTICES,
  VAO_NORMALS
};

bool smooth = false; // smooth = true means smooth normals, default false means
                     // face-wise normals.
bool drawmesh = true;
bool drawwireframe = false;
bool drawmeshvertices = false;
bool drawsilhouette = false;
bool drawnormals = false;

void makeBuffers(myMesh *input_mesh) {
  vector<GLfloat> verts;
  verts.clear();
  vector<GLfloat> norms_per_face;
  norms_per_face.clear();
  vector<GLfloat> norms;
  norms.clear();
  vector<GLfloat> verts_and_normals;
  verts_and_normals.clear();

  num_triangles = 0;
  unsigned int index = 0;
  for (unsigned int i = 0; i < input_mesh->faces.size(); i++) {
    myHalfedge *e = input_mesh->faces[i]->adjacent_halfedge->next;
    do {
      verts.push_back(
          (GLfloat)input_mesh->faces[i]->adjacent_halfedge->source->point->X);
      verts.push_back(
          (GLfloat)input_mesh->faces[i]->adjacent_halfedge->source->point->Y);
      verts.push_back(
          (GLfloat)input_mesh->faces[i]->adjacent_halfedge->source->point->Z);
      input_mesh->faces[i]->adjacent_halfedge->source->index = index;
      index++;

      verts.push_back((GLfloat)e->source->point->X);
      verts.push_back((GLfloat)e->source->point->Y);
      verts.push_back((GLfloat)e->source->point->Z);
      e->source->index = index;
      index++;

      verts.push_back((GLfloat)e->next->source->point->X);
      verts.push_back((GLfloat)e->next->source->point->Y);
      verts.push_back((GLfloat)e->next->source->point->Z);
      e->next->source->index = index;
      index++;

      norms_per_face.push_back((GLfloat)input_mesh->faces[i]->normal->dX);
      norms_per_face.push_back((GLfloat)input_mesh->faces[i]->normal->dY);
      norms_per_face.push_back((GLfloat)input_mesh->faces[i]->normal->dZ);

      norms_per_face.push_back((GLfloat)input_mesh->faces[i]->normal->dX);
      norms_per_face.push_back((GLfloat)input_mesh->faces[i]->normal->dY);
      norms_per_face.push_back((GLfloat)input_mesh->faces[i]->normal->dZ);

      norms_per_face.push_back((GLfloat)input_mesh->faces[i]->normal->dX);
      norms_per_face.push_back((GLfloat)input_mesh->faces[i]->normal->dY);
      norms_per_face.push_back((GLfloat)input_mesh->faces[i]->normal->dZ);

      norms.push_back(
          (GLfloat)input_mesh->faces[i]->adjacent_halfedge->source->normal->dX);
      norms.push_back(
          (GLfloat)input_mesh->faces[i]->adjacent_halfedge->source->normal->dY);
      norms.push_back(
          (GLfloat)input_mesh->faces[i]->adjacent_halfedge->source->normal->dZ);

      norms.push_back((GLfloat)e->source->normal->dX);
      norms.push_back((GLfloat)e->source->normal->dY);
      norms.push_back((GLfloat)e->source->normal->dZ);

      norms.push_back((GLfloat)e->next->source->normal->dX);
      norms.push_back((GLfloat)e->next->source->normal->dY);
      norms.push_back((GLfloat)e->next->source->normal->dZ);

      num_triangles++;
      e = e->next;
    } while (e->next != input_mesh->faces[i]->adjacent_halfedge);
  }

  for (unsigned int i = 0; i < input_mesh->vertices.size(); i++) {
    verts_and_normals.push_back((GLfloat)input_mesh->vertices[i]->point->X);
    verts_and_normals.push_back((GLfloat)input_mesh->vertices[i]->point->Y);
    verts_and_normals.push_back((GLfloat)input_mesh->vertices[i]->point->Z);

    verts_and_normals.push_back(
        (GLfloat)(input_mesh->vertices[i]->point->X +
                  input_mesh->vertices[i]->normal->dX / 20.0f));
    verts_and_normals.push_back(
        (GLfloat)(input_mesh->vertices[i]->point->Y +
                  input_mesh->vertices[i]->normal->dY / 20.0f));
    verts_and_normals.push_back(
        (GLfloat)(input_mesh->vertices[i]->point->Z +
                  input_mesh->vertices[i]->normal->dZ / 20.0f));
  }

  vector<GLuint> indices_edges;
  for (unsigned int i = 0; i < input_mesh->halfedges.size(); i++) {
    if (input_mesh->halfedges[i] == NULL ||
        input_mesh->halfedges[i]->next->next == NULL)
      continue;
    indices_edges.push_back(input_mesh->halfedges[i]->source->index);
    indices_edges.push_back(input_mesh->halfedges[i]->next->source->index);
  }

  vector<GLuint> indices_vertices;
  for (unsigned int i = 0; i < input_mesh->vertices.size(); i++)
    indices_vertices.push_back(input_mesh->vertices[i]->index);

  glDeleteBuffers(NUM_BUFFERS, &buffers[0]);
#ifdef __APPLE__
  if (GLEW_APPLE_vertex_array_object)
    glDeleteVertexArrays(NUM_BUFFERS, &vaos[0]);
#else
  glDeleteVertexArrays(NUM_BUFFERS, &vaos[0]);
#endif

  buffers.assign(buffers.size(), 0);
  vaos.assign(vaos.size(), 0);

  glGenBuffers(NUM_BUFFERS, &buffers[0]);

  glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTICES]);
  glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), &verts[0],
               GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_NORMALS_PERVERTEX]);
  glBufferData(GL_ARRAY_BUFFER, norms.size() * sizeof(GLfloat), &norms[0],
               GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_NORMALS_PERFACE]);
  glBufferData(GL_ARRAY_BUFFER, norms_per_face.size() * sizeof(GLfloat),
               &norms_per_face[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTICESFORNORMALDRAWING]);
  glBufferData(GL_ARRAY_BUFFER, verts_and_normals.size() * sizeof(GLfloat),
               &verts_and_normals[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[BUFFER_INDICES_EDGES]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_edges.size() * sizeof(GLuint),
               &indices_edges[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[BUFFER_INDICES_VERTICES]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               indices_vertices.size() * sizeof(GLuint), &indices_vertices[0],
               GL_STATIC_DRAW);

  bool use_vao = true;
#ifdef __APPLE__
  use_vao = (GLEW_APPLE_vertex_array_object != 0);
#endif
  if (!use_vao) {
    vaos.assign(vaos.size(), 0);
    return;
  }

  glGenVertexArrays(NUM_BUFFERS, &vaos[0]);

  glBindVertexArray(vaos[VAO_TRIANGLES_NORMSPERVERTEX]);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTICES]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_NORMALS_PERVERTEX]);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);
  glBindVertexArray(0);

  glBindVertexArray(vaos[VAO_TRIANGLES_NORMSPERFACE]);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTICES]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_NORMALS_PERFACE]);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);
  glBindVertexArray(0);

  glBindVertexArray(vaos[VAO_EDGES]);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTICES]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_NORMALS_PERVERTEX]);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[BUFFER_INDICES_EDGES]);
  glBindVertexArray(0);

  glBindVertexArray(vaos[VAO_VERTICES]);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTICES]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_NORMALS_PERVERTEX]);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[BUFFER_INDICES_VERTICES]);
  glBindVertexArray(0);

  glBindVertexArray(vaos[VAO_NORMALS]);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTICESFORNORMALDRAWING]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);
}

void draw_text(GLfloat x, GLfloat y, GLfloat z, string text,
               vector<GLfloat> color, void *font = GLUT_BITMAP_TIMES_ROMAN_24) {
  glUseProgram(0);

  GLfloat w = 1;
  GLfloat fx, fy;
  glColor3f(color[0], color[1], color[2]);

  glPushAttrib(GL_TRANSFORM_BIT | GL_VIEWPORT_BIT);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glDepthRange(z, z);
  glViewport((int)x - 1, (int)y - 1, 2, 2);

  fx = x - (int)x;
  fy = y - (int)y;
  glRasterPos4f(fx, fy, 0.0, w);

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glPopAttrib();

  for (const char *c = text.c_str(); *c != '\0'; c++)
    glutBitmapCharacter(font, *c);

  glUseProgram(shaderprogram);
}

bool PickedPoint(int x, int y) {
  if ((x < 0) || (Glut_w <= x) || (y < 0) || (Glut_h <= y)) {
    printf("Pick (%d,%d) outside viewport: (0,%d) (0,%d)\n", x, y, Glut_w,
           Glut_h);
    return false;
  }
  return false;
}

// This function is called when a mouse button is pressed.
void mouse(int button, int state, int x, int y) {
  button_pressed = (state == GLUT_DOWN) ? 1 : 0;

  GLUTmouse[0] = x;
  GLUTmouse[1] = Glut_h - y;

  int mode = glutGetModifiers();
  if (state == GLUT_DOWN) {
    if (button == GLUT_LEFT_BUTTON) {
      if (mode == GLUT_ACTIVE_CTRL) {
        menu(MENU_SELECTVERTEX);
      }
    }
  }

  glutPostRedisplay();
}

// This function is called when the mouse is dragged.
void mousedrag(int x, int y) {
  y = Glut_h - y;

  int dx = x - GLUTmouse[0];
  int dy = y - GLUTmouse[1];

  GLUTmouse[0] = x;
  GLUTmouse[1] = y;

  if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
    return;
  if ((dx == 0 && dy == 0) || !button_pressed)
    return;

  double vx = (double)dx / (double)Glut_w;
  double vy = (double)dy / (double)Glut_h;
  double theta = 4.0 * (fabs(vx) + fabs(vy));

  myVector3D camera_right = camera_forward.crossproduct(camera_up);
  camera_right.normalize();

  myVector3D tomovein_direction = -camera_right * vx + -camera_up * vy;

  myVector3D rotation_axis = tomovein_direction.crossproduct(camera_forward);
  rotation_axis.normalize();

  camera_forward.rotate(rotation_axis, theta);
  camera_up.rotate(rotation_axis, theta);
  camera_eye.rotate(rotation_axis, theta);

  camera_up.normalize();
  camera_forward.normalize();

  glutPostRedisplay();
}

void mouseWheel(int button, int dir, int x, int y) {
  if (dir > 0)
    camera_eye += camera_forward * 0.02;
  else
    camera_eye += -camera_forward * 0.02;
  glutPostRedisplay();
}

// This function is called when an arrow key is pressed.
void keyboard2(int key, int x, int y) {
  switch (key) {
  case GLUT_KEY_UP:
    camera_eye += camera_forward * 0.01;
    break;
  case GLUT_KEY_DOWN:
    camera_eye += -camera_forward * 0.01;
    break;
  case GLUT_KEY_LEFT:
    camera_up.normalize();
    camera_forward.rotate(camera_up, 0.01);
    camera_forward.normalize();
    break;
  case GLUT_KEY_RIGHT:
    camera_up.normalize();
    camera_forward.rotate(camera_up, -0.01);
    camera_forward.normalize();
    break;
  }
  glutPostRedisplay();
}

void timerFunc(int lastTime) {
  frame++;
  int time = glutGet(GLUT_ELAPSED_TIME);

  int diffTime = time - timebase;
  if (diffTime > 200) {
    fps = frame * 1000.0f / diffTime;
    timebase = time;
    frame = 0;
  }
  glutPostRedisplay();
  glutTimerFunc((1000 + lastTime - time) / 60, timerFunc, time);
}

void reshape(int width, int height) {
  Glut_w = width;
  Glut_h = height;

  glutPostRedisplay();
}

// This function is called when a key is pressed.
void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 27: // Escape to quit
    m->clear();
    glDeleteBuffers(10, &buffers[0]);
    glDeleteVertexArrays(10, &vaos[0]);
    exit(0);
    break;
  case 'r':
    camera_eye = myPoint3D(0, 0, 2);
    camera_forward = myVector3D(0, 0, -1);
    camera_up = myVector3D(0, 1, 0);
    break;
  case 'h':
    cout << "The keys for various algorithms are:\n";
    break;
  }
  glutPostRedisplay();
}

string textFileRead(const char *filename) {
  string str, ret = "";
  ifstream in;
  in.open(filename);
  if (in.is_open()) {
    getline(in, str);
    while (in) {
      ret += str + "\n";
      getline(in, str);
    }
    return ret;
  } else {
    cerr << "Unable to Open File " << filename << "\n";
    throw 2;
  }
}

void programerrors(const GLint program) {
  GLint length;
  GLchar *log;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
  log = new GLchar[length + 1];
  glGetProgramInfoLog(program, length, &length, log);
  cout << "Compile Error, Log Below\n" << log << "\n";
  delete[] log;
}
void shadererrors(const GLint shader) {
  GLint length;
  GLchar *log;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
  log = new GLchar[length + 1];
  glGetShaderInfoLog(shader, length, &length, log);
  cout << "Compile Error, Log Below\n" << log << "\n";
  delete[] log;
}

GLuint initshaders(GLenum type, const char *filename) {
  GLuint shader = glCreateShader(type);
  GLint compiled;
  string str = textFileRead(filename);
  GLchar *cstr = new GLchar[str.size() + 1];
  const GLchar *cstr2 = cstr;
  strncpy(cstr, str.c_str(), str.size() + 1);
  glShaderSource(shader, 1, &cstr2, NULL);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    shadererrors(shader);
    throw 3;
  }
  return shader;
}

GLuint initprogram(GLuint vertexshader, GLuint fragmentshader) {
  GLuint program = glCreateProgram();
  GLint linked;
  glAttachShader(program, vertexshader);
  glAttachShader(program, fragmentshader);
  glBindAttribLocation(program, 0, "vertex_modelspace");
  glBindAttribLocation(program, 1, "normal_modelspace");
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (linked)
    glUseProgram(program);
  else {
    programerrors(program);
    throw 4;
  }
  return program;
}

void initInterface(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
  glutCreateWindow("My 4I-IG3 Application!");

  glewExperimental = GL_TRUE;
  glewInit();
  glutReshapeWindow(Glut_w, Glut_h);

  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(keyboard2);
  glutMotionFunc(mousedrag);
  glutMouseFunc(mouse);
#ifndef __APPLE__
  glutMouseWheelFunc(mouseWheel);
#endif
  glutTimerFunc(1000.0 / 60, timerFunc, 0);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glClearColor(1, 1, 1, 0);

  glEnable(GL_MULTISAMPLE);

  GLuint vertexshader =
      initshaders(GL_VERTEX_SHADER, "shaders/light.vert.glsl");
  GLuint fragmentshader =
      initshaders(GL_FRAGMENT_SHADER, "shaders/light.frag.glsl");
  shaderprogram = initprogram(vertexshader, fragmentshader);

  int sm1 = glutCreateMenu(menu);
  glutAddMenuEntry("Vertex-shading/face-shading", MENU_SHADINGTYPE);
  glutAddMenuEntry("Mesh", MENU_DRAWMESH);
  glutAddMenuEntry("Wireframe", MENU_DRAWWIREFRAME);
  glutAddMenuEntry("Vertices", MENU_DRAWMESHVERTICES);
  glutAddMenuEntry("Normals", MENU_DRAWNORMALS);
  glutAddMenuEntry("Silhouette", MENU_DRAWSILHOUETTE);
  glutAddMenuEntry("Crease", MENU_DRAWCREASE);

  int sm2 = glutCreateMenu(menu);
  glutAddMenuEntry("Catmull-Clark subdivision", MENU_CATMULLCLARK);
  glutAddMenuEntry("Loop subdivision", MENU_LOOP);
  glutAddMenuEntry("Inflate", MENU_INFLATE);
  glutAddMenuEntry("Smoothen", MENU_SMOOTHEN);
  glutAddMenuEntry("Simplification", MENU_SIMPLIFY);

  int sm3 = glutCreateMenu(menu);
  glutAddMenuEntry("Closest Edge", MENU_SELECTEDGE);
  glutAddMenuEntry("Closest Face", MENU_SELECTFACE);
  glutAddMenuEntry("Closest Vertex", MENU_SELECTVERTEX);
  glutAddMenuEntry("Clear", MENU_SELECTCLEAR);

  int sm4 = glutCreateMenu(menu);
  glutAddMenuEntry("Split edge", MENU_SPLITEDGE);
  glutAddMenuEntry("Split face", MENU_SPLITFACE);
  glutAddMenuEntry("Contract edge", MENU_CONTRACTEDGE);
  glutAddMenuEntry("Contract face", MENU_CONTRACTFACE);

  int mn = glutCreateMenu(menu);
  glutAddSubMenu("Draw", sm1);
  glutAddSubMenu("Mesh Operations", sm2);
  glutAddSubMenu("Select", sm3);
  glutAddSubMenu("Face Operations", sm4);
  glutAddMenuEntry("Open File", MENU_OPENFILE);
  glutAddMenuEntry("Triangulate", MENU_TRIANGULATE);
  glutAddMenuEntry("Write to File", MENU_WRITE);
  glutAddMenuEntry("Undo", MENU_UNDO);
  glutAddMenuEntry("Generate Mesh", MENU_GENERATE);
  glutAddMenuEntry("Cut Mesh", MENU_CUT);
  glutAddMenuEntry("Exit", MENU_EXIT);

  glutAttachMenu(GLUT_RIGHT_BUTTON);
}
