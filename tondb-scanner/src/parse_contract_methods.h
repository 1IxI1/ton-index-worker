#pragma once
#include <map>
#include "td/utils/Status.h"
#include "vm/cells/Cell.h"
#include "common/refcnt.hpp"
#include "tokens.h"

td::Result<std::vector<td::RefInt256>> parse_contract_methods(td::Ref<vm::Cell> code_cell);
