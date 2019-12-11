#pragma once

#include "MathGeoLib/include/Math/float4x4.h"
#include "MathGeoLib/include/Math/float3.h"
#include <vector>
#include "Color.h"
#include <tuple>
#include "ModuleObjects.h"

class ComponentMesh;
class ResourceMesh;
enum class PrimitiveType;

class __declspec(dllexport) Gizmos {
	friend class Gizmos;
	friend class ModuleObjects;
public:

	static void DrawCube(float3 position, float3 size, Color color);
	static void DrawWireCube(float3 position, float3 size, Color color, float line_width = 2.0F);
	static void DrawSphere(float3 position, float radius, Color color);
	static void DrawWireSphere(float3 position, float radius, Color color, float line_width = 2.0F);
	static void DrawLine(float3 from, float3 to, Color color, float line_width = 3.0F);
	static void DrawWireMesh(ComponentMesh* mesh, float4x4 global_transform, Color color, float line_width = 2.0F);
	// TODO: when UI available
	// static void DrawGUITexture();

private:

	static void DrawPoly(ResourceMesh* mesh, const float4x4& matrix, const Color& color);
	static void DrawWire(ResourceMesh* mesh, const float4x4& matrix, const Color& color, float line_width);
	static void RemoveGizmos();
	static void ClearAllCurrentGizmos();

private:
	struct Gizmo {
		ResourceMesh* mesh = nullptr;
		bool controller = true;
		PrimitiveType type = PrimitiveType::UNKONWN;
	};
	static std::vector<Gizmo> active_gizmos;
	static bool controller;
};
