#include <SimpleIni.h>
#include "RecoveryHandler.h"

bool RecoveryHandler::GetCastingGraphVariable(RE::PlayerCharacter* player, std::string& variable_name) {
	bool is_casting = false;
	player->GetGraphVariableBool(variable_name, is_casting);
	return is_casting;
}

bool RecoveryHandler::ApplyRecovery(bool use_left_hand, RE::ActorValueOwner* av_owner, RE::ActorValue left_value, 
	RE::ActorValue right_value)
{
	RE::ActorValue drain_value = use_left_hand ? left_value : right_value;
	float current_drain_value = av_owner->GetActorValue(drain_value);
	if (current_drain_value > 2.0f) {
		av_owner->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kMagicka, drain_amount);
		av_owner->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, drain_value, -drain_amount);
		return true;
	}
	return false;
}

void RecoveryHandler::CheckCast(bool use_left_hand) {
	RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
	RE::TESForm* current_spell = player->GetEquippedObject(use_left_hand);
	if (current_spell && current_spell->GetFormType() == RE::FormType::Spell) {
		if (!GetCastingGraphVariable(player, graph_variable_map[use_left_hand])) {
			auto av_owner = player->AsActorValueOwner();
			if (!ApplyRecovery(use_left_hand, av_owner, left_hand_actor_value, right_hand_actor_value) && use_fallback) {
				ApplyRecovery(use_left_hand, av_owner, left_hand_alternate_value, right_hand_alternate_value);
			}
		}
	}
}

void RecoveryHandler::AssignValue(const char* ini_value, RE::ActorValue& actor_value, RE::ActorValue& alternate_value) {
	std::string s_value(ini_value);
	for (auto& character : s_value) {
		character = std::tolower(character);
	}
	if (s_value == "stamina") {
		actor_value = RE::ActorValue::kStamina;
		alternate_value = RE::ActorValue::kHealth;
	} else if (s_value == "health") {
		actor_value = RE::ActorValue::kHealth;
		alternate_value = RE::ActorValue::kStamina;
	}
}

void RecoveryHandler::ImportSettings() {
	std::lock_guard<std::shared_mutex> lk(settings_mtx);
	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(L"Data\\SKSE\\Plugins\\Equilibrium.ini");

	const char* right_value = ini.GetValue("Settings", "sRightHand", "Stamina");
	const char* left_value = ini.GetValue("Settings", "sLeftHand", "Stamina");
	AssignValue(right_value, right_hand_actor_value, right_hand_alternate_value);
	AssignValue(left_value, left_hand_actor_value, left_hand_alternate_value);

	use_fallback = ini.GetBoolValue("Settings", "bFallback", false);
}