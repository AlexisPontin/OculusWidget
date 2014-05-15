#ifndef GEOMETRYENGINE_H
#define GEOMETRYENGINE_H

#include <QOpenGLFunctions_4_3_Core>
#include <QGLShaderProgram>
#include <QOpenGLBuffer>

class GeometryEngine : protected QOpenGLFunctions_4_3_Core
{
public:
  GeometryEngine();
  virtual ~GeometryEngine();

  void init();
  void drawSphereGeometry(QGLShaderProgram *program);

private:
  void initSphereGeometry(float hfov, float vfov);

  QOpenGLBuffer vbo;
};

#endif // GEOMETRYENGINE_H
