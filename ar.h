#pragma once
#include <string>
#include <filesystem>

void scatter(const std::filesystem::path& fn);
void gather(const std::filesystem::path& dn);
