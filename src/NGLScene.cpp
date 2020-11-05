#include "NGLScene.h"
#include <ngl/Transformation.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include "GameControls.h"
#include <QMouseEvent>
#include <QGuiApplication>
#include <QApplication>
#include <QFileDialog>

NGLScene::NGLScene()
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  setTitle("Game style key control in Qt");
  m_recording=false;
  m_playbackActive=false;
  m_currentPlaybackFrame=0;
  m_keysPressed=0;
}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}


void NGLScene::resizeGL(int _w , int _h)
{
  m_project=ngl::perspective(45.0f,static_cast<float>(_w)/_h,0.05f,350.0f);
  m_win.width=static_cast<int>(_w*devicePixelRatio());
  m_win.height=static_cast<int>(_h*devicePixelRatio());
}



void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::initialize();

  glClearColor(0.0f,0.0f,0.0f,1.0f);			   // black Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  ngl::ShaderLib::use(ngl::nglColourShader);
  ngl::ShaderLib::setUniform("Colour",1.0f,1.0f,0.0f,1.0f);


  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0.0f,0.0f,80.0f);
  ngl::Vec3 to(0.0f,0.0f,0.0f);
  ngl::Vec3 up(0.0f,1.0f,0.0f);
  // now load to our new camera
  m_view=ngl::lookAt(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project=ngl::perspective(45.0f,720.0f/576.0f,0.05f,350.0f);
  // create our spaceship
  m_ship=std::make_unique<SpaceShip>(ngl::Vec3(0.0f,0.0f,0.0f),"models/SpaceShip.obj");
  m_text=std::make_unique<ngl::Text>("fonts/Arial.ttf",12);
  m_text->setScreenSize(width(),height());
  m_text->setColour(1.0f,1.0f,1.0f);
  // now we start a timer and assign it to the m_updateShipTimer variable
  // this will be triggered at an interval of every 5ms
  // to stop this timer we call killTimer(m_animationTimer) we can have
  // multiple timers but each must have it's own ID
  m_updateShipTimer=startTimer(15);
  m_redrawTimer=startTimer(30);

}



void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_win.width,m_win.height);
  // now load these values to the shader
  m_ship->draw(ngl::nglColourShader,m_project*m_view);

	if (m_recording==true)
	{
	 m_text->setColour(1.0f,0.0f,0.0f);
	 m_text->renderText(10,18,"Recording");
	}
	if (m_playbackActive==true)
	{
	m_text->setColour(1.0f,1.0f,0.0f);
	m_text->renderText(10,18,fmt::format("Playback doing frame {}",m_currentPlaybackFrame));
	}
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent( QMouseEvent* _event )
{
  // note the method buttons() is the button state when event was called
  // that is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if ( m_win.rotate && _event->buttons() == Qt::LeftButton )
  {
    int diffx = _event->x() - m_win.origX;
    int diffy = _event->y() - m_win.origY;
    m_win.spinXFace += static_cast<int>( 0.5f * diffy );
    m_win.spinYFace += static_cast<int>( 0.5f * diffx );
    m_win.origX = _event->x();
    m_win.origY = _event->y();
    update();
  }
  // right mouse translate code
  else if ( m_win.translate && _event->buttons() == Qt::RightButton )
  {
    int diffX      = static_cast<int>( _event->x() - m_win.origXPos );
    int diffY      = static_cast<int>( _event->y() - m_win.origYPos );
    m_win.origXPos = _event->x();
    m_win.origYPos = _event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();
  }
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent( QMouseEvent* _event )
{
  // that method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if ( _event->button() == Qt::LeftButton )
  {
    m_win.origX  = _event->x();
    m_win.origY  = _event->y();
    m_win.rotate = true;
  }
  // right mouse translate mode
  else if ( _event->button() == Qt::RightButton )
  {
    m_win.origXPos  = _event->x();
    m_win.origYPos  = _event->y();
    m_win.translate = true;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent( QMouseEvent* _event )
{
  // that event is called when the mouse button is released
  // we then set Rotate to false
  if ( _event->button() == Qt::LeftButton )
  {
    m_win.rotate = false;
  }
  // right mouse translate mode
  if ( _event->button() == Qt::RightButton )
  {
    m_win.translate = false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent( QWheelEvent* _event )
{

  // check the diff of the wheel position (0 means no change)
  if ( _event->delta() > 0 )
  {
    m_modelPos.m_z += ZOOM;
  }
  else if ( _event->delta() < 0 )
  {
    m_modelPos.m_z -= ZOOM;
  }
  update();
}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::timerEvent( QTimerEvent *_event)
{
	// the usual process is to check the event timerID and compare it to
	// any timers we have started with startTimer
	if (_event->timerId() == m_updateShipTimer)
	{

		if(m_playbackActive == true)
		{
			if(m_currentPlaybackFrame <= 0)
			{
				m_ship->setPos(m_keyRecorder.getStartPosition());
			}
			if(m_currentPlaybackFrame < m_keyRecorder.size())
			{
				m_keysPressed=m_keyRecorder[m_currentPlaybackFrame];
				++m_currentPlaybackFrame;
			}
			else
			{
				m_playbackActive=false;
				m_currentPlaybackFrame=0;
			}

		}
		else if (m_recording== true)
		{
			m_keyRecorder.addFrame(m_keysPressed);
		}
	 moveShip();
	}
	if (_event->timerId() == m_redrawTimer)
	{
		update();
	}
}

void NGLScene::moveShip()
{
	m_ship->move(m_keysPressed);

}
void NGLScene::keyPressEvent(QKeyEvent *_event	)
{
	switch(_event->key())
	{
		case Qt::Key_Up: m_keysPressed |= kUp; break;
		case Qt::Key_Down: m_keysPressed |= kDown; break;
		case Qt::Key_Left: m_keysPressed |= kLeft; break;
		case Qt::Key_Right: m_keysPressed |= kRight; break;
		case Qt::Key_R: m_keysPressed |= kRotate; break;
		case Qt::Key_Space :
		{

			m_keyRecorder.setStartPosition(m_ship->getPos());
			m_recording^=true; break;
		}
		case Qt::Key_P : m_playbackActive^=true; break;

		case Qt::Key_S :
		{
		QString filename = QFileDialog::getSaveFileName(
            nullptr,
						tr("Save Keypresses"),
						QDir::currentPath(),
						tr("*.kp") );
				if( !filename.isNull() )
				{
					m_keyRecorder.save( filename.toStdString() );
				}
		break;
	}
	case Qt::Key_L :
	{
	QString filename = QFileDialog::getOpenFileName(
          nullptr,
					tr("load Keypresses"),
					QDir::currentPath(),
					tr("*.kp") );
			if( !filename.isNull() )
			{
				m_keyRecorder.load( filename.toStdString() );
			}
	}
	break;
	case Qt::Key_Escape: QApplication::exit(EXIT_SUCCESS); break;

	}
}


//----------------------------------------------------------------------------------------------------------------------
// in this case this method is called from the MainWindow class and passed
// the key event from that, if we didn't need to process the events in main
// window we could just overload this as the keyPressEvent and only process here.
//----------------------------------------------------------------------------------------------------------------------
void NGLScene::keyReleaseEvent( QKeyEvent *_event	)
{
	switch(_event->key())
	{
		case Qt::Key_Up: m_keysPressed &= ~kUp; break;
		case Qt::Key_Down: m_keysPressed &= ~kDown; break;
		case Qt::Key_Left: m_keysPressed &= ~kLeft; break;
		case Qt::Key_Right: m_keysPressed &= ~kRight; break;
		case Qt::Key_R: m_keysPressed &= ~kRotate; break;
	}
}

