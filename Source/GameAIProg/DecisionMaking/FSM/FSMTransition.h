#pragma once
#include <functional>

namespace GameAI::FSM
{
	class Transition
	{
	public:
		Transition(State* From, State* To, std::function<bool()> EvalFunc)
			: m_FromState(From), m_ToState(To), m_EvalFunc(EvalFunc) {}
		virtual ~Transition() = default;
		virtual bool Evaluate() { return m_EvalFunc(); }
		State* GetFromState() { return m_FromState; }
		State* GetToState() { return m_ToState; }
	
	private:
		State* m_FromState;
		State* m_ToState;
		std::function<bool()> m_EvalFunc;
	};
}
