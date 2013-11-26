#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

const float UP_ACTION = 0.5f;
const float DOWN_ACTION = -0.5f;
const float STOP_ACTION = 0;

class Camera {
	glm::vec3 eye;
	glm::vec3 target;
	glm::vec3 up;
	glm::vec3 pointOfView;
	int xOrigin, yOrigin;
	float hAngle, vAngle;
	float vDeltaAngle;
    float hDeltaAngle;
    float deltaMove;
public:
	Camera();
	void setOrigins(int* x, int* y);
	void releaseMouseButton();
	void moveMouseUpdate(int x, int y);
	void setDeltaMove(float action);
	void update();
	glm::mat4x4 getViewMatrix();
};