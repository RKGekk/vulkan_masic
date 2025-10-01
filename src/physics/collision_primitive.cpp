#include "collision_primitive.h"

void CollisionPrimitive::calculateInternals() {
	transform = offset * body->getTransform();
}