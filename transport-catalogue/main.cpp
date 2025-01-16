#include <iostream>
#include <fstream>
#include "json_reader.h"
#include "transport_catalogue.h"

int main()
{
    TransportCatalogue catalogue;
    //std::ifstream input_file("Текст.txt");
    //std::ifstream input_file("Пример1.txt");

    InformationProcessing processor(catalogue, std::cin, std::cout);
    InformationProcessing processor(catalogue, input_file, std::cout);
    processor.Process();
    processor.ProcessRequest();
    return 0;
}
