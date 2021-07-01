// (c) 2019-2021 Ciliz::W4
// Part of Ciliz W4 Game Creation SDK and Ciliz games codebase

#pragma once

#include <array>
#include <functional>
#include <type_traits>

#include "Variant.h"

#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/facilities/empty.hpp>

namespace w4c::fsm
{

#include "_details/FSM_header.inl"

template<typename EventType>
class IFSM
{
public:
    virtual ~IFSM() {}
    virtual bool processEvent(const EventType& event, const Variant& params = w4c::NoneType()) = 0;
    virtual void update(float dt) = 0;
    virtual void reset() = 0;
};

template<typename EventType>
struct FuncState
{
    void onEnter(IFSM<EventType>& fsm, const EventType& event, const w4c::Variant& params);
    void onLeave(IFSM<EventType>& fsm, const EventType& event, const w4c::Variant& params);
    void onUpdate(IFSM<EventType>& fsm, float dt);

    std::function<void (IFSM<EventType>&, const EventType&, const w4c::Variant&)> onEnterFunc;
    std::function<void (IFSM<EventType>&, const EventType&, const w4c::Variant&)> onLeaveFunc;
    std::function<void (IFSM<EventType>&, float dt)> onUpdateFunc;
};


DECLARE_FSM_HANDLER_TRAIT(onEnter, const EventType&, const Variant&)
DECLARE_FSM_HANDLER_TRAIT(onLeave, const EventType&, const Variant&)
DECLARE_FSM_HANDLER_TRAIT(processEvent, const EventType&, const Variant&)
DECLARE_FSM_HANDLER_TRAIT(onUpdate, float)

template<typename... Types>
struct States : public Types...
{
    template<typename... Args>
    States(Args... args);

    FSM_VOID_HNDL(onEnter)
    FSM_VOID_HNDL(onLeave)
    FSM_BOOL_HNDL(processEvent)
    FSM_VOID_HNDL(onUpdate)

    template<typename Type>
    static constexpr size_t getStateIdx();

    static constexpr size_t nStates();

    template<typename EventType, typename FSM>
    void initCallbacks(FSM& fsm);

private:

    template<typename EventType, typename FSM, typename Type, size_t idx>
    void initCallbacks_impl_Typed(FSM& fsm);

    template<typename EventType, typename FSM, size_t... Is>
    void initCallbacks_impl(FSM& fsm, Indices<Is...>);
};

template <typename T> struct PlainTypes {
    static_assert(std::is_same_v<T, std::decay_t<T>>);
    static_assert(!std::is_pointer_v<T>);
    using type = T;
};

template<auto event, typename FromState, typename ToState>
struct Transition
{
    template<typename States>
    static void initTransitions(std::unordered_map<size_t , std::unordered_map<decltype(event), size_t>>& collection);
    using type1 = FromState;
    using type2 = ToState;
    using eventType = decltype(event);
};

template<typename T1, typename... Transition>
struct Transitions
{
    template <typename States, typename EventType>
    static void initTransitions(std::unordered_map<size_t , std::unordered_map<EventType, size_t>>& collection);

    using types = typename detail::set<PlainTypes, typename T1::type1, typename T1::type2, typename Transition::type1..., typename Transition::type2...>::unique;
    using eventType = typename T1::eventType;
};



template<typename... FsmTransition>
 class FSM : public IFSM<typename fsm::Transitions<FsmTransition...>::eventType>
{
public:
    using Transitions = fsm::Transitions<FsmTransition...>;
    using States = typename detail::instantiate_with_set<fsm::States, typename Transitions::types>::type;
    using EventType = typename fsm::Transitions<FsmTransition...>::eventType;

    template<typename... Args>
    FSM(Args... args);

    bool processEvent(const EventType& event, const Variant& params = NoneType()) override;

    void reset() override;

    void update(float dt) override;
    void onEnter(IFSM<EventType>& fsm, const EventType& event, const Variant& params);
    void onLeave(IFSM<EventType>& fsm, const EventType& event, const Variant& params);
    void onUpdate(IFSM<EventType>& fsm, float dt);

    void setStateChangedHdl(const std::function<void ()>& hdl);

    template<typename State>
    void init(const EventType& event, const Variant& params = NoneType());

    template <typename State>
    State& state();

private:
    friend States;

    States m_states;

    std::unordered_map<size_t , std::unordered_map<EventType, size_t>> m_transitions;
    size_t m_startState = 0;
    size_t m_currentState = 0;
    bool m_inited = false;

    using CallbackPtr = void (States::*)(IFSM<EventType>&, const EventType& event, const Variant& params);
    using ProcessPtr = bool (States::*)(IFSM<EventType>&, const EventType& event, const Variant& params);
    using UpdatePtr = void (States::*)(IFSM<EventType>&, float dt);

    std::array<CallbackPtr, States::nStates()> m_onEnterCallbacks;
    std::array<CallbackPtr, States::nStates()> m_onLeaveCallbacks;
    std::array<ProcessPtr, States::nStates()> m_ProcessSubEvent;
    std::array<UpdatePtr, States::nStates()> m_UpdateEvent;

     std::function<void ()> m_stateChangedHandler;
};


#include "_details/FSM_footer.inl"
}
