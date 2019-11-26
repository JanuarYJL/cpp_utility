#ifndef utility_include_utility_pattern_observer_h
#define utility_include_utility_pattern_observer_h

namespace diy
{
namespace utility
{
class ObserverImpl
{
public:
    virtual void Update() = 0;
};

class SubjectImpl
{
public:
    virtual void Attach(ObserverImpl *) = 0;

    virtual void Detach(ObserverImpl *) = 0;

    virtual void Notify() = 0;
};
}//namespace utility
}//namespace diy

#endif//!utility_include_utility_pattern_observer_h