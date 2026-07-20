#ifndef STAGE_STATUS_HPP_
#define STAGE_STATUS_HPP_

#include <cstdint>
#include <string>
#include <vector>

struct StageStatus{
    uint16_t count;
    std::vector<std::string> table;
};

#endif
