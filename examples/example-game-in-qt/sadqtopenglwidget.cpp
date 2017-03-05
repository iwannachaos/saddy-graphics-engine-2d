#include "sadqtopenglwidget.h"

#include <mousecursor.h>
#include <scene.h>
#include <fpsinterpolation.h>

#include <input/events.h>

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>

sad::qt::OpenGLWidget::OpenGLWidget(QWidget* parent, Qt::WindowFlags f) : QOpenGLWidget(parent, f), m_first(true), m_reshaped(false), m_window(NULL)
{
	QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
	fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
	fmt.setRenderableType(QSurfaceFormat::OpenGL);
	fmt.setSwapInterval(1);
	fmt.setRedBufferSize(8);
	fmt.setGreenBufferSize(8);
	fmt.setBlueBufferSize(8);
	fmt.setAlphaBufferSize(8);
	fmt.setStencilBufferSize(8);
	fmt.setDepthBufferSize(24);
	fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);

	this->setFormat(fmt);
	this->setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
    
	connect(QApplication::instance(), SIGNAL(lastWindowClosed()), this, SLOT(applicationQuit()));
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));
	m_timer.setInterval(0);
	m_timer.start();

	m_renderer = new sad::qt::Renderer();
	m_renderer->setWidget(this);

	this->setMouseTracking(true);
}

sad::qt::OpenGLWidget::~OpenGLWidget()
{
	if (m_renderer->initialized())
	{
		m_renderer->deinitRendererAfterLoop();
	}
	if (m_renderer)
	{
		delete m_renderer;
	}
}


void sad::qt::OpenGLWidget::setRenderer(sad::qt::Renderer* renderer)
{
	if (renderer)
	{
		if (renderer != m_renderer)
		{
			m_first = true;
			m_reshaped = false;
			if (m_renderer)
			{
				if (m_renderer->initialized())
				{
					m_renderer->submitEvent(new sad::input::QuitEvent(), true);
				}
			}
			delete m_renderer;
			m_renderer = renderer;
			m_renderer->setWidget(this);
		}
	}
}

sad::qt::Renderer* sad::qt::OpenGLWidget::renderer() const
{
	return m_renderer;
}

void sad::qt::OpenGLWidget::resizeGL(int width, int height)
{
	if (width == 0)
	{
		width = 1;
	}
	if (m_first)
	{
		if (m_renderer->initialized())
		{
			m_renderer->initRendererBeforeLoop();
			if (this->isActiveWindow())
			{
				m_renderer->submitEvent(new sad::input::ActivateEvent());
			}
			m_first = false;
		}
		else
		{
			m_first = true;
		}
	}
	if (m_renderer->initialized())
	{
		m_renderer->reshape(width, height);
		sad::input::ResizeEvent* ev = new sad::input::ResizeEvent();
		ev->OldSize = m_old_size;
		ev->NewSize = sad::Size2I(width, height);
		m_renderer->submitEvent(ev);
		m_reshaped = true;
	}
	this->update();
	m_old_size = sad::Size2I(width, height);
}

void sad::qt::OpenGLWidget::paintGL()
{
	if (this->window() != m_window)
	{
		if (m_window)
		{
			m_window->removeEventFilter(this);
		}
		m_window = this->window();
		m_window->installEventFilter(this);
	}
	if (m_renderer)
	{
		if (m_reshaped == false && m_renderer->initialized())
		{
			this->resizeGL(this->width(), this->height());
		}
	}
	
	if (m_renderer)
	{
		if (m_renderer->initialized())
		{			
			m_renderer->runOnce();
		}
	}
}

void sad::qt::OpenGLWidget::enterEvent(QEvent* ev)
{
	if (this->m_renderer)
	{
		if (m_renderer->initialized())
		{
			QPoint p = QCursor::pos();
			p = this->mapFromGlobal(p);
			sad::input::MouseEnterEvent* sev = new sad::input::MouseEnterEvent();
			sev->Point3D = this->toViewport(p);
			m_renderer->submitEvent(sev);
		}
	}

	this->QOpenGLWidget::enterEvent(ev);
}

void sad::qt::OpenGLWidget::mouseMoveEvent(QMouseEvent* ev)
{
	if (this->m_renderer)
	{
		if (m_renderer->initialized())
		{
			sad::input::MouseMoveEvent* sev = new sad::input::MouseMoveEvent();
			sev->Point3D = this->toViewport(ev->pos());
			m_renderer->submitEvent(sev);
		}
	}
	this->QOpenGLWidget::mouseMoveEvent(ev);
}

void sad::qt::OpenGLWidget::leaveEvent(QEvent* ev)
{
	if (m_renderer)
	{
		if (m_renderer->initialized())
		{
			m_renderer->submitEvent(new sad::input::MouseLeaveEvent());
		}
	}
	this->QOpenGLWidget::leaveEvent(ev);
}


bool sad::qt::OpenGLWidget::event(QEvent* e)
{
	this->tryHandleActivateEvent(e);
	this->tryHandleMinimization(e);
	return this->QOpenGLWidget::event(e);
}

bool sad::qt::OpenGLWidget::eventFilter(QObject* obj, QEvent* ev)
{
	if (obj != this)
	{
		this->tryHandleActivateEvent(ev);
		this->tryHandleMinimization(ev);
	}
	return QObject::eventFilter(obj, ev);
}

// ======================= PUBLIC SLOT METHODS =======================

void sad::qt::OpenGLWidget::applicationQuit()
{
	if (m_renderer)
	{
		if (m_renderer->initialized())
		{
			m_renderer->submitEvent(new sad::input::QuitEvent(), true);
		}
	}
}

// ======================= PROTECTED METHODS =======================

void sad::qt::OpenGLWidget::tryHandleActivateEvent(QEvent* ev) const
{
	if (ev->type() == QEvent::ActivationChange)
	{
		if (m_renderer)
		{
			if (m_renderer->initialized())
			{
				if (this->isActiveWindow())
				{
					m_renderer->submitEvent(new sad::input::ActivateEvent());
				}
				else
				{
					m_renderer->submitEvent(new sad::input::DeactivateEvent());
				}
			}
		}
	}
}

void sad::qt::OpenGLWidget::tryHandleMinimization(QEvent* ev) const
{
	if (ev->type() == QEvent::WindowStateChange)
	{
		if (this->window()->isMinimized() && (this->m_renderer != NULL))
		{
			if (m_renderer->initialized())
			{
				this->m_renderer->fpsInterpolation()->reset();
			}
		}
	}
}

sad::Point3D sad::qt::OpenGLWidget::toViewport(const QPoint& p) const
{
	double y = this->height() - p.y();
	sad::Point3D mx = m_renderer->mapToViewport(sad::Point2D(p.x(), y));
	return mx;
}
