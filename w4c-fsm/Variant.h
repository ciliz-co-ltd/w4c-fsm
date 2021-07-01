// (c) 2019-2021 Ciliz::W4
// Part of Ciliz W4 Game Creation SDK and Ciliz games codebase

#pragma once

#include <vector>
#include <unordered_map>
#include "variant"

namespace w4c
{
    enum class VariantType : uint8_t
    {
        NoneType,
        Int,
        Float,
        String,
        Array,
        Object
    };

    struct NoneType{
        NoneType(){}
        bool operator == (const NoneType& rVal) const { return true; };
        bool operator != (const NoneType& rVal) const { return false;};
    };

    struct VariantArray;
    struct VariantObject;

    using Variant = std::variant<
        NoneType,
        float,
        int,
        std::string,
        VariantArray,
        VariantObject
    >;

    using VecVariant = std::vector<Variant>;

    struct VariantArray : VecVariant
    {
        using VecVariant::VecVariant;
    };

    struct VariantObject : std::unordered_map<std::string, Variant>
    {};

    struct GetType
    {
        VariantType operator() (const NoneType&)       const { return VariantType::NoneType; }
        VariantType operator() (const float&)          const { return VariantType::Float; }
        VariantType operator() (const int&)            const { return VariantType::Int; }
        VariantType operator() (const std::string&)    const { return VariantType::String; }
        VariantType operator() (const VariantArray&)   const { return VariantType::Array; }
        VariantType operator() (const VariantObject&)  const { return VariantType::Object; }

        GetType() = default;
    };

    inline VariantType typeOfVariant(const Variant& v)
    {
        return std::visit(GetType(), v);
    }

    inline Variant getFromObjectOrNone(const VariantObject& obj, const std::string& key)
    {
        auto it = obj.find(key);
        if (it == obj.end())
        {
            return NoneType();
        }

        return it->second;
    }
}
