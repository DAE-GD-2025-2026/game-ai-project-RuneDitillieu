#pragma once
#include "FSMState.h"

namespace GameAI::FSM
{
	class Search : public State
	{
	public:
		virtual void Tick(float deltaTime) override;
	};
}
