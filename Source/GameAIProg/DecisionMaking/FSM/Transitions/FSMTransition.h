#pragma once

namespace GameAI::FSM
{
	class Transition
	{
	public:
		virtual ~Transition() = default;
		virtual type_info Tick(float deltaTime);
		type_info& GetFromStateType() { return m_FromStateType; }
	
	private:
		type_info m_FromStateType;
		type_info m_ToStateType;
	};
}
