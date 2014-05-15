#include "geometryengine.hpp"
#include <qmath.h>
#include <QVector>

#define NUM_PARALLELS 80
#define NUM_MERIDIANS 60
#define DEFAULT_FOV   120
#define FOV_MAX       140
#define FOV_MIN       5


/*************************************/
/********* Construc/Destruc **********/
/*************************************/

GeometryEngine::GeometryEngine() : vbo(QOpenGLBuffer::VertexBuffer) {

}

GeometryEngine::~GeometryEngine() {
  vbo.destroy();
}

void GeometryEngine::init() {

  initializeOpenGLFunctions();
  vbo.create();
  initSphereGeometry(360.f, 180.f);
}

void GeometryEngine::initSphereGeometry(float hfov, float vfov) {

  float rHFov = hfov * M_PI / 180;
  float rVFov = vfov * M_PI / 180;
  QVector<GLfloat> data;

  for (int i = 0; i <= NUM_PARALLELS; i++) {
    float v0 = (i - 1) / (float)(NUM_PARALLELS);
    float lat0 = rVFov * (-0.5f + v0);
    float z0  = sinf(lat0);
    float zr0 =  cosf(lat0);

    float v1 = i / (float)(NUM_PARALLELS);
    float lat1 = rVFov * (-0.5f + v1);
    float z1 = sinf(lat1);
    float zr1 = cosf(lat1);

    for (int j = 0; j <= NUM_MERIDIANS; j++) {
      float u = (j - 1) / (float)NUM_MERIDIANS;
      float lng = rHFov * u;
      float x = cosf(lng);
      float y = sinf(lng);

      data.push_back(x * zr0); //X
      data.push_back(y * zr0); //Y
      data.push_back(z0);      //Z

      data.push_back(u); //U
      data.push_back(v0);  //V

      data.push_back(x * zr1); //X
      data.push_back(y * zr1); //Y
      data.push_back(z1);      //Z

      data.push_back(u); //U
      data.push_back(v1);  //V

    }
  }
  vbo.bind();
  vbo.allocate(&data.front(), data.size() * 4);
  vbo.release();
}

void GeometryEngine::drawSphereGeometry(QGLShaderProgram *program) {

    vbo.bind();

    int vertexLocation = program->attributeLocation("a_position");
    program->enableAttributeArray(vertexLocation);
    glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

    int texcoordLocation = program->attributeLocation("a_texcoord");
    program->enableAttributeArray(texcoordLocation);
    glVertexAttribPointer(texcoordLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const void*)(3 * sizeof(float)));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 2 * (NUM_PARALLELS + 1) * (NUM_MERIDIANS + 1));

    vbo.release();
}
