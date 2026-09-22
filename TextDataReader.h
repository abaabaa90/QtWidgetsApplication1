#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cctype>
#include "mio.h"

#ifdef _OPENMP
#include <omp.h>
#endif

class TextDataReader {
private:
    mio::mmap_source mapped_file;
    std::vector<std::string> metadata;
    std::vector<double> data_vector;
    const char* text_data_ptr = nullptr;
    size_t text_data_size = 0;
    size_t data_count = 0;
    size_t metadata_size = 0;

public:
    bool open(const std::string& filename);
    void computeStatistics();

    template<typename Func>
    void processInBatches(size_t batch_size, Func&& processor);

    const std::vector<double>& getData() const;
    size_t getDataCount() const;
    void printBasicInfo() const;

private:
    void parseMetadata();
    void parseTextData();
};
