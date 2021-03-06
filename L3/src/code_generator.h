#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <vector>

#include "inst_selection.h"

namespace L3 {

    void generateCode(Program & p, std::vector<std::vector<InstSelectForest * >> & codeGenerator);
}