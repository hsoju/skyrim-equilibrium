#include "InputEventHandler.h"

void InputEventHandler::CheckInitialInput(bool& hand_down, std::string& hand_ready_graph_variable) {
	bool can_cast = recovery_handler->GetCastingGraphVariable(RE::PlayerCharacter::GetSingleton(),
		hand_ready_graph_variable);
	ready_to_cast = can_cast;
	hand_down = can_cast;
}

void InputEventHandler::HandleInitialInput(RE::InputEvent* const* a_event)
{
	if (a_event) {
		for (RE::InputEvent* given_input = *a_event; given_input; given_input = given_input->next) {
			if (given_input->eventType.get() != RE::INPUT_EVENT_TYPE::kButton) {
				continue;
			}
			
			const RE::ButtonEvent* given_button = given_input->AsButtonEvent();
			if (!given_button->IsDown()) {
				continue;
			}
			const RE::IDEvent* given_id = given_input->AsIDEvent();
			const auto& given_user = given_id->userEvent;
			const auto user_events = RE::UserEvents::GetSingleton();

			if (!right_down && given_user == user_events->rightAttack) {
				CheckInitialInput(right_down, recovery_handler->right_ready_graph_variable);
				break;
			} else if (!left_down && given_user == user_events->leftAttack) {
				CheckInitialInput(left_down, recovery_handler->left_ready_graph_variable);
				break;
			}
		}
	}
}

void InputEventHandler::CheckHeldInput(const RE::ButtonEvent* given_button, bool use_left_hand)
{
	if (given_button->IsUp()) {
		bool& hand_down = use_left_hand ? left_down : right_down;
		hand_down = false;
		recovery_handler->has_recovered_map[use_left_hand] = false;
		if (!right_down && !left_down) {
			ready_to_cast = false;
		}
	} else {
		if (!(right_down && left_down)) {
			recovery_handler->CheckCast(use_left_hand);
		} else {
			recovery_handler->CheckDualCast(use_left_hand);
		}
	}
}

void InputEventHandler::HandleHeldInput(RE::InputEvent* const* a_event)
{
	if (a_event) {
		for (RE::InputEvent* given_input = *a_event; given_input; given_input = given_input->next) {
			if (given_input->eventType.get() != RE::INPUT_EVENT_TYPE::kButton) {
				continue;
			}
			
			const RE::ButtonEvent* given_button = given_input->AsButtonEvent();
			if (!(given_button->IsHeld() || given_button->IsUp())) {
				continue;
			}
			const RE::IDEvent* given_id = given_input->AsIDEvent();
			const auto& given_user = given_id->userEvent;
			const auto user_events = RE::UserEvents::GetSingleton();

			if (right_down && given_user == user_events->rightAttack) {
				CheckHeldInput(given_button, false);
				break;
			} else if (left_down && given_user == user_events->leftAttack) {
				CheckHeldInput(given_button, true);
				break;
			}
		}
	}
}
