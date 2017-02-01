#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <QMatrix4x4>
#include <QVector3D>

#include "perspective.h"

class Camera {
    public:
        Camera();

        // Sets the new perspective of the camera
        void set_perspective(const Perspective& p);

        // Returns the current perspective of the camera
        Perspective perspective() const;

        void look_at(const QVector3D& eye, const QVector3D& center, const QVector3D& up);

        void rotate(float angle, const QVector3D& axis);

        // Returns projection matrix of the camera
        QMatrix4x4 projection() const;

        // Returns the transformation matrix of the camera
        QMatrix4x4 transformation() const;

    private:
        QMatrix4x4 _projection;
        QMatrix4x4 _transformation;

        Perspective _perspective;
};

#endif // _CAMERA_H_
