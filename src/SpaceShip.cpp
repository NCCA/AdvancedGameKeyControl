#include "SpaceShip.h"
#include "GameControls.h"
#include <ngl/ShaderLib.h>

SpaceShip::SpaceShip(const ngl::Vec3 &_pos, const std::string &_fname )
 {
   m_pos=_pos;
   m_mesh.reset(new ngl::Obj(_fname));
   m_mesh->createVAO();
	 m_rotation=0;
 }

void SpaceShip::draw(const std::string &_shader, const ngl::Mat4 &_viewProject )
{
  ngl::ShaderLib::use(_shader);
  m_transform.setPosition(m_pos);
	m_transform.setRotation(0,m_rotation,0);
  ngl::Mat4 MVP=_viewProject*m_transform.getMatrix();
  ngl::ShaderLib::setUniform("MVP",MVP);
  m_mesh->draw();
}


void SpaceShip::move( uint8_t _keysPressed )
{
  // increment
    m_pos.m_x += g_motionTable[_keysPressed].offsetX;
    m_pos.m_y += g_motionTable[_keysPressed].offsetY;
    const static float s_rotationUpdate=4.0;
    m_rotation += s_rotationUpdate*g_motionTable[_keysPressed].rotation;

    // clamp
    m_pos.m_x = std::max(-s_xExtents, m_pos.m_x);
    m_pos.m_y = std::max(-s_yExtents, m_pos.m_y);
    m_pos.m_x = std::min(s_xExtents, m_pos.m_x);
    m_pos.m_y = std::min(s_yExtents, m_pos.m_y);
}

void SpaceShip::rotate(float _rot	 )
{
	m_rotation+=_rot;
}









