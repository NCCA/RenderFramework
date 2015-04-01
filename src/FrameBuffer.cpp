#include "FrameBuffer.h"
#include <iostream>
#include <vector>

FrameBuffer::FrameBuffer(int _w, int _h)
{
  m_width=_w;
  m_height=_h;
  m_hasDepthBuffer=false;
  m_hasStencilBuffer=false;
  m_bound=false;
  m_colourAttatchment=0;
  glGenFramebuffers(1, &m_id);
  glBindFramebuffer(GL_FRAMEBUFFER, m_id);

  glBindFramebuffer(GL_FRAMEBUFFER,0);
}

FrameBuffer::~FrameBuffer()
{
  glDeleteFramebuffers(1,&m_id);
  if(m_hasStencilBuffer)
    glDeleteRenderbuffers(1,&m_stencilBufferID);
  if(m_hasDepthBuffer)
    glDeleteRenderbuffers(1,&m_depthBufferID);
}

void FrameBuffer::bind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_id);
  m_bound=true;
}

void FrameBuffer::unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER,0);
  m_bound=false;
}

void FrameBuffer::attatchDepthBuffer()
{
  if(m_bound == false)
  {
    std::cerr<<"Warning attatching Depth buffer to unbound Framebuffer\n";
  }

  glGenRenderbuffers(1, &m_depthBufferID);
  glBindRenderbuffer(GL_RENDERBUFFER, m_depthBufferID);

  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
  m_hasDepthBuffer=true;
  // attatch
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBufferID);
  //glBindRenderbuffer(GL_RENDERBUFFER,0);

}

void FrameBuffer::attatchStencilBuffer()
{
  if(m_bound == false)
  {
    std::cerr<<"Warning attatching Stencil buffer to unbound Framebuffer\n";
  }
  glGenRenderbuffers(1, &m_stencilBufferID);
  glBindRenderbuffer(GL_RENDERBUFFER, m_stencilBufferID);

  glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX16, m_width, m_height);
  m_hasStencilBuffer=true;
  // attatch
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_stencilBufferID);

  //glBindRenderbuffer(GL_RENDERBUFFER,0);

}


 void FrameBuffer::attatchDepthAndStencilBuffer()
 {
   if(m_bound == false)
   {
     std::cerr<<"Warning attatching Stencil and Depth buffer to unbound Framebuffer\n";
   }
   glGenRenderbuffers(1, &m_stencilBufferID);
   glBindRenderbuffer(GL_RENDERBUFFER, m_stencilBufferID);

   glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
   m_hasStencilBuffer=true;
   // attatch
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_stencilBufferID);

   //glBindRenderbuffer(GL_RENDERBUFFER,0);
 }


void FrameBuffer::attatchRenderBuffer(GLenum _type)
{
  if(m_bound == false)
  {
    std::cerr<<"Warning attatching Render buffer to unbound Framebuffer\n";
  }

}


void FrameBuffer::attatchTexture(const std::string &_name,
                                GLenum _minFilter,
                                GLenum _magFilter,
                                GLint _internalFormat,
                                GLenum _format,
                                GLenum _type
                                )
{
  TextureAttatchment t;
  t.name=_name;
  t.number=GL_COLOR_ATTACHMENT0+m_colourAttatchment;
  t.active=true;
  glGenTextures(1, &t.id);
  // bind it to make it active
  glBindTexture(GL_TEXTURE_2D, t.id);
  // set params
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _magFilter);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _minFilter);
  glTexImage2D(GL_TEXTURE_2D, 0, _internalFormat, m_width, m_height, 0, _format, _type, 0);
  // attatch texture
  std::cout<<t.number<<" "<<t.id<<"\n";
  glFramebufferTexture2D(GL_FRAMEBUFFER, t.number, GL_TEXTURE_2D, t.id, 0);
  // and store
  m_attatchments.push_back(t);

 ++m_colourAttatchment;

}

void FrameBuffer::setDrawBuffers()
{
  if(m_bound == false)
  {
    std::cerr<<"Warning setting draw buffer to unbound Framebuffer\n";
  }

  std::vector<GLenum> buffers;
  int active=0;
  for(unsigned int i=0; i<m_attatchments.size(); ++i)
  {
    if(m_attatchments[i].active==true)
    {
      buffers.push_back(m_attatchments[i].number);
      ++active;
    }
  }
  glDrawBuffers(active,&buffers[0]);
}

GLuint FrameBuffer::getTextureID(const std::string &_name) const
{
  GLuint id=0;
  for(unsigned int i=0; i<m_attatchments.size(); ++i)
  {
    if(m_attatchments[i].name == _name)
    {
      id=m_attatchments[i].id;
      break;
    }
  }
  return id;
}

void FrameBuffer::debug() const
{
	int res;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &res);
	std::cout<<"Max Color Attachments: "<<res<<"\n";


	GLint buffer;
	int i = 0;
	do
	{
		glGetIntegerv(GL_DRAW_BUFFER0+i, &buffer);

		if (buffer != GL_NONE)
		{
			std::cout<<"Shader Output Location "<<i<<"- color attachment "<<buffer- GL_COLOR_ATTACHMENT0<<"\n";
			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, buffer, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &res);
			std::cout<<"\tAttachment Type: "<< (res==GL_TEXTURE? "Texture":"Render Buffer") <<"\n";
			glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, buffer, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &res);
			std::cout<<"\tAttachment object name: "<<res<<"\n";
		}
		++i;
	} while (buffer != GL_NONE);

}
