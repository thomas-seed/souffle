/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2014, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file WriteStreamCSV.h
 *
 ***********************************************************************/

#pragma once

#include "CompiledRecord.h"
#include "IODirectives.h"
#include "ParallelUtils.h"
#include "RecordTable.h"
#include "SymbolTable.h"
#include "WriteStream.h"
#ifdef USE_LIBZ
#include "gzfstream.h"
#endif

#include <cassert>
#include <fstream>
#include <memory>
#include <ostream>
#include <string>

namespace souffle {

class WriteStreamCSV {
protected:
    virtual std::string getDelimiter(const IODirectives& ioDirectives) const {
        if (ioDirectives.has("delimiter")) {
            return ioDirectives.get("delimiter");
        }
        return "\t";
    }
};

class WriteFileCSV : public WriteStreamCSV, public WriteStream {
public:
    WriteFileCSV(const std::vector<TypeId>& typeMask, const SymbolTable& symbolTable,
            const RecordTable& recordTable, const TypeTable& typeTable, const IODirectives& ioDirectives,
            const bool provenance = false)
            : WriteStream(typeMask, symbolTable, recordTable, typeTable, provenance),
              delimiter(getDelimiter(ioDirectives)),
              file(ioDirectives.getFileName(), std::ios::out | std::ios::binary) {
        if (ioDirectives.has("headers") && ioDirectives.get("headers") == "true") {
            file << ioDirectives.get("attributeNames") << std::endl;
        }
    }

    ~WriteFileCSV() override = default;

protected:
    const std::string delimiter;
    std::ofstream file;

    void writeNullary() override {
        file << "()\n";
    }

    void writeNextTuple(const RamDomain* tuple) override {
        for (size_t col = 0; col < arity; ++col) {
            writeValue(file, tuple[col], typeMask.at(col));
            if (col != arity - 1) {
                file << delimiter;
            }
        }
        file << "\n";
    }
};

#ifdef USE_LIBZ
class WriteGZipFileCSV : public WriteStreamCSV, public WriteStream {
public:
    WriteGZipFileCSV(const std::vector<TypeId>& typeMask, const SymbolTable& symbolTable,
            const RecordTable& recordTable, const TypeTable& typeTable, const IODirectives& ioDirectives,
            const bool provenance = false)
            : WriteStream(typeMask, symbolTable, recordTable, typeTable, provenance),
              delimiter(getDelimiter(ioDirectives)),
              file(ioDirectives.getFileName(), std::ios::out | std::ios::binary) {
        if (ioDirectives.has("headers") && ioDirectives.get("headers") == "true") {
            file << ioDirectives.get("attributeNames") << std::endl;
        }
    }

    ~WriteGZipFileCSV() override = default;

protected:
    void writeNullary() override {
        file << "()\n";
    }

    void writeNextTuple(const RamDomain* tuple) override {
        for (size_t col = 0; col < arity; ++col) {
            writeValue(file, tuple[col], typeMask.at(col));
            if (col != arity - 1) {
                file << delimiter;
            }
        }
        file << "\n";
    }

    const std::string delimiter;
    gzfstream::ogzfstream file;
};
#endif

class WriteCoutCSV : public WriteStreamCSV, public WriteStream {
public:
    WriteCoutCSV(const std::vector<TypeId>& typeMask, const SymbolTable& symbolTable,
            const RecordTable& recordTable, const TypeTable& typeTable, const IODirectives& ioDirectives,
            const bool provenance = false)
            : WriteStream(typeMask, symbolTable, recordTable, typeTable, provenance),
              delimiter(getDelimiter(ioDirectives)) {
        std::cout << "---------------\n" << ioDirectives.getRelationName();
        if (ioDirectives.has("headers") && ioDirectives.get("headers") == "true") {
            std::cout << "\n" << ioDirectives.get("attributeNames");
        }
        std::cout << "\n===============\n";
    }

    ~WriteCoutCSV() override {
        std::cout << "===============\n";
    }

protected:
    void writeNullary() override {
        std::cout << "()\n";
    }

    void writeNextTuple(const RamDomain* tuple) override {
        for (size_t col = 0; col < arity; ++col) {
            writeValue(std::cout, tuple[col], typeMask.at(col));
            if (col != arity - 1) {
                std::cout << delimiter;
            }
        }
        std::cout << "\n";
    }

    const std::string delimiter;
};

class WriteCoutPrintSize : public WriteStream {
public:
    WriteCoutPrintSize(const IODirectives& ioDirectives)
            : WriteStream({}, {}, {}, {}, false, true), lease(souffle::getOutputLock().acquire()) {
        std::cout << ioDirectives.getRelationName() << "\t";
    }

    ~WriteCoutPrintSize() override = default;

protected:
    void writeNullary() override {
        assert(false && "attempting to iterate over a print size operation");
    }

    void writeNextTuple(const RamDomain* /* tuple */) override {
        assert(false && "attempting to iterate over a print size operation");
    }

    void writeSize(std::size_t size) override {
        std::cout << size << "\n";
    }

    Lock::Lease lease;
};

class WriteFileCSVFactory : public WriteStreamFactory {
public:
    std::unique_ptr<WriteStream> getWriter(const std::vector<TypeId>& typeMask,
            const SymbolTable& symbolTable, const RecordTable& recordTable, const TypeTable& typeTable,
            const IODirectives& ioDirectives, const bool provenance) override {
#ifdef USE_LIBZ
        if (ioDirectives.has("compress")) {
            return std::make_unique<WriteGZipFileCSV>(
                    typeMask, symbolTable, recordTable, typeTable, ioDirectives, provenance);
        }
#endif
        return std::make_unique<WriteFileCSV>(
                typeMask, symbolTable, recordTable, typeTable, ioDirectives, provenance);
    }
    const std::string& getName() const override {
        static const std::string name = "file";
        return name;
    }
    ~WriteFileCSVFactory() override = default;
};

class WriteCoutCSVFactory : public WriteStreamFactory {
public:
    std::unique_ptr<WriteStream> getWriter(const std::vector<TypeId>& typeMask,
            const SymbolTable& symbolTable, const RecordTable& recordTable, const TypeTable& typeTable,
            const IODirectives& ioDirectives, const bool provenance) override {
        return std::make_unique<WriteCoutCSV>(
                typeMask, symbolTable, recordTable, typeTable, ioDirectives, provenance);
    }
    const std::string& getName() const override {
        static const std::string name = "stdout";
        return name;
    }
    ~WriteCoutCSVFactory() override = default;
};

// TODO TODO TODO: std::vector<int> should be changed to std::vector<kind>...

class WriteCoutPrintSizeFactory : public WriteStreamFactory {
public:
    std::unique_ptr<WriteStream> getWriter(const std::vector<int>& /* typeMask */,
            const SymbolTable& /* symbolTable */, const RecordTable& /* recordTable */,
            const TypeTable& /* typeTable */, const IODirectives& ioDirectives,
            const bool /* provenance */) override {
        return std::make_unique<WriteCoutPrintSize>(ioDirectives);
    }
    const std::string& getName() const override {
        static const std::string name = "stdoutprintsize";
        return name;
    }
    ~WriteCoutPrintSizeFactory() override = default;
};

} /* namespace souffle */
