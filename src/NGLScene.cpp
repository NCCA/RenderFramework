#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/Camera.h>
#include <ngl/Light.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>


//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for x/y translation with mouse movement
//----------------------------------------------------------------------------------------------------------------------
const static float INCREMENT=0.01f;
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//----------------------------------------------------------------------------------------------------------------------
const static float ZOOM=0.1f;

NGLScene::NGLScene(QWindow *_parent) : QOpenGLWindow(_parent)
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  m_rotate=false;
  // mouse rotation values set to 0
  m_spinXFace=0.0f;
  m_spinYFace=0.0f;
  setTitle("Qt5 Simple NGL Demo");
  m_buffer=0;
  m_rot=0.0f;
   m_freq=1.0;
   m_animate=true;
}


NGLScene::~NGLScene()
{
  ngl::NGLInit *Init = ngl::NGLInit::instance();
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
  delete m_framebuffer;
  Init->NGLQuit();

}

void NGLScene::resizeEvent(QResizeEvent *_event )
{
  if(isExposed())
  {
  // set the viewport for openGL we need to take into account retina display
  // etc by using the pixel ratio as a multiplyer
  glViewport(0,0,width()*devicePixelRatio(),height()*devicePixelRatio());
  // now set the camera size values as the screen size has changed
  m_cam->setShape(45.0f,(float)width()/height(),0.05f,350.0f);
  renderLater();
  }
}


void NGLScene::initializeLG()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::instance();

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
   // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // we are creating a shader for Pass 1
  shader->createShaderProgram("Pass1");
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader("Pass1Vertex",ngl::ShaderType::VERTEX);
  shader->attachShader("Pass1Fragment",ngl::ShaderType::FRAGMENT);
  // attach the source
  shader->loadShaderSource("Pass1Vertex","shaders/Pass1Vert.glsl");
  shader->loadShaderSource("Pass1Fragment","shaders/Pass1Frag.glsl");
  // compile the shaders
  shader->compileShader("Pass1Vertex");
  shader->compileShader("Pass1Fragment");
  // add them to the program
  shader->attachShaderToProgram("Pass1","Pass1Vertex");
  shader->attachShaderToProgram("Pass1","Pass1Fragment");

  shader->linkProgramObject("Pass1");


  // we are creating a shader for Pass 1
  shader->createShaderProgram("LightPass");
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader("LightingPassVertex",ngl::ShaderType::VERTEX);
  shader->attachShader("LightingPassFragment",ngl::ShaderType::FRAGMENT);
  // attach the source
  shader->loadShaderSource("LightingPassVertex","shaders/LightingPassVert.glsl");
  shader->loadShaderSource("LightingPassFragment","shaders/LightingPassFrag.glsl");
  // compile the shaders
  shader->compileShader("LightingPassVertex");
  shader->compileShader("LightingPassFragment");
  // add them to the program
  shader->attachShaderToProgram("LightPass","LightingPassVertex");
  shader->attachShaderToProgram("LightPass","LightingPassFragment");

  shader->linkProgramObject("LightPass");
  shader->use("LightPass");
  shader->setShaderParam1i("pointTex",0);
  shader->setShaderParam1i("normalTex",1);
  shader->setShaderParam1i("colourTex",2);

//
  // we are creating a shader for Pass 1
  shader->createShaderProgram("null");
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader("nullVertex",ngl::ShaderType::VERTEX);
  shader->attachShader("nullFragment",ngl::ShaderType::FRAGMENT);
  // attach the source
  shader->loadShaderSource("nullVertex","shaders/NullVertex.glsl");
  shader->loadShaderSource("nullFragment","shaders/NullFragment.glsl");
  // compile the shaders
  shader->compileShader("nullVertex");
  shader->compileShader("nullFragment");
  // add them to the program
  shader->attachShaderToProgram("null","nullVertex");
  shader->attachShaderToProgram("null","nullFragment");

  shader->linkProgramObject("null");



  // as re-size is not explicitly called we need to do this.
  // set the viewport for openGL we need to take into account retina display
  // etc by using the pixel ratio as a multiplyer
  glViewport(0,0,width()*devicePixelRatio(),height()*devicePixelRatio());
  m_framebuffer = new FrameBuffer(width()*devicePixelRatio(),height()*devicePixelRatio());
  m_framebuffer->bind();
  //m_framebuffer->attatchDepthBuffer();
  //m_framebuffer->attatchStencilBuffer();
  m_framebuffer->attatchDepthAndStencilBuffer();
  m_framebuffer->attatchTexture("point");
  m_framebuffer->attatchTexture("normal");
  m_framebuffer->attatchTexture("colour");
  m_framebuffer->attatchTexture("shading");
  m_framebuffer->attatchTexture("final");
  m_framebuffer->debug();
  m_framebuffer->unbind();
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  prim->createSphere("sphere",0.5,50);

  prim->createSphere("lightSphere",1.0,20);
  prim->createCylinder("cylinder",0.5,1.4,40,40);

  prim->createCone("cone",0.5,1.4,20,20);

  prim->createDisk("disk",0.8,120);
  prim->createTorus("torus",0.15,0.4,40,40);
  prim->createTrianglePlane("plane",14,14,80,80,ngl::Vec3(0,1,0));
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0,2,5);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);
  // now load to our new camera
  m_cam= new ngl::Camera(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam->setShape(45,(float)float(width()/height()),0.05,350);
  startTimer(10);
}


void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use("Pass1");
 // ngl::Mat4 view=ngl::lookAt(ngl::Vec3(0,1,5),ngl::Vec3(0,0,0),ngl::Vec3(0,1,0));
 // ngl::Mat4 proj=ngl::perspective(45,float(width()/height()),0.1,50);
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat4 M;

  M=m_transform.getMatrix()*m_mouseGlobalTX;
  MV=  M*m_cam->getViewMatrix();
  MVP=  MV*m_cam->getProjectionMatrix();


  shader->setShaderParamFromMat4("MVP",MVP);
  shader->setShaderParamFromMat4("MV",MV);
}

void NGLScene::paintGL()
{
  // clear the screen and depth buffer

  // grab an instance of the shader manager
 // ngl::ShaderLib *shader=ngl::ShaderLib::instance();

  m_framebuffer->bind();
  m_framebuffer->setDrawBuffers();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //glViewport(0,0,m_framebuffer->width(),m_framebuffer->height());

  // Only the geometry pass updates the depth buffer
  glDepthMask(GL_TRUE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  drawScene("Pass1");

  glDepthMask(GL_FALSE);
  glEnable(GL_STENCIL_TEST);

  stencilPass();
  lightPass();
  m_framebuffer->unbind();
  glDisable(GL_STENCIL_TEST);


  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer->id());

  glReadBuffer(GL_COLOR_ATTACHMENT0+m_buffer);
  int w=width()*devicePixelRatio();
  int h=height()*devicePixelRatio();

  glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_LINEAR);


}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent (QMouseEvent * _event)
{
  // note the method buttons() is the button state when event was called
  // this is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if(m_rotate && _event->buttons() == Qt::LeftButton)
  {
    int diffx=_event->x()-m_origX;
    int diffy=_event->y()-m_origY;
    m_spinXFace += (float) 0.5f * diffy;
    m_spinYFace += (float) 0.5f * diffx;
    m_origX = _event->x();
    m_origY = _event->y();
    renderLater();

  }
        // right mouse translate code
  else if(m_translate && _event->buttons() == Qt::RightButton)
  {
    int diffX = (int)(_event->x() - m_origXPos);
    int diffY = (int)(_event->y() - m_origYPos);
    m_origXPos=_event->x();
    m_origYPos=_event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    renderLater();

   }
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent ( QMouseEvent * _event)
{
  // this method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if(_event->button() == Qt::LeftButton)
  {
    m_origX = _event->x();
    m_origY = _event->y();
    m_rotate =true;
  }
  // right mouse translate mode
  else if(_event->button() == Qt::RightButton)
  {
    m_origXPos = _event->x();
    m_origYPos = _event->y();
    m_translate=true;
  }

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent ( QMouseEvent * _event )
{
  // this event is called when the mouse button is released
  // we then set Rotate to false
  if (_event->button() == Qt::LeftButton)
  {
    m_rotate=false;
  }
        // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_translate=false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent(QWheelEvent *_event)
{

	// check the diff of the wheel position (0 means no change)
	if(_event->delta() > 0)
	{
		m_modelPos.m_z+=ZOOM;
	}
	else if(_event->delta() <0 )
	{
		m_modelPos.m_z-=ZOOM;
	}
	renderLater();
}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quit
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  // turn on wirframe rendering
  case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
  // turn off wire frame
  case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
  // show full screen
  case Qt::Key_F : showFullScreen(); break;
  // show windowed
  case Qt::Key_N : showNormal(); break;
  case Qt::Key_1 : m_buffer=0; break;
  case Qt::Key_2 : m_buffer=1; break;
  case Qt::Key_3 : m_buffer=2; break;
  case Qt::Key_4 : m_buffer=3; break;
  case Qt::Key_5 : m_buffer=4; break;
  case Qt::Key_I : m_freq+=1; break;
  case Qt::Key_O : m_freq-=1; break;
  case Qt::Key_Space : m_animate^=true; break;

  default : break;
  }
  // finally update the GLWindow and re-draw
  //if (isExposed())
    renderLater();
}


void NGLScene::drawScene(const std::string &_shader)
{
  // clear the screen and depth buffer
   // Rotation based on the mouse position for our global
   // transform
   ngl::Mat4 rotX;
   ngl::Mat4 rotY;
   // create the rotation matrices
   rotX.rotateX(m_spinXFace);
   rotY.rotateY(m_spinYFace);
   // multiply the rotations
   m_mouseGlobalTX=rotY*rotX;
   // add the translations
   m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
   m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
   m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

   // get the VBO instance and draw the built in teapot
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  ngl::ShaderLib *shader = ngl::ShaderLib::instance();
  shader->use(_shader);
  //glPolygonMode(GL_FRONT_AND_BACK,m_wireframe);
  m_transform.reset();
  {
    shader->setShaderParam3f("Colour",1,0,0);
    loadMatricesToShader();
    prim->draw("teapot");
  } // and before a pop

  m_transform.reset();
  {
    shader->setShaderParam3f("Colour",0,1,0);

    m_transform.setPosition(-3,0.0,0.0);
    loadMatricesToShader();
    prim->draw("sphere");
  } // and before a pop

  m_transform.reset();
  {
    shader->setShaderParam3f("Colour",1,0,1);

    m_transform.setPosition(3,0.0,0.0);
    loadMatricesToShader();
    prim->draw("cylinder");
  } // and before a pop

  m_transform.reset();
  {
    shader->setShaderParam3f("Colour",0,0,1);

    m_transform.setPosition(0.0,0.01,3.0);
    loadMatricesToShader();
    prim->draw("cube");
  } // and before a pop

  m_transform.reset();
  {
    shader->setShaderParam3f("Colour",1,1,0);

    m_transform.setPosition(-3.0,0.05,3.0);
    loadMatricesToShader();
    prim->draw("torus");
  } // and before a pop

  m_transform.reset();
  {

    m_transform.setPosition(3.0,0.5,3.0);
    loadMatricesToShader();
    prim->draw("icosahedron");
  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(0.0,0.0,-3.0);
    loadMatricesToShader();
    prim->draw("cone");
  } // and before a pop


  m_transform.reset();
  {
    m_transform.setPosition(-3.0,0.6,-3.0);
    loadMatricesToShader();
    prim->draw("tetrahedron");
  } // and before a pop


  m_transform.reset();
  {
    shader->setShaderParam3f("Colour",0,0,0);

    m_transform.setPosition(3.0,0.5,-3.0);
    loadMatricesToShader();
    prim->draw("octahedron");
  } // and before a pop


  m_transform.reset();
  {
    shader->setShaderParam3f("Colour",1,1,1);

    m_transform.setPosition(0.0,0.5,-6.0);
    loadMatricesToShader();
    prim->draw("football");
  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(-3.0,0.5,-6.0);
    m_transform.setRotation(0,180,0);
    loadMatricesToShader();
    prim->draw("disk");
  } // and before a pop


  m_transform.reset();
  {
    m_transform.setPosition(3.0,0.5,-6.0);
    loadMatricesToShader();
    prim->draw("dodecahedron");
  } // and before a pop

  m_transform.reset();
  {
    shader->setShaderParam3f("Colour",1,1,0);

    m_transform.setPosition(1.0,0.35,1.0);
    m_transform.setScale(1.5,1.5,1.5);
    loadMatricesToShader();
    prim->draw("troll");
  } // and before a pop

  m_transform.reset();
  {
    shader->setShaderParam3f("Colour",1,0,0);

    m_transform.setPosition(-1.0,-0.5,1.0);
    m_transform.setScale(0.1,0.1,0.1);
    loadMatricesToShader();
    prim->draw("dragon");
  } // and before a pop

  m_transform.reset();
  {
    shader->setShaderParam3f("Colour",1,0,1);

    m_transform.setPosition(-2.5,-0.5,1.0);
    m_transform.setScale(0.1,0.1,0.1);
    loadMatricesToShader();
    prim->draw("buddah");
  } // and before a pop

  m_transform.reset();
  {
    shader->setShaderParam3f("Colour",1,0,0);

    m_transform.setPosition(2.5,-0.5,1.0);
    m_transform.setScale(0.1,0.1,0.1);
    loadMatricesToShader();
    prim->draw("bunny");
  } // and before a pop

  m_transform.reset();
  {
    shader->setShaderParam3f("Colour",0,0,0.8);

    m_transform.setPosition(0.0,-0.5,0.0);
    loadMatricesToShader();
    prim->draw("plane");
  } // and before a pop


//  for(float z=-5.0; z<5.0; z+=0.5)
//  {
//    for(float x=-5.0; x<5.0; x+=0.5)
//    {
//      m_transform.reset();

//      m_transform.setPosition(x,0,z);
//      loadMatricesToShader();
//      prim->draw("lightSphere");

//    }
//  }


glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
}

void NGLScene::loadLightShader(const ngl::Vec3 &_pos, float _intensity)
{
//  ngl::Transformation t;
//  t.setScale(2.0,2.0,2.0);
//  t.setPosition(_pos);

  ngl::ShaderLib *shader= ngl::ShaderLib::instance();
  shader->use("LightPass");

  ngl::Mat4 MVP=m_stack.top()*m_mouseGlobalTX*m_cam->getVPMatrix();
  shader->setShaderParamFromMat4("MVP",MVP);
  shader->setShaderParam3f("cam",m_cam->getEye().m_x,m_cam->getEye().m_y,m_cam->getEye().m_z);

  shader->setShaderParam1f("lightI",_intensity);
  shader->setShaderParam3f("lightPos",_pos.m_x,_pos.m_y,_pos.m_z);
 // shader->setShaderParamFromMat4("V",m_cam->getViewMatrix());
  shader->setShaderParam2f("wh",m_framebuffer->width(),m_framebuffer->height());
}



void NGLScene::loadNullShader(const ngl::Vec3 &_pos,float _intensity)
{
//  ngl::Transformation t;
//  t.setScale(2.0,2.0,2.0);
//  t.setPosition(_pos);

  ngl::ShaderLib *shader= ngl::ShaderLib::instance();
  shader->use("null");
  ngl::Mat4 MVP=m_stack.top()*m_mouseGlobalTX*m_cam->getVPMatrix();
  shader->setShaderParamFromMat4("MVP",MVP);
}


void NGLScene::lightPass()
{
  glDrawBuffer(GL_COLOR_ATTACHMENT3);
 // m_framebuffer->setDrawBuffers();
  glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);



  glActiveTexture (GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_2D, m_framebuffer->getTextureID("point"));
  glActiveTexture (GL_TEXTURE1);
  glBindTexture (GL_TEXTURE_2D, m_framebuffer->getTextureID("normal"));
  glActiveTexture (GL_TEXTURE2);
  glBindTexture (GL_TEXTURE_2D, m_framebuffer->getTextureID("colour"));
  glActiveTexture (GL_TEXTURE3);
  glBindTexture (GL_TEXTURE_2D, m_framebuffer->getTextureID("shading"));
  drawLights(RENDER);

  glCullFace(GL_BACK);

  glDisable(GL_BLEND);

}

void NGLScene::stencilPass()
{
  glDrawBuffer(GL_NONE);
  glEnable(GL_DEPTH_TEST);

  glDisable(GL_CULL_FACE);

  glClear(GL_STENCIL_BUFFER_BIT);

  // We need the stencil test to be enabled but we want it
  // to succeed always. Only the depth test matters.
  glStencilFunc(GL_ALWAYS, 0, 0);

  glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
  glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

  drawLights(STENCIL);
}


void NGLScene::drawLights(LightMode _mode)
{
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  float zz=2.0;
  m_stack.pushMatrix();
  m_stack.scale(16,16,16);
  if(_mode==STENCIL)
    loadNullShader(ngl::Vec3(0,2,0),0.4);
  else
    loadLightShader(ngl::Vec3(0,2,0),0.2);
  prim->draw("cube");
  m_stack.popMatrix();



  for(float i=0; i<2*M_PI; i+=0.1)
  {
    m_stack.pushMatrix();

      float x=cos(i)*zz;
      float z=sin(i)*zz;
      float y=sin(i*m_freq)*zz;

      m_stack.rotate(m_rot,0,1,0);
      m_stack.translate(x,y,z);
      m_stack.pushMatrix();
      m_stack.scale(0.05,0.05,0.05);

        if(_mode==STENCIL)
          loadNullShader(ngl::Vec3(x,y,z),0.1);
        else
          loadLightShader(ngl::Vec3(x,y,z),0.8);
        prim->draw("lightSphere");
      m_stack.popMatrix();
    m_stack.popMatrix();
  }

}

void NGLScene::timerEvent(QTimerEvent *_event)
{
  if(m_animate)
  m_rot+=0.5f;
  renderLater();
}
