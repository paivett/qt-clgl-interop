#ifndef _GLWIDGET_H_
#define _GLWIDGET_H_

#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QMatrix4x4>
#include <QTimer>
#include "utils/camera.h"

#include <CL/cl.hpp>

class GLWidget : public QOpenGLWidget {
    Q_OBJECT

    public:
        GLWidget(QWidget *parent = 0);
        ~GLWidget();

        QSize minimumSizeHint() const;
        QSize sizeHint() const;

    protected:
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);

    private slots:
        void timerEvent();

    private:
        const int _points_per_side = 64;
        const int _particle_count = _points_per_side * _points_per_side;
        std::vector<cl_float4> _initial_vertex_positions;

        QOpenGLFunctions_4_5_Core* _gl_functions;
        QOpenGLShaderProgram _shader;

        Camera _camera;
        QMatrix4x4 _model_transformation; 

        cl::Context _cl_context;
        cl::CommandQueue _cl_queue;
        cl::Kernel _cl_kernel;

        GLuint _vao;
        GLuint _vbo_vertices;
        cl::BufferGL _cl_vertices;
        std::vector<cl::Memory> _clgl_shared_buffers;

        void _init_cl_context();
        void _init_gl_state();
        void _init_cl_state();
        void _init_cl_kernel();
        void _init_shader();
        void _compute_surface(float t);
        void _aquire_shared_buffers();
        void _release_shared_buffers();

        //Timer is used to keep simulation going, and cap fps
        QTimer _timer;

        float _t;
};

#endif
