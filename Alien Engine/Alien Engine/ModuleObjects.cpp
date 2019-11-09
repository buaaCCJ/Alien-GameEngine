#include "Globals.h"
#include "ModuleObjects.h"
#include "GameObject.h"
#include "ModuleInput.h"
#include "glew/include/glew.h"
#include "Application.h"
#include <algorithm>
#include "ComponentTransform.h"
#include "ComponentMaterial.h"
#include "ComponentMesh.h"
#include "ComponentLight.h"

ModuleObjects::ModuleObjects(bool start_enabled):Module(start_enabled)
{
	name.assign("ModuleObject");
}

ModuleObjects::~ModuleObjects()
{
}

bool ModuleObjects::Init()
{
	base_game_object = new GameObject(nullptr);
	base_game_object->ID = 0;
	//base_game_object->AddComponent(new ComponentTransform(base_game_object, { 0,0,0 }, { 0,0,0,0 }, { 1000,1000,1000 }));

	return true;
}

bool ModuleObjects::Start()
{
	LOG("Starting Module Objects");
	bool ret = true;

	GameObject* light_test = new GameObject(base_game_object);
	light_test->SetName("Light");
	light_test->AddComponent(new ComponentTransform(light_test, { 0,0,2.5f }, { 0,0,0,0 }, { 1,1,1 }));
	light_test->AddComponent(new ComponentLight(light_test));

	current_scene.name_without_extension = "Untitled*";

	return ret;
}

update_status ModuleObjects::PreUpdate(float dt)
{
	// delete objects
	if (need_to_delete_objects) { 
		need_to_delete_objects = false;
		base_game_object->SearchToDelete();
	}

	// change parent
	if (!to_reparent.empty()) {
		std::map<GameObject*, GameObject*>::iterator item = to_reparent.begin();
		for (; item != to_reparent.end(); ++item) {
			if ((*item).first != nullptr && (*item).second != nullptr) {
				(*item).first->SetNewParent((*item).second);
			}
		}
		to_reparent.clear();
	}

	return UPDATE_CONTINUE;
}

update_status ModuleObjects::Update(float dt)
{

	return UPDATE_CONTINUE;
}

update_status ModuleObjects::PostUpdate(float dt)
{

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, App->renderer3D->frame_buffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearStencil(0);
	if (allow_grid)
		App->renderer3D->RenderGrid();

	base_game_object->Draw();

	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	
	return UPDATE_CONTINUE;
}

bool ModuleObjects::CleanUp()
{

	delete base_game_object;
	base_game_object = nullptr;

	return true;
}

void ModuleObjects::ChangeWireframeMode()
{
	base_game_object->ChangeWireframe(wireframe_mode);
}

void ModuleObjects::ChangeViewMeshMode()
{
	base_game_object->ChangeMeshView(view_mesh_mode);
}

void ModuleObjects::DeleteAllObjects() 
{
	std::vector<GameObject*>::iterator item = base_game_object->children.begin();
	while (item != base_game_object->children.end()) {
		if (*item != nullptr) {
			delete* item;
			*item = nullptr;
			item = base_game_object->children.erase(item);
		}
		else
			++item;
	}
	game_object_selected = nullptr;
}

void ModuleObjects::ChangeEnableGrid()
{
	allow_grid = !allow_grid;
}

void ModuleObjects::ChangeEnableNormalVertex()
{
	base_game_object->ChangeVertexNormalsView(draw_vertex_normals);
}

void ModuleObjects::ChangeEnableNormalFace()
{
	base_game_object->ChangeFaceNormalsView(draw_face_normals);
}

void ModuleObjects::ChangeEnableAABB()
{
	base_game_object->ChangeAABB(draw_all_AABB);
}

void ModuleObjects::ChangeEnableOBB()
{
	base_game_object->ChangeOBB(draw_all_OBB);
}

void ModuleObjects::SetNewSelectedObject(GameObject* object_selected)
{
	if (!object_selected->IsSelected()) {
		if (game_object_selected == nullptr) {
			object_selected->ChangeSelected(true);
			game_object_selected = object_selected;
		}
		else if (object_selected != game_object_selected) {
			game_object_selected->ChangeSelected(false);
			object_selected->ChangeSelected(true);
			game_object_selected = object_selected;
		}
	}
}

GameObject* ModuleObjects::GetSelectedObject()
{
	return game_object_selected;
}

void ModuleObjects::DeselectObject()
{
	game_object_selected->ChangeSelected(false);
	game_object_selected = nullptr;
}

GameObject* ModuleObjects::CreateEmptyGameObject(GameObject* parent, bool set_selected)
{
	GameObject* object = nullptr;

	if (parent != nullptr) {
		object = new GameObject(parent);
		object->SetName("Empty Child");
	}
	else {
		object = new GameObject(base_game_object);
		object->SetName("Empty GameObject");
	}

	object->AddComponent(new ComponentTransform(object, { 0,0,0 }, { 0,0,0,0 }, { 1,1,1 }));
	
	if (set_selected)
		SetNewSelectedObject(object);

	return object;
}

void ModuleObjects::MoveObjectDown(GameObject* object, bool bottom)
{
	GameObject* parent = object->parent;

	if (object == parent->children.back())
		return;

	if (bottom) {
		parent->children.erase(std::find(parent->children.begin(), parent->children.end(), object));
		parent->AddChild(object);
	}
	else {
		std::vector<GameObject*>::iterator item = parent->children.begin();
		for (; item != parent->children.end(); ++item) {
			if (*item != nullptr) {
				if (*item == object) {
					item = parent->children.erase(item);
					parent->children.insert(++item, object);
					break;
				}
			}
		}
	}
}

void ModuleObjects::MoveObjectUp(GameObject* object, bool top)
{
	GameObject* parent = object->parent;

	if (object == parent->children.front())
		return;

	if (top) {
		parent->children.erase(std::find(parent->children.begin(), parent->children.end(), object));
		parent->children.insert(parent->children.begin(), object);
	}
	else {
		std::vector<GameObject*>::iterator item = parent->children.begin();
		for (; item != parent->children.end(); ++item) {
			if (*item != nullptr) {
				if (*item == object) {
					item = parent->children.erase(item);
					parent->children.insert(--item, object);
					break;
				}
			}
		}
	}
}

void ModuleObjects::MoveComponentDown(GameObject* object, Component* component, bool bottom)
{
	if (component == object->components.back())
		return;

	if (bottom) {
		object->components.erase(std::find(object->components.begin(), object->components.end(), component));
		object->components.push_back(component);
	}
	else {
		std::vector<Component*>::iterator item = object->components.begin();
		for (; item != object->components.end(); ++item) {
			if (*item != nullptr) {
				if (*item == component) {
					item = object->components.erase(item);
					object->components.insert(++item, component);
					break;
				}
			}
		}
	}
}

void ModuleObjects::MoveComponentUp(GameObject* object, Component* component, bool top)
{
	if (component == object->components.at(1))
		return;

	if (top) {
		object->components.erase(std::find(object->components.begin(), object->components.end(), component));
		object->components.insert(object->components.begin() + 1, component);
	}
	else {
		std::vector<Component*>::iterator item = object->components.begin();
		for (; item != object->components.end(); ++item) {
			if (*item != nullptr) {
				if (*item == component) {
					item = object->components.erase(item);
					object->components.insert(--item, component);
					break;
				}
			}
		}
	}
}

GameObject* ModuleObjects::GetGameObjectByID(const u64& id)
{
	return base_game_object->GetGameObjectByID(id);
}

void ModuleObjects::ReparentGameObject(GameObject* object, GameObject* next_parent)
{
	if (object != nullptr && next_parent != nullptr && !object->Exists(next_parent)) {
		to_reparent.emplace(object, next_parent);
	}
}

void ModuleObjects::SaveScene(const char* path)
{
	JSON_Value* value = json_value_init_object();
	JSON_Object* object = json_value_get_object(value);
	json_serialize_to_file_pretty(value, path);

	if (value != nullptr && object != nullptr)
	{
		JSONfilepack* scene = new JSONfilepack(path, object, value);

		scene->StartSave();

		JSONArraypack*  game_objects = scene->InitNewArray("Scene.GameObjects");
		
		game_objects->SetAnotherNode();

		std::vector<GameObject*>::iterator item = base_game_object->children.begin();
		for (; item != base_game_object->children.end(); ++item) {
			if (*item != nullptr) {
				SaveGameObject(*item, game_objects, 1);
				if ((*item) != base_game_object->children.back())
					game_objects->SetAnotherNode();
			}
		}

		scene->FinishSave();
		delete scene;
	}
	else {
		LOG("Could not load scene, fail when creating the file");
	}
}

void ModuleObjects::LoadScene(const char* path)
{
	JSON_Value* value = json_parse_file(path);
	JSON_Object* object = json_value_get_object(value);
	// TODO: delete all scene before that and set again the base game object. Add a popup to accept the load
	if (value != nullptr && object != nullptr)
	{
		JSONfilepack* scene = new JSONfilepack(path, object, value);

		JSONArraypack* game_objects = scene->GetArray("Scene.GameObjects");

		std::vector<std::pair<uint, u64>> objects_to_create;

		for (uint i = 0; i < game_objects->GetArraySize(); ++i) {
			uint family_number = game_objects->GetNumber("FamilyNumber");
			u64 parentID = std::stoull(game_objects->GetString("ParentID"));
			objects_to_create.push_back({ family_number,parentID });
			game_objects->GetAnotherNode();
		}
		std::sort(objects_to_create.begin(), objects_to_create.end(), ModuleObjects::SortByFamilyNumber);
		game_objects->GetFirstNode();
		std::vector<GameObject*> objects_created;

		std::vector<std::pair<uint, u64>>::iterator item = objects_to_create.begin();
		for (; item != objects_to_create.end(); ++item) {
			GameObject* obj = new GameObject();
			if ((*item).first == 1) { // family number == 1 so parent is the base game object
				obj->LoadObject(game_objects, base_game_object);
			}
			else { // search parent
				std::vector<GameObject*>::iterator objects = objects_created.begin();
				for (; objects != objects_created.end(); ++objects) {
					if ((*objects)->ID == (*item).second) {
						obj->LoadObject(game_objects, *objects);
						break;
					}
				}
			}
			objects_created.push_back(obj);
			game_objects->GetAnotherNode();
		}
		delete scene;
	}
	else {
		LOG("Error loading scene %s", path);
	}
}

void ModuleObjects::SaveGameObject(GameObject* obj, JSONArraypack* to_save, const uint& family_number)
{
	obj->SaveObject(to_save, family_number);

	std::vector<GameObject*>::iterator item = obj->children.begin();
	for (; item != obj->children.end(); ++item) {
		if (*item != nullptr) {
			to_save->SetAnotherNode();
			SaveGameObject(*item, to_save, family_number + 1);
		}
	}
}

bool ModuleObjects::SortByFamilyNumber(std::pair<uint,u64> pair1, std::pair<uint, u64> pair2)
{
	return pair1.first < pair2.first;
}

void ModuleObjects::LoadConfig(JSONfilepack*& config) 
{
	wireframe_mode = config->GetBoolean("Configuration.Renderer.Wireframe");
	view_mesh_mode = config->GetBoolean("Configuration.Renderer.MeshView");
	draw_vertex_normals = config->GetBoolean("Configuration.Renderer.VertexNormals");
	draw_face_normals = config->GetBoolean("Configuration.Renderer.FaceNormals");
	mesh_line_width = config->GetNumber("Configuration.Renderer.MeshWidth");
	face_n_width = config->GetNumber("Configuration.Renderer.FaceNormalsWidth");
	vertex_n_width = config->GetNumber("Configuration.Renderer.VertexNormalsWidth");
	vertex_normal_length = config->GetNumber("Configuration.Renderer.VertexNormalsLength");
	face_normal_length = config->GetNumber("Configuration.Renderer.FaceNormalsLength");
	allow_grid = config->GetBoolean("Configuration.Renderer.AllowGrid");
	App->renderer3D->grid_spacing = config->GetNumber("Configuration.Renderer.GridSpacing");
	App->renderer3D->length_grid = config->GetNumber("Configuration.Renderer.GridLength");
	App->renderer3D->line_grid_width = config->GetNumber("Configuration.Renderer.GridWidth");
	App->renderer3D->grid_color = config->GetColor("Configuration.Renderer.GridColor");
	App->renderer3D->background_color = config->GetColor("Configuration.Renderer.BackgroundColor");
	vertex_n_color = config->GetColor("Configuration.Renderer.VertexNormalColor");
	face_n_color = config->GetColor("Configuration.Renderer.FaceNormalColor");
	mesh_color = config->GetColor("Configuration.Renderer.MeshColor");
	parent_outline_color = config->GetColor("Configuration.Renderer.ParentOutlineColor");
	no_child_outline_color = config->GetColor("Configuration.Renderer.NoChildOutlineColor");
	outline = config->GetBoolean("Configuration.Renderer.Outline");
	parent_line_width = config->GetNumber("Configuration.Renderer.ParentLineWidth");
	no_child_line_width = config->GetNumber("Configuration.Renderer.NoChildLineWidth");
}

void ModuleObjects::SaveConfig(JSONfilepack*& config)
{
	config->SetBoolean("Configuration.Renderer.Wireframe", wireframe_mode);
	config->SetBoolean("Configuration.Renderer.MeshView", view_mesh_mode);
	config->SetBoolean("Configuration.Renderer.VertexNormals", draw_vertex_normals);
	config->SetBoolean("Configuration.Renderer.FaceNormals", draw_face_normals);
	config->SetNumber("Configuration.Renderer.MeshWidth", mesh_line_width);
	config->SetNumber("Configuration.Renderer.FaceNormalsWidth", face_n_width);
	config->SetNumber("Configuration.Renderer.VertexNormalsWidth", vertex_n_width);
	config->SetNumber("Configuration.Renderer.VertexNormalsLength", vertex_normal_length);
	config->SetNumber("Configuration.Renderer.FaceNormalsLength", face_normal_length);
	config->SetBoolean("Configuration.Renderer.AllowGrid", allow_grid);
	config->SetNumber("Configuration.Renderer.GridSpacing", App->renderer3D->grid_spacing);
	config->SetNumber("Configuration.Renderer.GridWidth", App->renderer3D->line_grid_width);
	config->SetNumber("Configuration.Renderer.GridLength", App->renderer3D->length_grid);
	config->SetColor("Configuration.Renderer.GridColor", App->renderer3D->grid_color);
	config->SetColor("Configuration.Renderer.BackgroundColor", App->renderer3D->background_color);
	config->SetColor("Configuration.Renderer.VertexNormalColor", vertex_n_color);
	config->SetColor("Configuration.Renderer.FaceNormalColor", face_n_color);
	config->SetColor("Configuration.Renderer.MeshColor", mesh_color);
	config->SetColor("Configuration.Renderer.ParentOutlineColor", parent_outline_color);
	config->SetColor("Configuration.Renderer.NoChildOutlineColor", no_child_outline_color);
	config->SetBoolean("Configuration.Renderer.Outline", outline);
	config->SetNumber("Configuration.Renderer.ParentLineWidth", parent_line_width);
	config->SetNumber("Configuration.Renderer.NoChildLineWidth", no_child_line_width);
}

void ModuleObjects::CreateBasePrimitive(PrimitiveType type)
{
	GameObject* object = new GameObject(App->objects->base_game_object);
	ComponentTransform* transform = new ComponentTransform(object, { 0,0,0 }, { 0,0,0,0 }, { 1,1,1 });
	ComponentMesh* mesh = new ComponentMesh(object);
	ComponentMaterial* material = new ComponentMaterial(object);
	par_shapes_mesh* par_mesh = nullptr;
	
	switch (type) {
	case PrimitiveType::CUBE:
		par_mesh = par_shapes_create_cube();
		object->SetName("Cube");
		break;
	case PrimitiveType::DODECAHEDRON:
		par_mesh = par_shapes_create_dodecahedron();
		object->SetName("Dodecahedron");
		break;
	case PrimitiveType::ICOSAHEDRON:
		par_mesh = par_shapes_create_icosahedron();
		object->SetName("Icosahedron");
		break;
	case PrimitiveType::OCTAHEDRON:
		par_mesh = par_shapes_create_octahedron();
		object->SetName("Octahedron");
		break;
	case PrimitiveType::ROCK:
		par_mesh = par_shapes_create_rock(5, 3);
		object->SetName("Rock");
		break;
	case PrimitiveType::SPHERE_ALIEN:
		par_mesh = par_shapes_create_subdivided_sphere(4);
		object->SetName("Sphere");
		break;
	case PrimitiveType::TORUS:
		par_mesh = par_shapes_create_torus(3, 10, 0.5F);
		object->SetName("Torus");
		break;
	case PrimitiveType::KLEIN_BOTTLE:
		par_mesh = par_shapes_create_klein_bottle(10, 10);
		object->SetName("Klein Bottle");
		break;
	default:
		break;
	}
	App->importer->LoadParShapesMesh(par_mesh, mesh);
	object->AddComponent(transform);
	object->AddComponent(mesh);
	object->AddComponent(material);
	SetNewSelectedObject(object);
	par_shapes_free_mesh(par_mesh);
}

