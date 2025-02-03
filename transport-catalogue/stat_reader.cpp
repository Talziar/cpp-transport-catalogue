#include "stat_reader.h"

#include <algorithm>
#include <iomanip>

using namespace std;

namespace transport_catalogue::stat_reader {

    namespace detail {
        string_view Trim(string_view string) {
            const auto start = string.find_first_not_of(' ');
            if (start == string.npos) {
                return {};
            }
            return string.substr(start, string.find_last_not_of(' ') + 1 - start);
        }

        CommandDescription ParseRequest(string_view request) {
            string_view trimmed_request = Trim(request);
            auto space_pos = trimmed_request.find(' ');
            if (space_pos == string::npos) {
                return {};
            }

            auto not_space = trimmed_request.find_first_not_of(' ', space_pos);
            if (not_space == string::npos) {
                return {};
            }

            const unordered_map<string, CommandType> text_to_command{
                {"Bus", CommandType::Bus}, {"Stop", CommandType::Stop}};

            if (!text_to_command.contains(string(trimmed_request.substr(0, space_pos)))) {
                return {};
            }

            return CommandDescription{
                text_to_command.at(string(trimmed_request.substr(0, space_pos))),
                trimmed_request.substr(not_space)};
        }

        void StopRequestPrint(const TransportCatalogue &transport_catalogue, string_view parsed_request, ostream &output) {
            optional<vector<Bus *>> stop_buses = transport_catalogue.GetBusesByStop(parsed_request);
            if (!stop_buses.has_value()) {
                output << "Stop "s << parsed_request << ": not found"s << endl;
            } else if (stop_buses.value().empty()) {
                output << "Stop "s << parsed_request << ": no buses"s << endl;
            } else {
                sort(stop_buses.value().begin(), stop_buses.value().end(), [](const Bus *lhs, const Bus *rhs) {
                    return lhs->first < rhs->first;
                });
                output << "Stop "s << parsed_request << ": buses"s;
                for (auto el : stop_buses.value()) {
                    output << ' ' << el->first;
                }
                output << endl;
            }
        }

        void BusRequestPrint(const TransportCatalogue &transport_catalogue, string_view parsed_request, ostream &output) {
            Bus *requested_bus = transport_catalogue.FindBus(parsed_request);
            if (!requested_bus) {
                output << "Bus "s << parsed_request << ": not found"s << endl;
            } else {
                BusInfo requested_bus_info = transport_catalogue.GetBusInfo(parsed_request);
                output << "Bus "s << parsed_request << setprecision(6) << ": "s << requested_bus_info.stop_count_ << " stops on route, "s
                       << requested_bus_info.unique_stop_count_ << " unique stops, "s << requested_bus_info.route_length_ << " route length" << endl;
            }
        }
    } // namespace detail

    void ParseAndPrintStat(const TransportCatalogue &transport_catalogue, string_view request, ostream &output) {
        using namespace detail;

        const CommandDescription parsed_request = ParseRequest(request);
        switch (parsed_request.type) {
        case CommandType::Bus: {
            BusRequestPrint(transport_catalogue, parsed_request.description, output);
            break;
        }
        case CommandType::Stop: {
            StopRequestPrint(transport_catalogue, parsed_request.description, output);
        }
        case CommandType::NullType: {
        }
        }
    }

} // namespace transport_catalogue::stat_reader
