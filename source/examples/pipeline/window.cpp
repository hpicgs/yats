#include "window.h"

#include <QApplication>
#include <QOpenGLContext>

window::window()
    : m_context(new QOpenGLContext)
{
    QSurfaceFormat format;
    format.setVersion(4, 0);
    format.setRenderableType(QSurfaceFormat::OpenGL);

    setSurfaceType(OpenGLSurface);
    create();

    m_context->setFormat(format);
    if (!m_context->create())
    {
        qDebug() << "Could not create OpenGL context.";
        QApplication::quit();
    }
}

window::~window()
{
}

void window::exec()
{
    m_context->makeCurrent(this);
    run();
    m_context->doneCurrent();
}