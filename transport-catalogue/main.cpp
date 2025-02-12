#include <iostream>
#include <string>

#include "json_reader.h"

using namespace std;

int main() {
    using namespace transport_catalogue;
    TransportCatalogue catalogue;

    json_reader::Run(catalogue, cin, cout);
}