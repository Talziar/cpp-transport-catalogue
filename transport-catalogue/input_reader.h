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
            CommandDescription() = default;
            CommandDescription(CommandType type, const std::string &id, const std::string &description) : type_(type), id_(id), description_(description) {}

            explicit operator bool() const { return !(type_ == CommandType::NullType); }

            bool operator!() const { return !operator bool(); }

            CommandType type_ = CommandType::NullType; // Тип команды
            std::string id_;                           // Название маршрута или остановки
            std::string description_;                  // Параметры команды
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