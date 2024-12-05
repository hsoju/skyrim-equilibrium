#pragma once
#include "RecoveryHandler.h"

bool inputs_tracked = false;

bool ready_to_cast = false;
bool right_down = false;
bool left_down = false;

class InputEventHandler : public RE::BSTEventSink<RE::InputEvent*>
{
private:
	RecoveryHandler* recovery_handler = RecoveryHandler::GetSingleton();

	void CheckInitialInput(bool& hand_down, std::string& hand_ready_graph_variable);
	void CheckHeldInput(const RE::ButtonEvent* given_button, bool use_left_hand);

public:
	void HandleInitialInput(RE::InputEvent* const* a_event);
	void HandleHeldInput(RE::InputEvent* const* a_event);

	virtual RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*)
	{
		if (!ready_to_cast) {
			HandleInitialInput(a_event);
		} else {
			if (!right_down || !left_down) {
				HandleInitialInput(a_event);
			}
			HandleHeldInput(a_event);
		}
		return RE::BSEventNotifyControl::kContinue;
	}

	static bool Register()
	{
		static InputEventHandler singleton;

		auto input_manager = RE::BSInputDeviceManager::GetSingleton();

		if (!input_manager) {
			logger::error("Input event source not found");
			return false;
		}
		input_manager->AddEventSink(&singleton);
		logger::info("Registered {}", typeid(singleton).name());
		inputs_tracked = true;
		return true;
	}
};