#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/Random.h>
#include <ngl/ShaderLib.h>
#include <ngl/Util.h>

const static int s_numPoints=1000;

NGLScene::NGLScene(QWindow *_parent) : OpenGLWindow(_parent)
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  setTitle("Blank NGL");
  m_rot=0.0;
}


NGLScene::~NGLScene()
{
  ngl::NGLInit *Init = ngl::NGLInit::instance();
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
  Init->NGLQuit();
}

void NGLScene::resizeEvent(QResizeEvent *_event )
{
  if(isExposed())
  {
  int w=_event->size().width();
  int h=_event->size().height();
  // set the viewport for openGL
  glViewport(0,0,w,h);
  renderLater();
  }
}


void NGLScene::initialize()
{
  // we need to initialise the NGL lib which will load all of the OpenGL functions, this must
  // be done once we have a valid GL context but before we call any GL commands. If we dont do
  // this everything will crash
  ngl::NGLInit::instance();
  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  // as re-size is not explicitly called we need to do this.
  glViewport(0,0,width(),height());
  // lets create a static camera view and projection
  ngl::Mat4 view=ngl::lookAt(ngl::Vec3(5,5,5),ngl::Vec3(0,0,0),ngl::Vec3(0,1,0));
  ngl::Mat4 perspective=ngl::perspective(45.0f,float(width()/height()),0.1,100);
  // store to vp for later use
  m_vp=view*perspective;
  // now load the default nglColour shader and set the colour for it.
  ngl::ShaderLib *shader = ngl::ShaderLib::instance();
  // set this as the active shader
  shader->use("nglColourShader");
  // set the colour to red
  shader->setShaderParam4f("Colour",1.0f,1.0f,1.0f,1.0f);
  createPoints(s_numPoints);
  glPointSize(5);
  startTimer(1);
}

void NGLScene::createPoints(unsigned int _size)
{
  // we are going to create an array of random points using the
  // random generator in ngl
  ngl::Random *rng= ngl::Random::instance();
  // create an array of ngl::Vec3 and re-size
  std::vector<ngl::Vec3> points(_size);

  // now populate the array with random points in the range -5 -> 5
  for(unsigned int i=0; i<_size; ++i)
  {
    points[i]=rng->getRandomPoint(5.0f,5.0f,5.0f);
  }

  // first create the VAO
  m_vao = ngl::VertexArrayObject::createVOA(GL_POINTS);
  // to use this it must be bound
  m_vao->bind();
  // now copy the data
  m_vao->setData(points.size()*sizeof(ngl::Vec3),points[0].m_x);
  // now we need to tell OpenGL the size and layout of the data
  m_vao->setVertexAttributePointer(0,3,GL_FLOAT,0,0);
  // now tell OpenGL how maya elements we have
  m_vao->setNumIndices(points.size());
  // always best to unbind after use
  m_vao->unbind();
}


void NGLScene::updatePoints(unsigned int _size)
{
  std::cout<<"update\n";
  // we are going to create an array of random points using the
  // random generator in ngl
  ngl::Random *rng= ngl::Random::instance();
  // create an array of ngl::Vec3 and re-size
  std::vector<ngl::Vec3> points(_size);

  // now populate the array with random points in the range -5 -> 5
  for(unsigned int i=0; i<_size; ++i)
  {
    points[i]=rng->getRandomPoint(5.0f,5.0f,5.0f);
  }
  // to use this it must be bound
  m_vao->bind();
  // now copy the data
  m_vao->updateData(points.size()*sizeof(ngl::Vec3),points[0].m_x);

  // this can also be done via GL directly using
  //glBindBuffer(GL_ARRAY_BUFFER, m_vao->getVBOid(0));
  //glBufferData(GL_ARRAY_BUFFER, points.size()*sizeof(ngl::Vec3), &points[0].m_x, GL_STATIC_DRAW);

  // always best to unbind after use
  m_vao->unbind();

}

void NGLScene::render()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  ngl::Transformation transform;
  transform.setRotation(0.0,m_rot,0.0);
  ngl::Mat4 MVP=transform.getMatrix()*m_vp;
  shader->setRegisteredUniformFromMat4("MVP",MVP);
  m_vao->bind();
  m_vao->draw();
  m_vao->unbind();

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent (QMouseEvent * _event)
{

}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent ( QMouseEvent * _event)
{

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent ( QMouseEvent * _event )
{

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent(QWheelEvent *_event)
{

}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  case Qt::Key_Space : updatePoints(s_numPoints); break;
  default : break;
  }
  // finally update the GLWindow and re-draw
  if (isExposed())
    renderLater();
}


void NGLScene::timerEvent(QTimerEvent *_event)
{
  m_rot+=0.1;
  renderNow();
}
