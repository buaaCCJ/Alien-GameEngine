#include "ResourceModel.h"
#include "Globals.h"
#include "ResourceMesh.h"
#include "ModuleFileSystem.h"
#include "Application.h"

ResourceModel::ResourceModel() : Resource()
{
	type = ResourceType::RESOURCE_MODEL;
}

ResourceModel::~ResourceModel()
{
}

void ResourceModel::CreateMetaData()
{

	uint num_meshes = meshes_attached.size();

	// file size
	uint size = sizeof(num_meshes) + sizeof(char*) * num_meshes;

	char* data = new char[size]; // Allocate
	char* cursor = data;

	// save the number of meshes
	uint bytes = sizeof(num_meshes);

	memcpy(cursor, &num_meshes, bytes);
	cursor += bytes;

	std::vector<ResourceMesh*>::iterator item = meshes_attached.begin();
	for (; item != meshes_attached.end(); ++item) {
		if ((*item) != nullptr) {
			char* mesh_path[1] = { (*item)->CreateMetaData() };
			memcpy(cursor, mesh_path, sizeof(mesh_path));
			cursor += sizeof(mesh_path);
			LOG("Created alienMesh file %s", mesh_path[0]);
		}
	}

	// Create the file
	std::string output;
	App->file_system->SaveUnique(output, data, size, LIBRARY_MODELS_FOLDER, App->file_system->GetBaseFileName(path.data()).data(), ".alien");
	LOG("Created alien file %s", output.data());

	delete[] data;




}
