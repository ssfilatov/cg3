#define _USE_MATH_DEFINES
#include <cmath>

#include "camera.h"

Camera::Camera(): 
	eye(-2, 10, 10),
	target(0, 0, 0),
	up(0, 1, 0),
	pointOfView(0, 0, -1),
	xOrigin(-1), 
	yOrigin(-1),
	hAngle(M_PI), 
	vAngle(0),
	vDeltaAngle(0),
    hDeltaAngle(0),
    deltaMove(0)
{}

void Camera::setOrigins(int* x, int* y) {
	xOrigin = *x;
	yOrigin = *y;
}

void Camera::releaseMouseButton() {
	hAngle += hDeltaAngle;
	vAngle += vDeltaAngle; 
	xOrigin = -1;
	yOrigin = -1;
}

void Camera::moveMouseUpdate(int x, int y) {
    if (xOrigin >= 0 && yOrigin >= 0) {
	
		hDeltaAngle = (x - xOrigin) * 0.001f;
		vDeltaAngle = (y - yOrigin) * 0.001f; 

		pointOfView.x = cos(vAngle + vDeltaAngle) * sin(hAngle + hDeltaAngle), 
		pointOfView.y = sin(vAngle + vDeltaAngle);
        pointOfView.z = cos(vAngle + vDeltaAngle) * cos(hAngle + hDeltaAngle);
	}
}

void Camera::setDeltaMove(float action_value) {
	deltaMove = action_value;
}

void Camera::update() {
	if (deltaMove) {
		eye += glm::vec3(deltaMove * pointOfView.x * 0.1f, deltaMove * pointOfView.y * 0.1f, deltaMove * pointOfView.z * 0.1f);
	}
	target = eye + pointOfView;
}

glm::mat4x4 Camera::getViewMatrix() {
	return glm::lookAt(eye, target, up);
}
