#pragma once
#include <shared_mutex>

class RecoveryHandler
{
private:
	std::shared_mutex settings_mtx;

	RE::ActorValue right_hand_actor_value = RE::ActorValue::kStamina;
	RE::ActorValue right_hand_alternate_value = RE::ActorValue::kHealth;
	RE::ActorValue left_hand_actor_value = RE::ActorValue::kStamina;
	RE::ActorValue left_hand_alternate_value = RE::ActorValue::kHealth;
	bool use_fallback = false;

	float regen_amount = 0.75f;
	float drain_amount = 1.25f;

	std::unordered_map<bool, std::string> graph_variable_map = { { true, std::string("bWantCastLeft") },
		{ false, std::string("bWantCastRight") } };

	void AssignValue(const char* ini_value, RE::ActorValue& actor_value, RE::ActorValue& alternate_value);
	bool ApplyRecovery(bool use_left_hand, RE::ActorValueOwner* av_owner, RE::ActorValue left_value, RE::ActorValue right_value);

public:
	std::string right_ready_graph_variable = "bMRh_Ready";
	std::string left_ready_graph_variable = "bMLh_Ready";

	static RecoveryHandler* GetSingleton()
	{
		static RecoveryHandler singleton;
		return &singleton;
	}

	bool GetCastingGraphVariable(RE::PlayerCharacter* player, std::string& variable_name);
	void CheckCast(bool use_left_hand);
	void ImportSettings();

protected:
	RecoveryHandler() = default;
	RecoveryHandler(const RecoveryHandler&) = delete;
	RecoveryHandler(RecoveryHandler&&) = delete;
	virtual ~RecoveryHandler() = default;

	auto operator=(const RecoveryHandler&) -> RecoveryHandler& = delete;
	auto operator=(RecoveryHandler&&) -> RecoveryHandler& = delete;
};