#pragma once

#include <iosfwd>
#include <string_view>
#include "transport_catalogue.h"
#include"input_reader.h"

//обработка запросов к базе и вывод данных
void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& output);
