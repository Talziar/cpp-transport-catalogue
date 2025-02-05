#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    using namespace transport_catalogue;
    TransportCatalogue catalogue;

    input_reader::RunFromStream(catalogue, cin);
    stat_reader::RunFromStream(catalogue, cin, cout);
}