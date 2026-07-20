#include "SpawnCommand.h"

#include "jsonEditor/JsonSerialization.h" // Vector3のto_json/from_json

void to_json(nlohmann::json& j, const SpawnCommand& command)
{
	j = nlohmann::json{
		{"timing", command.timing},
		{"type", command.type},
		{"position", command.position},
	};
}

void from_json(const nlohmann::json& j, SpawnCommand& command)
{
	j.at("timing").get_to(command.timing);
	j.at("type").get_to(command.type);
	j.at("position").get_to(command.position);
}
