#pragma once

#include "config.hpp"
#include <string>

void interpret(const std::string& src_code, std::istream& in, std::ostream& out, std::ostream& err,
               const Config& config);
