#include <SimpleIni.h>
#include "RecoveryHandler.h"

bool RecoveryHandler::GetCastingGraphVariable(RE::PlayerCharacter* player, std::string& variable_name) {
	bool is_casting = false;
	player->GetGraphVariableBool(variable_name, is_casting);
	return is_casting;
}

bool RecoveryHandler::HasMaxMagicka(RE::ActorValueOwner* av_owner) {
	float current_value = av_owner->GetActorValue(RE::ActorValue::kMagicka);
	float permanent_value = av_owner->GetPermanentActorValue(RE::ActorValue::kMagicka);
	return current_value >= (permanent_value - 1.f);
}

void RecoveryHandler::UpdateRecoveryMap(bool use_left_hand, bool has_recovered) {
	if (has_recovered && !has_recovered_map[use_left_hand]) {
		has_recovered_map[use_left_hand] = has_recovered;
	}
}

bool RecoveryHandler::ApplyRecovery(bool use_left_hand, RE::ActorValueOwner* av_owner, RE::ActorValue left_value, 
	RE::ActorValue right_value)
{
	RE::ActorValue drain_value = use_left_hand ? left_value : right_value;
	float current_drain_value = av_owner->GetActorValue(drain_value);
	if (current_drain_value > 2.f) {
		av_owner->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kMagicka, regen_amount);
		av_owner->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, drain_value, -drain_amount);
		return true;
	}
	return false;
}

void RecoveryHandler::ChooseRecovery(bool use_left_hand, RE::ActorValueOwner* av_owner)
{
	bool has_recovered = ApplyRecovery(use_left_hand, av_owner, left_hand_actor_value, right_hand_actor_value);
	if (!has_recovered && use_fallback) {
		has_recovered = ApplyRecovery(use_left_hand, av_owner, left_hand_alternate_value, right_hand_alternate_value);
	}
	UpdateRecoveryMap(use_left_hand, has_recovered);
}

void RecoveryHandler::CheckMagickaAmount(bool use_left_hand, RE::PlayerCharacter* player) {
	auto av_owner = player->AsActorValueOwner();
	if (!HasMaxMagicka(av_owner)) {
		ChooseRecovery(use_left_hand, av_owner);
	}
}

void RecoveryHandler::CheckCannotCastSpell(bool use_left_hand, RE::PlayerCharacter* player, RE::TESForm* current_form, 
	bool is_dualcast) {
	RE::SpellItem* current_spell = current_form->As<RE::SpellItem>();
	auto cast_reason = RE::MagicSystem::CannotCastReason();
	bool can_cast = player->As<RE::Actor>()->CheckCast(current_spell, is_dualcast, &cast_reason);
	if (!can_cast) {
		CheckMagickaAmount(use_left_hand, player);
	} else if (has_recovered_map[use_left_hand]) {
		// TODO: Start casting
		has_recovered_map[use_left_hand] = false;
	}
}

void RecoveryHandler::CheckIsNotCasting(bool use_left_hand, RE::PlayerCharacter* player, RE::TESForm* current_form, 
	bool is_dualcast) {
	bool is_casting = GetCastingGraphVariable(player, graph_variable_map[use_left_hand]);
	if (is_dualcast) {
		is_casting = is_casting || GetCastingGraphVariable(player, graph_variable_map[!use_left_hand]);
	}
	if (!is_casting) {
		CheckCannotCastSpell(use_left_hand, player, current_form, is_dualcast);
	}
}

bool RecoveryHandler::CheckHasSpellEquipped(RE::TESForm* current_form) {
	return current_form && current_form->GetFormType() == RE::FormType::Spell;
}

void RecoveryHandler::CheckCast(bool use_left_hand) {
	RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
	RE::TESForm* current_form = player->GetEquippedObject(use_left_hand);
	if (CheckHasSpellEquipped(current_form)) {
		CheckIsNotCasting(use_left_hand, player, current_form, false);
	}
}

void RecoveryHandler::CheckDualCast(bool use_left_hand) {
	RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
	RE::TESForm* current_form = player->GetEquippedObject(use_left_hand);
	if (CheckHasSpellEquipped(current_form)) {
		RE::TESForm* other_form = player->GetEquippedObject(!use_left_hand);
		if (other_form && other_form == current_form) {
			CheckIsNotCasting(use_left_hand, player, current_form, true);
		} else {
			CheckIsNotCasting(use_left_hand, player, current_form, false);
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