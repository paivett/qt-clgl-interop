#include <QtWidgets>
#include <GL/glx.h>
#include <math.h>
#include <chrono>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <fstream>

#include "glwidget.h"

#define CL_CHECK(status, msg)   {if (status != CL_SUCCESS) { cout << status << endl; throw runtime_error(msg); }}

using namespace std;


GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent), _t(0.0f) {

    setMouseTracking(false);

    _model_transformation.setToIdentity();

    _camera.look_at(QVector3D(1, 1, 1),
                    QVector3D(0, 0, 0),
                    QVector3D(0, 1, 0));

    for (int i=0; i < _points_per_side; ++i) {
        for (int j=0; j < _points_per_side; ++j) {
            auto x = ((float)i / _points_per_side) - 0.5f;
            auto z = ((float)j / _points_per_side) - 0.5f;
            cl_float4 p = {{x, 0.0f, z, 1.0f}};
            
            _initial_vertex_positions.push_back(p);
        }
    }
    
    connect(&_timer, SIGNAL(timeout()), this, SLOT(timerEvent()));
    _timer.setSingleShot(true);
    _timer.start();
}

GLWidget::~GLWidget() {

}

QSize GLWidget::minimumSizeHint() const {
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const {
    return QSize(400, 400);
}

void GLWidget::initializeGL() {
    _gl_functions = context()->versionFunctions<QOpenGLFunctions_4_5_Core>();

    // CL context must be initialized after the OpenGL context is ready
    // Here at initializeGL we already have a gl context ready :)
    _init_cl_context();

    _init_gl_state();

    _init_cl_state();
}

void GLWidget::_init_cl_context() {
    vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    
    cout << "Avaiable OpenCL platforms:" << endl;
    for (auto &p : platforms) {
        string plat_name = p.getInfo<CL_PLATFORM_NAME>();
        string plat_vendor = p.getInfo<CL_PLATFORM_VENDOR>();
        string plat_ver = p.getInfo<CL_PLATFORM_VERSION>();
        cout << " * " << plat_name << ", " << plat_ver << " (" << plat_vendor << ")" << endl;
    }
    
    cout << "Using first GPU on first platform found..." << endl;
    cl::Platform platform = platforms[0];

    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)(platform)(),
        CL_GL_CONTEXT_KHR, (cl_context_properties) glXGetCurrentContext(),
        CL_GLX_DISPLAY_KHR, (cl_context_properties) glXGetCurrentDisplay(),
        0
    };

    cl_int status;
    _cl_context = cl::Context(
        CL_DEVICE_TYPE_GPU,
        properties,
        nullptr,
        nullptr,
        &status
    );

    CL_CHECK(status, "OpenCL context initialization failed")

    _cl_queue = cl::CommandQueue(
        _cl_context,
        CL_QUEUE_PROFILING_ENABLE,
        &status
    );

    CL_CHECK(status, "OpenCL command-queue initialization failed")
}

void GLWidget::_init_gl_state() {
    // Generate VBO to store vertices and normals
    _gl_functions->glGenBuffers(1, &_vbo_vertices);
    _gl_functions->glBindBuffer(GL_ARRAY_BUFFER, _vbo_vertices);
    _gl_functions->glBufferData(GL_ARRAY_BUFFER,
                                _particle_count * sizeof(cl_float4),
                                nullptr, // Data will be uploaded using OpenCL
                                GL_DYNAMIC_DRAW);
    _gl_functions->glBindBuffer(GL_ARRAY_BUFFER, 0);

    _gl_functions->glEnable(GL_DEPTH_TEST);

    _init_shader();
}

void GLWidget::_init_shader() {
    _shader.addShaderFromSourceFile(QOpenGLShader::Vertex, "shaders/shader.vert");
    _shader.addShaderFromSourceFile(QOpenGLShader::Fragment, "shaders/shader.frag");

    if (!_shader.link()) {
        throw runtime_error("Could not link shader program");
    }

    _shader.bind();

    // Create a VAO for this stage
    _gl_functions->glGenVertexArrays(1, &_vao);
    _gl_functions->glBindVertexArray(_vao);

    // Bind the VBO to the shader
    auto vertex_coord_loc = _shader.attributeLocation("vertex_coord");
    _gl_functions->glBindBuffer(GL_ARRAY_BUFFER, _vbo_vertices);
    _gl_functions->glVertexAttribPointer(vertex_coord_loc,
                                         4,
                                         GL_FLOAT,
                                         GL_FALSE,
                                         0,
                                         nullptr);
    _shader.enableAttributeArray(vertex_coord_loc);
}

void GLWidget::_init_cl_state() {
    cl_int status;
    _cl_vertices = cl::BufferGL(_cl_context,
                                CL_MEM_READ_WRITE,
                                _vbo_vertices,
                                &status);
    _clgl_shared_buffers.push_back(_cl_vertices);
    CL_CHECK(status, "GL-CL vertices buffer could not be initialized")

    // Now let's write some data to the GL buffer
    _aquire_shared_buffers();

    status = _cl_queue.enqueueWriteBuffer(_cl_vertices,
                                          CL_TRUE,
                                          0,
                                          _initial_vertex_positions.size() * sizeof(cl_float4),
                                          _initial_vertex_positions.data(),
                                          nullptr,
                                          nullptr);
    CL_CHECK(status, "Could not upload vertex data to GL buffer")

    _release_shared_buffers();
    
    _init_cl_kernel();
}

void GLWidget::_aquire_shared_buffers() {
    cl_int status = _cl_queue.enqueueAcquireGLObjects(&_clgl_shared_buffers, 
                                                      nullptr, 
                                                      nullptr);
    CL_CHECK(status, "Could not aquire shared GL buffers")
}

void GLWidget::_release_shared_buffers() {
    cl_int status = _cl_queue.enqueueReleaseGLObjects(&_clgl_shared_buffers, 
                                                      nullptr, 
                                                      nullptr);
    CL_CHECK(status, "Could not release shared GL buffers")
}

void GLWidget::_init_cl_kernel() {
    string path = "kernel/surface.cl";
    ifstream source_file(path);
    if (!source_file.good()) {
        throw runtime_error("Could not open OpenCL program source!");
    }

    string program_src((istreambuf_iterator<char>(source_file)),
                        istreambuf_iterator<char>());
    
    auto program = cl::Program(_cl_context,
                               program_src);

    program.build();

    cl_int status;
    _cl_kernel = cl::Kernel(program, "compute_surface", &status);
    CL_CHECK(status, "Could not initialize OpenCL kernel")
}



void GLWidget::_compute_surface(float t) {
    // Now let's write some data to the GL buffer
    _aquire_shared_buffers();

    _cl_kernel.setArg(0, _cl_vertices);
    _cl_kernel.setArg(1, t);
    _cl_kernel.setArg(2, _points_per_side);

    cl_int status = _cl_queue.enqueueNDRangeKernel(_cl_kernel,
                                                  cl::NDRange(0, 0),
                                                  cl::NDRange(_points_per_side, _points_per_side),
                                                  cl::NDRange(8, 8));

    CL_CHECK(status, "Failed at running OpenCL kernel")

    _release_shared_buffers();
}

void GLWidget::paintGL() {
    _compute_surface(_t);

    _gl_functions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _gl_functions->glBindVertexArray(_vao);
    _shader.bind();
    
    auto v_matrix = _camera.transformation();
    _shader.setUniformValue("m_matrix", _model_transformation);
    _shader.setUniformValue("v_matrix", v_matrix);
    _shader.setUniformValue("p_matrix", _camera.projection());

    _gl_functions->glDrawArrays(GL_POINTS, 0, _particle_count);
}

void GLWidget::resizeGL(int w, int h) {
    Perspective p;
    p.aspect = (float)w / (float)(h ? h : 1);
    p.nearPlane = 0.1;
    p.farPlane = 30.0;
    p.fov = 45.0;
    p.width = w;
    p.height = h;
    _camera.set_perspective(p);
}

void GLWidget::timerEvent() { 
    auto t0 = chrono::high_resolution_clock::now();
    update();
    auto tf = chrono::high_resolution_clock::now();
    auto d = chrono::duration_cast<chrono::milliseconds>(tf-t0);
    auto ms = d.count();

    _camera.rotate(0.01, QVector3D(0, 1, 0));

    _t += 0.0002;
    if (_t > 2.0f) {
        _t -= 1.0f;
    }

    // 60 fps
    _timer.start(max(0.1 - ms, 0.0));
}

