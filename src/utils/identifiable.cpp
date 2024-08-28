#include <lunar/utils/identifiable.hpp>
#include <atomic>

Identifiable::Identifiable(NativeType customId)
    : id(customId)
{

}

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
