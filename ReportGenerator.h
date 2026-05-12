#pragma once
#include "Network.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

class ReportGenerator {
public:
    static void generate(const std::string& filename,
                         const std::vector<RoundStats>& multiHopStats,
                         const std::vector<RoundStats>& leachStats,
                         int totalNodes);
};
