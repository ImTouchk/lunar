#include <lunar/utils/identifiable.hpp>
#include <atomic>

Identifiable::Identifiable()
    : id(generateId())
{

}

int Identifiable::getId() const
{
    return id;
}

int Identifiable::generateId()
{
    static std::atomic<int> counter = 0;
    return ++counter;
}
