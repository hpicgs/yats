#pragma once
#include <QWindow>
#include <QDebug>
#include <QScopedPointer>

class QOpenGLContext;

class window : public QWindow
{
public:
    window();
    ~window();

    void exec();

protected:
    void run();

    QScopedPointer<QOpenGLContext> m_context;
};