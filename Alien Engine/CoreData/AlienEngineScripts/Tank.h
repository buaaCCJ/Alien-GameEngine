#pragma once

#include "..\..\Alien Engine\Alien.h"
#include "Macros/AlienScripts.h"

class ALIEN_ENGINE_API Tank : public Alien {

public:

	Tank();
	virtual ~Tank();

	void Start();
	void Update();

	void Shoot();
	void Movement();
	void Rotation();

	void OnDrawGizmos();

public:

	Prefab bullet;

	ComponentTransform* wheels_transform = nullptr;
	ComponentTransform* turret_transform = nullptr;

	float max_velocity_forward = 8.0f;
	float max_velocity_backward = -8.0f;

	float velocity = 0.0f;
	float friction_force = 0.05f;
	float acceleration = 4.0f;
	float recoil = 10.0f;
	float turning_velocity = 30.0f;

private:

	float angle = 0.0f;
	LineSegment ray;
	float3 to_look = { 0,0,0 };
};

ALIEN_FACTORY Tank* CreateTank() {
	Tank* tank = new Tank();
	SHOW_IN_INSPECTOR_AS_DRAGABLE_FLOAT(tank->max_velocity_forward);
	SHOW_IN_INSPECTOR_AS_DRAGABLE_FLOAT(tank->max_velocity_backward);
	SHOW_IN_INSPECTOR_AS_SLIDER_FLOAT(tank->acceleration, 0.1f, tank->max_velocity_forward);
	SHOW_IN_INSPECTOR_AS_SLIDER_FLOAT(tank->turning_velocity, 0.1f, 360.0f);
	SHOW_IN_INSPECTOR_AS_INPUT_FLOAT(tank->recoil);
	SHOW_IN_INSPECTOR_AS_DRAGABLE_FLOAT(tank->friction_force);
	SHOW_IN_INSPECTOR_AS_PREFAB(tank->bullet);
	return tank;
}
