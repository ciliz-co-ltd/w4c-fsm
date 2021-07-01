
#include <iostream>
#include "w4c-fsm//FSM.h"

enum class FSM_Events
{
    Init,
    ToState1,
    ToState2
};

struct State1
{
    void onUpdate(w4c::fsm::IFSM<FSM_Events>& fsm, float dt)
    {
        std::cout << "State1: onUpdate" << std::endl;
    }

    void onEnter(w4c::fsm::IFSM<FSM_Events>& fsm, const FSM_Events& event, const w4c::Variant& params)
    {
        std::cout << "State1: onEnter " << std::get<int>(params) << std::endl;
    }

    void onLeave(w4c::fsm::IFSM<FSM_Events>& fsm, const FSM_Events& event, const w4c::Variant& params)
    {
        std::cout << "State1: onLeave " << std::get<int>(params) << std::endl;
    }
};

struct State2
{
    void onUpdate(w4c::fsm::IFSM<FSM_Events>& fsm, float dt)
    {
        std::cout << "State2: onUpdate" << std::endl;
    }

    void onEnter(w4c::fsm::IFSM<FSM_Events>& fsm, const FSM_Events& event, const w4c::Variant& params)
    {
        std::cout << "State2: onEnter " << std::get<int>(params) << std::endl;
    }

    void onLeave(w4c::fsm::IFSM<FSM_Events>& fsm, const FSM_Events& event, const w4c::Variant& params)
    {
        std::cout << "State2: onLeave " << std::get<int>(params) << std::endl;
    }
};

using TestFSM = w4c::fsm::FSM<
    w4c::fsm::Transition<FSM_Events::ToState1,      State2,         State1>,
    w4c::fsm::Transition<FSM_Events::ToState2,      State1,         State2>
>;

int main()
{
    TestFSM fsm;
    fsm.init<State1>(FSM_Events::Init, 0);
    fsm.update(0.1f);
    fsm.processEvent(FSM_Events::ToState2, 10);
    fsm.update(0.1f);
    fsm.processEvent(FSM_Events::ToState1, 20);
    fsm.update(0.1f);
    return 0;
}
