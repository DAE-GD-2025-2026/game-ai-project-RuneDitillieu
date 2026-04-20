#pragma once

namespace GameAI::FSM
{
	class State
	{
	public:
		virtual ~State() = default;
		virtual void Tick(float deltaTime);
	};
}