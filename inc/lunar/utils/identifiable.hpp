#pragma once

class Identifiable
{
public:
    Identifiable();

    virtual int getId() const;

protected:
    int generateId();

    int id;
};


