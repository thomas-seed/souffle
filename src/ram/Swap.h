/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2014, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file Swap.h
 *
 ***********************************************************************/

#pragma once

#include "ram/BinRelationStatement.h"
#include "ram/Relation.h"
#include "souffle/utility/MiscUtil.h"
#include "souffle/utility/StreamUtil.h"
#include <memory>
#include <ostream>
#include <string>
#include <utility>

namespace souffle {

/**
 * @class RamSwap
 * @brief Swap operation with respect to two relations
 *
 * Swaps the contents of the two relations
 *
 * For example:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * SWAP(A, B)
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
class RamSwap : public RamBinRelationStatement {
public:
    RamSwap(std::unique_ptr<RamRelationReference> f, std::unique_ptr<RamRelationReference> s)
            : RamBinRelationStatement(std::move(f), std::move(s)) {}

    RamSwap* clone() const override {
        return new RamSwap(souffle::clone(first), souffle::clone(second));
    }

protected:
    void print(std::ostream& os, int tabpos) const override {
        os << times(" ", tabpos);
        os << "SWAP (" << getFirstRelation().getName() << ", " << getSecondRelation().getName() << ")";
        os << std::endl;
    }
};

}  // end of namespace souffle
