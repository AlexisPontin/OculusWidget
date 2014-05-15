#ifndef OCULUSWIDGET_HPP
#define OCULUSWIDGET_HPP

#include <QtOpenGL>
#include <QGLWidget>
#include "geometryengine.hpp"
#include <QOpenGLFunctions_4_3_Core>
#include "OVR.h"
#include "../src/OVR_CAPI_GL.h"

class GeometryEngine;

class OculusWidget : public QGLWidget, protected QOpenGLFunctions_4_3_Core
{
  Q_OBJECT
public:
  OculusWidget(QGLWidget *parent = 0);
  ~OculusWidget();

private:
  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();

  void initOculus();
  void SetOpenGLState();
  void initTexture();
  void initRender();
  void initShaders();

  void renderCube();

  QGLContext *context;

  ovrHmd              oculus;
  ovrHmdDesc          oculusInfos;

  ovrEyeDesc          eyes[2];
  ovrEyeRenderDesc    eyeRenderDesc[2];

  ovrGLTexture        eyeTextureGL[2];
  ovrSizei            renderTargetSize;

  ovrGLConfig         cfg;

  GLuint            textID[2];
  GLuint            FBOId;
  GLuint            DepthBufferId;

  QMatrix4x4        projection;
  GeometryEngine    geometryEngine;
  QGLShaderProgram  program;

  int w,h;

};

#endif // OCULUSWIDGET_HPP
