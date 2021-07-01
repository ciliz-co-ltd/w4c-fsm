// (c) 2019-2021 Ciliz::W4
// Part of Ciliz W4 Game Creation SDK and Ciliz games codebase

template<typename EventType>
void FuncState<EventType>::onEnter(IFSM<EventType>& fsm, const EventType& event, const w4c::Variant& params)
{
    if (onEnterFunc)
    {
        onEnterFunc(fsm, event, params);
    }
}

template<typename EventType>
void FuncState<EventType>::onLeave(IFSM<EventType>& fsm, const EventType& event, const w4c::Variant& params)
{
    if (onLeaveFunc)
    {
        onLeaveFunc(fsm, event, params);
    }
}

template<typename EventType>
void FuncState<EventType>::onUpdate(IFSM<EventType>& fsm, float dt)
{
    if (onUpdateFunc)
    {
        onUpdateFunc(fsm, dt);
    }
}

template<typename... Types>
template<typename... Args>
States<Types...>::States(Args... args)
        : Types(args...)...
{}

template<typename... Types>
template<typename Type>
constexpr size_t States<Types...>::getStateIdx()
{
    constexpr int res = TypeList_IndexOf<Type, Types...>::value;
    static_assert(res >= 0, "State not found in state list");
    return res;
}

template<typename... Types>
constexpr size_t States<Types...>::nStates()
{
    return sizeof...(Types);
}

template<typename... Types>
template<typename EventType, typename FSM>
void States<Types...>::initCallbacks(FSM& fsm)
{
    initCallbacks_impl<EventType>(fsm, BuildIndices<sizeof...(Types)>{});
}

template<typename... Types>
template<typename EventType, typename FSM, typename Type, size_t idx>
void States<Types...>::initCallbacks_impl_Typed(FSM& fsm)
{
    fsm.m_onEnterCallbacks[idx] = &States<Types...>::template onEnter<Type, EventType>;
    fsm.m_onLeaveCallbacks[idx] = &States<Types...>::template onLeave<Type, EventType>;
    fsm.m_ProcessSubEvent[idx] = &States<Types...>::template processEvent<Type, EventType>;
    fsm.m_UpdateEvent[idx] = &States<Types...>::template onUpdate<Type>;
}

template<typename... Types>
template<typename EventType, typename FSM, size_t... Is>
void States<Types...>::initCallbacks_impl(FSM& fsm, Indices<Is...>)
{
    (initCallbacks_impl_Typed<EventType, FSM, Types, Is>(fsm), ...);
}

template<auto event, typename FromState, typename ToState>
template<typename States>
void Transition<event, FromState, ToState>::initTransitions(std::unordered_map<size_t , std::unordered_map<decltype(event), size_t>>& collection)
{
    collection[States::template getStateIdx<FromState>()][event] = States::template getStateIdx<ToState>();
}

template<typename T1, typename... Transition>
template <typename States, typename EventType>
void Transitions<T1, Transition...>::initTransitions(std::unordered_map<size_t , std::unordered_map<EventType, size_t>>& collection)
{
    T1::template initTransitions<States>(collection);
    (Transition::template initTransitions<States>(collection), ...);
}

template<typename... FsmTransition>
template<typename... Args>
FSM<FsmTransition...>::FSM(Args... args)
        : m_states(args...)
{
    Transitions::template initTransitions<States, EventType>(m_transitions);
    m_states.template initCallbacks<EventType>(*this);
}

template<typename... FsmTransition>
bool FSM<FsmTransition...>::processEvent(const EventType& event, const Variant& params)
{
    if (!m_inited)
    {
        return false;
    }

    auto fromIdx = m_currentState;

    if ((m_states.*m_ProcessSubEvent[fromIdx])(*this, event, params))
    {
        return true;
    }

    auto it = m_transitions.find(fromIdx);
    if (it == m_transitions.end())
    {
        return false;
    }

    auto& transitionsMap = it->second;
    auto toIt = transitionsMap.find(event);

    if (toIt == transitionsMap.end())
    {
        return false;
    }

    auto toIdx = toIt->second;

    (m_states.*m_onLeaveCallbacks[fromIdx])(*this, event, params);
    m_currentState = toIdx;
    (m_states.*m_onEnterCallbacks[toIdx])(*this, event, params);
    if (m_stateChangedHandler)
    {
        m_stateChangedHandler();
    }

    return true;
}

template<typename... FsmTransition>
void FSM<FsmTransition...>::reset()
{
    m_currentState = m_startState;
}

template<typename... FsmTransition>
void FSM<FsmTransition...>::update(float dt)
{
    (m_states.*m_UpdateEvent[m_currentState])(*this, dt);
}

template<typename... FsmTransition>
void FSM<FsmTransition...>::onEnter(IFSM<EventType>& fsm, const EventType& event, const Variant& params)
{
    m_currentState = m_startState;
    (m_states.*m_onEnterCallbacks[m_currentState])(*this, event, params);
    if (m_stateChangedHandler)
    {
        m_stateChangedHandler();
    }
}

template<typename... FsmTransition> [[maybe_unused]] void FSM<FsmTransition...>::onLeave(IFSM<EventType>& fsm, const EventType& event, const Variant& params)
{
    (m_states.*m_onLeaveCallbacks[m_currentState])(*this, event, params);
}

template<typename... FsmTransition>
void FSM<FsmTransition...>::onUpdate(IFSM<EventType>& fsm, float dt)
{
    (m_states.*m_UpdateEvent[m_currentState])(*this, dt);
}

template<typename... FsmTransition>
void FSM<FsmTransition...>::setStateChangedHdl(const std::function<void ()>& hdl)
{
    m_stateChangedHandler = hdl;
}

template<typename... FsmTransition>
template<typename State>
State& FSM<FsmTransition...>::state()
{
    return static_cast<State&>(m_states);
}

template<typename... FsmTransition>
template<typename State>
void FSM<FsmTransition...>::init(const EventType& event, const Variant& params)
{
    m_currentState = States::template getStateIdx<State>();
    m_startState = m_currentState;
    m_inited = true;
    (m_states.*m_onEnterCallbacks[m_currentState])(*this, event, params);
    if (m_stateChangedHandler)
    {
        m_stateChangedHandler();
    }
}

#undef FSM_VOID_HNDL
#undef FSM_BOOL_HNDL
#undef STATE_HNDL_IMPL
#undef DECLARE_FSM_HANDLER_TRAIT
#undef DECLVAL_WRAP
#undef COMMA_SEP