#include "camera.h"
#include <algorithm>

Camera::Camera() {
    _projection.setToIdentity();
    _transformation.setToIdentity();
}

QMatrix4x4 Camera::projection() const {
    return _projection;
}

QMatrix4x4 Camera::transformation() const {
    return _transformation;
}

void Camera::set_perspective(const Perspective& p) {
    _perspective = p;
    _projection.setToIdentity();
    _projection.perspective(p.fov, p.aspect, p.nearPlane, p.farPlane);
}

Perspective Camera::perspective() const {
    return _perspective;
}

void Camera::look_at(const QVector3D& eye, const QVector3D& center, const QVector3D& up) {
    _transformation.setToIdentity();
    _transformation.lookAt(eye, center, up);
}

void Camera::rotate(float angle, const QVector3D& axis) {
    _transformation.rotate(angle, axis);    
}
