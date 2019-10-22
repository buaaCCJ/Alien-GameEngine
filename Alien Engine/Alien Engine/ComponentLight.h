#pragma once

#include "Component.h"
#include "Color.h"

class ComponentLight : public Component {
public:
	ComponentLight(GameObject* attach);
	virtual ~ComponentLight();

	void LightLogic();

	void DrawInspector();
	void OnDisable();

	void Reset();
	void SetComponent(Component* component);

private:

	Color ambient{1,1,1};
	Color diffuse{1,1,1};

	uint light_id=0;
};