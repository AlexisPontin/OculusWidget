#include "oculuswidget.hpp"
#include <Qdebug>
#include <QtGlobal>
#include <QTimer>
#include <QGLShader>
#include <QDateTime>

QImage img("D:/Data/My Videos/equi.png");

OculusWidget::OculusWidget(QGLWidget *parent) :
  QGLWidget(parent)
  /*textID(-1)*/ {

  setAutoBufferSwap(false);

  //  QGLFormat fmt;
  //  fmt.setStereo(true);
  //  context = new QGLContext(fmt);
  //  this->setContext(context);

  initOculus();
}

OculusWidget::~OculusWidget() {
  ovrHmd_Destroy(oculus);
  ovr_Shutdown();
}

void OculusWidget::initOculus() {

  ovr_Initialize();

  oculus = ovrHmd_Create(0);
  if (!oculus){
    oculus = ovrHmd_CreateDebug(ovrHmd_DK1);
  }

  ovrHmd_GetDesc(oculus, &oculusInfos);

  if(!ovrHmd_StartSensor(oculus, ovrHmdCap_Orientation, 0)){
    qDebug() << "could not start sensor";
    exit(1);
  }

  resize(oculusInfos.Resolution.w, oculusInfos.Resolution.h);
}
void OculusWidget::SetOpenGLState(){
  // Some state...
  glEnable(GL_CULL_FACE);
//  glEnable(GL_LIGHTING);
//  glDisable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
//  glShadeModel(GL_SMOOTH);
//  glEnable(GL_BLEND);
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//  // Some (stationary) lights...
//  GLfloat l_Light0Position[] = { 5.0f, 6.0f, 3.0f, 0.0f };
//  GLfloat l_Light0Diffuse[] = { 1.0f, 0.8f, 0.6f, 1.0f };
//  glLightfv(GL_LIGHT0, GL_POSITION, l_Light0Position);
//  glLightfv(GL_LIGHT0, GL_DIFFUSE, l_Light0Diffuse);
//  glEnable(GL_LIGHT0);

//  GLfloat l_Light1Position[] = { -5.0f, -6.0f, 5.0f, 0.0f };
//  GLfloat l_Light1Diffuse[] = { 0.6f, 0.8f, 1.0f, 1.0f };
//  glLightfv(GL_LIGHT1, GL_POSITION, l_Light1Position);
//  glLightfv(GL_LIGHT1, GL_DIFFUSE, l_Light1Diffuse);
//  glEnable(GL_LIGHT1);

////   Material...
//    GLfloat l_MaterialSpecular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
//    GLfloat l_MaterialShininess[] = { 10.0f };
//    glMaterialfv(GL_FRONT, GL_SPECULAR, l_MaterialSpecular);
//    glMaterialfv(GL_FRONT, GL_SHININESS, l_MaterialShininess);
}
void OculusWidget::initTexture() {
  // Create FBO...
  glGenFramebuffers(1, &FBOId);
  glBindFramebuffer(GL_FRAMEBUFFER, FBOId);

  // The texture we're going to render to...
  glGenTextures(2, textID);
  //  glActiveTexture(GL_TEXTURE0);
  //   "Bind" the newly created texture : all future texture functions will modify this texture...

//    glActiveTexture(GL_TEXTURE1);
//    glBindTexture(GL_TEXTURE_2D, textID[1]);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textID[0]);
  // Give an empty image to OpenGL (the last "0")
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, renderTargetSize.w, renderTargetSize.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  // Linear filtering...
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // Create Depth Buffer...
  glGenRenderbuffers(1, &DepthBufferId);
  glBindRenderbuffer(GL_RENDERBUFFER, DepthBufferId);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, renderTargetSize.w, renderTargetSize.h);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, DepthBufferId);

  // Set the texture as our colour attachment #0...
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textID[0], 0);

  // Set the list of draw buffers...
  GLenum GLDrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, GLDrawBuffers); // "1" is the size of DrawBuffers

  // Unbind...
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  Q_ASSERT(!glGetError());

}
void OculusWidget::initRender() {

  this->makeCurrent();

  //left eye
  eyes[0].Eye = ovrEye_Left;
  eyes[1].Eye = ovrEye_Right;
  eyes[0].Fov = oculusInfos.DefaultEyeFov[0];
  eyes[1].Fov = oculusInfos.DefaultEyeFov[1];
  eyes[0].TextureSize = renderTargetSize;
  eyes[1].TextureSize = renderTargetSize;
  eyes[0].RenderViewport.Pos = OVR::Vector2i(0,0);
  eyes[1].RenderViewport.Pos = OVR::Vector2i((renderTargetSize.w + 1)/2,0);
  eyes[0].RenderViewport.Size = OVR::Sizei(renderTargetSize.w/2 , renderTargetSize.h);
  eyes[1].RenderViewport.Size = eyes[0].RenderViewport.Size;

  eyeTextureGL[0].OGL.Header.API = ovrRenderAPI_OpenGL;
  eyeTextureGL[0].OGL.Header.TextureSize = renderTargetSize;
  eyeTextureGL[0].OGL.Header.RenderViewport = eyes[0].RenderViewport;
  eyeTextureGL[0].OGL.TexId = textID[0];

  eyeTextureGL[1] = eyeTextureGL[0];
  eyeTextureGL[1].OGL.Header.RenderViewport = eyes[1].RenderViewport;

  const int backBufferMultisample = 0;
  cfg.OGL.Header.API =          ovrRenderAPI_OpenGL;
  cfg.OGL.Header.RTSize =       oculusInfos.Resolution; /*OVR::Sizei(oculusInfos.Resolution.w, oculusInfos.Resolution.h);*/
  cfg.OGL.Header.Multisample =  backBufferMultisample;
  cfg.OGL.Window =              reinterpret_cast<HWND>(winId());
  cfg.OGL.WglContext =          wglGetCurrentContext();
  cfg.OGL.GdiDc =               wglGetCurrentDC();

  qDebug() << "Window:" << cfg.OGL.Window;
  qDebug() << "Context:" << cfg.OGL.WglContext;
  qDebug() << "DC:" << cfg.OGL.GdiDc;

  if (!ovrHmd_ConfigureRendering(oculus, &cfg.Config, ovrHmdCap_NoVSync, ovrDistortion_Chromatic , eyes,  eyeRenderDesc)){
    qDebug() << "failed to configure rendering";
    exit(1);
  }
}
void OculusWidget::initShaders() {
  // Override system locale until shaders are compiled
  setlocale(LC_NUMERIC, "C");

  // Compile vertex shader
  if (!program.addShaderFromSourceFile(QGLShader::Vertex, "vertexShader.glsl"))
    close();

  // Compile fragment shader
  if (!program.addShaderFromSourceFile(QGLShader::Fragment, "fragmentShader.glsl"))
    close();

  // Link shader pipeline
  if (!program.link())
    close();

  // Bind shader pipeline for use
  if (!program.bind())
    close();

  // Restore system locale
  setlocale(LC_ALL, "");
}

void OculusWidget::initializeGL() {

  initializeOpenGLFunctions();
  glClearColor(1,0,0,0);

  ovrSizei recommendedTex0Size = ovrHmd_GetFovTextureSize(oculus, ovrEye_Left,  oculusInfos.DefaultEyeFov[0], 1.0f);
  ovrSizei recommendedTex1Size = ovrHmd_GetFovTextureSize(oculus, ovrEye_Right, oculusInfos.DefaultEyeFov[1], 1.0f);;
  renderTargetSize.w = recommendedTex0Size.w + recommendedTex1Size.w;
  renderTargetSize.h = qMax(recommendedTex0Size.h, recommendedTex1Size.h);

  SetOpenGLState();
  initShaders();
  initTexture();
  geometryEngine.init();
  initRender();
}

void OculusWidget::resizeGL(int width, int height) {
  w = width;
  h = height;
  glViewport(0,0,w, h);
}

void OculusWidget::paintGL() {

  const int ms(1000/60 /*fps*/);
  QTimer::singleShot(ms, this, SLOT(updateGL()));

  qint64 tmp = QDateTime::currentMSecsSinceEpoch() /10;
  GLfloat l_SpinX = (GLfloat) (tmp % 360);
  GLfloat l_SpinY = (GLfloat) (tmp% 360);

  ovrFrameTiming m_HmdFrameTiming = ovrHmd_BeginFrame(oculus, 0);

  // Bind the FBO...
  glBindFramebuffer(GL_FRAMEBUFFER, FBOId);
  glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (int l_EyeIndex=0; l_EyeIndex<ovrEye_Count; l_EyeIndex++)
  {
    ovrEyeType l_Eye = oculusInfos.EyeRenderOrder[l_EyeIndex];
    ovrPosef l_EyePose = ovrHmd_BeginEyeRender(oculus, l_Eye);

    glViewport(eyeRenderDesc[l_Eye].Desc.RenderViewport.Pos.x,   // StartX
               eyeRenderDesc[l_Eye].Desc.RenderViewport.Pos.y,   // StartY
               eyeRenderDesc[l_Eye].Desc.RenderViewport.Size.w,   // Width
               eyeRenderDesc[l_Eye].Desc.RenderViewport.Size.h   // Height
               );

    // Get Projection and ModelView matrici from the device...
    OVR::Matrix4f l_ProjectionMatrix = ovrMatrix4f_Projection(eyeRenderDesc[l_Eye].Desc.Fov, 0.3f, 100.0f, true);
    OVR::Quatf l_Orientation = OVR::Quatf(l_EyePose.Orientation);
    OVR::Matrix4f l_ModelViewMatrix = OVR::Matrix4f(l_Orientation);

    //   Pass matrici on to OpenGL...

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Translate for specific eye based on IPD...
    glTranslatef(eyeRenderDesc[l_Eye].ViewAdjust.x,
                 eyeRenderDesc[l_Eye].ViewAdjust.y,
                 eyeRenderDesc[l_Eye].ViewAdjust.z);

    // Multiply with orientation retrieved from sensor...
    glTranslatef(-0.00f, 0.0f, -2.0f);
    glMultMatrixf(&(l_ModelViewMatrix.Transposed().M[0][0]));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMultMatrixf(&(l_ProjectionMatrix.Transposed().M[0][0]));

    geometryEngine.drawSphereGeometry(&program);

    ovrHmd_EndEyeRender(oculus, l_Eye, l_EyePose, &eyeTextureGL[l_Eye].Texture);

  }

  //  // Unbind the FBO, back to normal drawing...
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Oculus wants CW orientations, avoid the problem by turning of culling...
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  ovrHmd_EndFrame(oculus);
  // Restore state after Oculus did it's work (-sigh-)...
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glClearDepth(1);
  glUseProgram(0);


}

void OculusWidget::renderCube() {


  // A cube...
  glBegin(GL_QUADS);        // Draw The Cube Using quads
      glColor3f(0.0f,1.0f,0.0f);    // Color Blue
      glVertex3f( 1.0f, 1.0f,-1.0f);    // Top Right Of The Quad (Top)
      glVertex3f(-1.0f, 1.0f,-1.0f);    // Top Left Of The Quad (Top)
      glVertex3f(-1.0f, 1.0f, 1.0f);    // Bottom Left Of The Quad (Top)
      glVertex3f( 1.0f, 1.0f, 1.0f);    // Bottom Right Of The Quad (Top)
      glColor3f(1.0f,0.5f,0.0f);    // Color Orange
      glVertex3f( 1.0f,-1.0f, 1.0f);    // Top Right Of The Quad (Bottom)
      glVertex3f(-1.0f,-1.0f, 1.0f);    // Top Left Of The Quad (Bottom)
      glVertex3f(-1.0f,-1.0f,-1.0f);    // Bottom Left Of The Quad (Bottom)
      glVertex3f( 1.0f,-1.0f,-1.0f);    // Bottom Right Of The Quad (Bottom)
      glColor3f(1.0f,0.0f,0.0f);    // Color Red
      glVertex3f( 1.0f, 1.0f, 1.0f);    // Top Right Of The Quad (Front)
      glVertex3f(-1.0f, 1.0f, 1.0f);    // Top Left Of The Quad (Front)
      glVertex3f(-1.0f,-1.0f, 1.0f);    // Bottom Left Of The Quad (Front)
      glVertex3f( 1.0f,-1.0f, 1.0f);    // Bottom Right Of The Quad (Front)
      glColor3f(1.0f,1.0f,0.0f);    // Color Yellow
      glVertex3f( 1.0f,-1.0f,-1.0f);    // Top Right Of The Quad (Back)
      glVertex3f(-1.0f,-1.0f,-1.0f);    // Top Left Of The Quad (Back)
      glVertex3f(-1.0f, 1.0f,-1.0f);    // Bottom Left Of The Quad (Back)
      glVertex3f( 1.0f, 1.0f,-1.0f);    // Bottom Right Of The Quad (Back)
      glColor3f(0.0f,0.0f,1.0f);    // Color Blue
      glVertex3f(-1.0f, 1.0f, 1.0f);    // Top Right Of The Quad (Left)
      glVertex3f(-1.0f, 1.0f,-1.0f);    // Top Left Of The Quad (Left)
      glVertex3f(-1.0f,-1.0f,-1.0f);    // Bottom Left Of The Quad (Left)
      glVertex3f(-1.0f,-1.0f, 1.0f);    // Bottom Right Of The Quad (Left)
      glColor3f(1.0f,0.0f,1.0f);    // Color Violet
      glVertex3f( 1.0f, 1.0f,-1.0f);    // Top Right Of The Quad (Right)
      glVertex3f( 1.0f, 1.0f, 1.0f);    // Top Left Of The Quad (Right)
      glVertex3f( 1.0f,-1.0f, 1.0f);    // Bottom Left Of The Quad (Right)
      glVertex3f( 1.0f,-1.0f,-1.0f);    // Bottom Right Of The Quad (Right)
    glEnd();            // End Drawing The Cube - See more at: http://www.codemiles.com/c-opengl-examples/draw-3d-cube-using-opengl-t9018.html#sthash.ABNhayCR.dpuf
}

