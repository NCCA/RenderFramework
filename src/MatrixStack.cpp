#include "MatrixStack.h"
#include <iostream>
#include <cstdlib>
#include <ngl/NGLStream.h>


void MatrixStack::pushMatrix()
{
  if(++m_top< m_depth)
  {
    // copy the matrix to the top of stack
    m_stack[m_top]=m_stack[m_top-1];
  }
  else
  {
    std::cerr<<"Matrix stack overflow\n";
    exit(EXIT_FAILURE);
  }
}
void MatrixStack::popMatrix()
{
  m_stack[m_top].identity();
  if(--m_top<0 )
  {
    std::cerr<<"Matrix stack underflow \n";
    exit(EXIT_FAILURE);
  }
}


void MatrixStack::rotate(float _x, float _y, float _z)
{
  ngl::Mat4 x,y,z;
  x.rotateX(_x);
  y.rotateY(_y);
  z.rotateZ(_z);
  ngl::Mat4 final=z*y*x;
  m_stack[m_top]*=final;
}

void MatrixStack::rotate(float _angle,float _x, float _y, float _z)
{
  ngl::Mat4 r;
  r.euler(_angle,_x,_y,_z);
  m_stack[m_top]*=r;
}

void MatrixStack::translate(float _x, float _y, float _z)
{
  ngl::Mat4 t;
  t.translate(_x,_y,_z);
  m_stack[m_top]*=t;
}

void MatrixStack::scale(float _x, float _y, float _z)
{
  ngl::Mat4 s;
  s.scale(_x,_y,_z);
  m_stack[m_top]*=s;
}

