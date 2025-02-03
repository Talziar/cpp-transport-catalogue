#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace transport_catalogue::stat_reader {

    namespace detail {
        enum class CommandType { NullType,
                                 Bus,
                                 Stop };

        struct CommandDescription {
            // Определяет, задана ли команда (поле command непустое)
            explicit operator bool() const { return !(type == CommandType::NullType); }

            bool operator!() const { return !operator bool(); }

            CommandType type = CommandType::NullType; // Название команды
            std::string_view description;             // Параметры команды
        };
    } // namespace detail

    void ParseAndPrintStat(
        const TransportCatalogue &transport_catalogue,
        std::string_view request, std::ostream &output);

} // namespace transport_catalogue::stat_reader
