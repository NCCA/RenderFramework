#ifndef FRAMEBUFFER_H__
#define FRAMEBUFFER_H__
#include <ngl/Types.h>
#include <vector>
#include <string>


typedef struct
{
  GLenum number;
  GLuint id;
  bool active;
  std::string name;
}TextureAttatchment;

class FrameBuffer
{
public :
  FrameBuffer(int _w, int _h);
  ~FrameBuffer();
  void bind();
  void unbind() ;
  GLuint id() const {return m_id;}
  void attatchRenderDepthBuffer();
  void attatchStencilBuffer();
  void attatchRenderBuffer(GLenum _type);
  bool hasStencilBuffer() const {return m_hasStencilBuffer;}
  bool hasDepthBuffer() const { return m_hasDepthBuffer; }
  void attatchTexture(const std::string &_name,
                      GLenum _minFilter=GL_NEAREST,
                      GLenum _magFilter=GL_NEAREST,
                      GLint _internalFormat=GL_RGB16F,
                      GLenum _format=GL_RGB,
                      GLenum _type=GL_UNSIGNED_BYTE
                      );
  void setDrawBuffers();
  GLuint getTextureID(const std::string &_name) const;
  void debug() const;
  int width()const {return m_width;}
  int height()const {return m_height;}
private :
  /// @brief disable copying
  FrameBuffer(const FrameBuffer &){}

  /// @brief the Framebuffer ID
  GLuint m_id;
  /// @brief the width of the framebuffer (note if using retina display)
  /// pre-multiply by devicePixelRation
  int m_width;
  /// @brief the height of the framebuffer (note if using retina display)
  /// pre-multiply by devicePixelRation
  int m_height;
  /// @brief flag to indicate if depth buffer is attatched
  bool m_hasDepthBuffer;
  /// @brief flag to indicate if stencil buffer is attatched
  bool m_hasStencilBuffer;
  /// @brief check to see if FB is bound
  bool m_bound;
  /// @brief id of the depth buffer
  GLuint m_depthBufferID;
  /// @brief id of the stencil buffer
  GLuint m_stencilBufferID;

  std::vector<TextureAttatchment> m_attatchments;

  GLuint m_colourAttatchment;



};

#endif
