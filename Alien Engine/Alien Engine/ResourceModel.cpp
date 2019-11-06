#include "ResourceModel.h"
#include "Globals.h"
#include "ResourceMesh.h"
#include "ModuleFileSystem.h"
#include "Application.h"
#include <algorithm>

ResourceModel::ResourceModel() : Resource()
{
	type = ResourceType::RESOURCE_MODEL;
}

ResourceModel::~ResourceModel()
{
}

void ResourceModel::CreateMetaData()
{	
	ID = App->resources->GetRandomID();

	std::string alien_path = std::string(App->file_system->GetPathWithoutExtension(path) + "_meta.alien").data();

	uint size = sizeof(uint) + sizeof(ID);

	char* data = new char[size];
	char* cursor = data;

	uint bytes = sizeof(ID);
	memcpy(cursor, &ID, bytes);
	cursor += bytes;

	bytes = sizeof((uint)type);
	memcpy(cursor, &type, bytes);
	
	std::string output;
	App->file_system->SaveUnique(output, data, size, App->file_system->GetPathWithoutExtension(path).data(), "_meta", ".alien");

	meta_data_path = std::string(LIBRARY_MODELS_FOLDER) + std::string(std::to_string(ID) + ".alienModel");

	JSON_Value* model_value = json_value_init_object();
	JSON_Object* model_object = json_value_get_object(model_value);
	json_serialize_to_file_pretty(model_value, meta_data_path.data());
	
	if (model_value != nullptr && model_object != nullptr) {

		JSONfilepack* meta = new JSONfilepack(meta_data_path, model_object, model_value);

		meta->StartSave();

		meta->SetString("Model.Name", name);

		meta->SetNumber("Model.NumMeshes", meshes_attached.size());

		std::vector<ResourceMesh*>::iterator item = meshes_attached.begin();
		for (; item != meshes_attached.end(); ++item) {
			if ((*item) != nullptr) {
				(*item)->CreateMetaData();

				meta->SetArrayString("Model.PathMeshes", (*item)->GetLibraryPath());

				LOG("Created alienMesh file %s", (*item)->GetLibraryPath());
			}
		}
		// Create the file
		LOG("Created alien file %s", meta_data_path.data());

		meta->FinishSave();
	}
}

bool ResourceModel::ReadMetaData(const char* path)
{
	bool ret = true;

	JSON_Value* value = json_parse_file(path);
	JSON_Object* object = json_value_get_object(value);

	if (value != nullptr && object != nullptr)
	{
		JSONfilepack* meta = new JSONfilepack(path, object, value);

		meta_data_path = std::string(path);

		int num_meshes = meta->GetNumber("Model.NumMeshes");

		name = meta->GetString("Model.Name");

		for (uint i = 0; i < num_meshes; ++i) {
			std::string mesh_path = meta->GetArrayString("Model.PathMeshes", i);
			ResourceMesh* r_mesh = new ResourceMesh();
			if (r_mesh->ReadMetaData(mesh_path.data())) {
				meshes_attached.push_back(r_mesh);
			}
			else {
				LOG("Error loading %s", mesh_path.data());
				delete r_mesh;
			}
		}

		App->resources->AddResource(this);
	}

	return ret;
}

void ResourceModel::ChangeFileMetaName(const char* new_name)
{
	JSON_Value* value = json_parse_file(meta_data_path.data());
	JSON_Object* object = json_value_get_object(value);

	if (value != nullptr && object != nullptr)
	{
		JSONfilepack* meta = new JSONfilepack(meta_data_path.data(), object, value);

		meta->StartSave();

		name = new_name;
		
		std::string copy;
		bool start_copying = false;
		std::string::reverse_iterator item = path.rbegin();
		for (; item != path.rend(); ++item) {
			if (!start_copying) {
				if ((*item) == '/') {
					start_copying = true;
					copy = (*item);
				}
			}
			else {
				copy = (*item) + copy;
			}
		}

		std::string extension;
		App->file_system->SplitFilePath(path.data(), nullptr, nullptr, &extension);
		path = copy + new_name + "." + extension;


		meta->SetString("Model.Name", new_name);
		meta->SetString("Model.Path", path);

		meta->FinishSave();

		std::string new_path = std::string(LIBRARY_MODELS_FOLDER) + std::string(App->file_system->GetCurrentFolder(path) + App->file_system->GetBaseFileName(path.data())).data() + ".alienModel";
		rename(meta_data_path.data(), new_path.data());
		meta_data_path = new_path;
	}
}

void ResourceModel::ConvertToGameObjects()
{
	// TODO: save & load the family number

	if (meshes_attached.empty())
		return;

	std::sort(meshes_attached.begin(), meshes_attached.end(), ResourceModel::SortByFamilyNumber);

	if (meshes_attached.size() > 1) { // needs an empty gameobject
		
		// create the parent
		GameObject* parent = App->objects->CreateEmptyGameObject(nullptr);
		parent->SetName(name.data());

		// vector to find the parents
		std::vector<GameObject*> objects_created;
		objects_created.push_back(parent);

		std::vector<ResourceMesh*>::iterator item = meshes_attached.begin();
		for (; item != meshes_attached.end(); ++item) {
			if (*item != nullptr) {
				(*item)->ConvertToGameObject(&objects_created);
			}
		}
		objects_created.clear();

		// set it selected
		App->objects->SetNewSelectedObject(parent);
	}
	else { 
		meshes_attached.at(0)->ConvertToGameObject(nullptr);
		App->objects->SetNewSelectedObject(App->objects->base_game_object->children.back());
	}
	App->camera->Focus();
}

bool ResourceModel::SortByFamilyNumber(const ResourceMesh* mesh1, const ResourceMesh* mesh2)
{
	return mesh1->family_number < mesh2->family_number;
}
