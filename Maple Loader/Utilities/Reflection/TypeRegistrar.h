#pragma once

template <typename Type>
struct TypeRegistrar
{
    bool IsRegistered = false;

    TypeRegistrar()
    {
        Type::Register();

        IsRegistered = true;
    }
};