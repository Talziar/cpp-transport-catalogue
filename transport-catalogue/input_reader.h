#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

namespace transport_catalogue::input_reader {

    namespace detail {
        enum class CommandType { NullType,
                                 Stop,
                                 Bus };

        struct CommandDescription {
            explicit operator bool() const { return !(type == CommandType::NullType); }

            bool operator!() const { return !operator bool(); }

            CommandType type = CommandType::NullType; // Тип команды
            std::string id;                           // Название маршрута или остановки
            std::string description;                  // Параметры команды
        };
    } // namespace detail

    class InputReader {
    public:
        void ParseLine(std::string_view line);

        void ApplyCommands(TransportCatalogue &catalogue) const;

    private:
        std::vector<detail::CommandDescription> commands_;
    };

    void RunFromStream(TransportCatalogue &catalogue, std::istream &in_stream);

} // namespace transport_catalogue::input_reader