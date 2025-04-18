#ifndef __OOC_ATTRIBUTES_HPP
#define __OOC_ATTRIBUTES_HPP

#include <geogram/api/defs.h>

#include "core.hpp"
#include <geogram/mesh/mesh.h>

namespace incremental_meshing::attributes
{
    enum class VertexDescriptorFlags : uint8_t
    {
        INTERFACE = 1 << 0, // marks vertex as interface vertex for the currently processed plane
        DISCARD   = 1 << 1  // marks vertex as discardable in the decimation stage
    };

    // needs to be defined for the compiler not to complain
    inline std::istream& operator>>(std::istream& is, VertexDescriptorFlags& flags)
    {
        return is;
    }

    // needs to be defined for the compiler not to complain
    inline std::ostream& operator<<(std::ostream& os, const VertexDescriptorFlags& flags)
    {
        return os;
    }

    // must be an enum class, geogram doesn't seem to like "normal" enums
    // https://stackoverflow.com/questions/32578638/how-to-use-c11-enum-class-for-flags
    inline VertexDescriptorFlags operator|(const VertexDescriptorFlags lhs, const VertexDescriptorFlags rhs)
    {
        return static_cast<VertexDescriptorFlags>(
            static_cast<std::underlying_type<VertexDescriptorFlags>::type>(lhs) |
            static_cast<std::underlying_type<VertexDescriptorFlags>::type>(rhs)
        );
    }

    inline VertexDescriptorFlags operator&(VertexDescriptorFlags lhs, VertexDescriptorFlags rhs)
    {
        return static_cast<VertexDescriptorFlags>(
            static_cast<std::underlying_type<VertexDescriptorFlags>::type>(lhs) &
            static_cast<std::underlying_type<VertexDescriptorFlags>::type>(rhs)
        );
    }

    inline bool operator==(const VertexDescriptorFlags lhs, const VertexDescriptorFlags rhs)
    {
        return (static_cast<std::underlying_type<VertexDescriptorFlags>::type>(rhs) ==
            static_cast<std::underlying_type<VertexDescriptorFlags>::type>(lhs) &
            static_cast<std::underlying_type<VertexDescriptorFlags>::type>(rhs)
        );
    }

    inline void initialize()
    {
        geogram::geo_register_attribute_type<VertexDescriptorFlags>("VertexDescriptorFlags");
    }
}
#endif // __OOC_ATTRIBUTES_HPP
