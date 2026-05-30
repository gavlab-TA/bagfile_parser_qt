#pragma once
#include <string>
#include <vector>
#include "bagfile_parser_qt/mat_writer.hpp"

void writeCsv(const std::string& filepath, const CollectedData& data, const std::vector<double>& timestamps);
