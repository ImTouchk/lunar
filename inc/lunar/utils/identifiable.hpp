#pragma once

class Identifiable
{
public:
    using NativeType = int;

    Identifiable();

    virtual NativeType getId() const;

protected:
    NativeType generateId();

    NativeType id;
};


