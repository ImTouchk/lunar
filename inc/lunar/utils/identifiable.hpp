#pragma once

class Identifiable
{
public:
    using NativeType = int;

    Identifiable();
    Identifiable(NativeType customId);

    virtual NativeType getId() const;

protected:
    NativeType generateId();

    NativeType id;
};


