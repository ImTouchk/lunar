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

Identifiable::NativeType Identifiable::getId() const
{
    return id;
}

Identifiable::NativeType Identifiable::generateId()
{
    static std::atomic<Identifiable::NativeType> counter = 0;
    return ++counter;
}
