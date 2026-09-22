#include "TextDataReader.h"
#include <functional>

bool TextDataReader::open(const std::string& filename) {
    try {
        auto start_time = std::chrono::high_resolution_clock::now();

        mapped_file = mio::make_mmap_source(filename);

        parseMetadata();

        text_data_ptr = mapped_file.data() + metadata_size;
        text_data_size = mapped_file.size() - metadata_size;

        auto mid_time = std::chrono::high_resolution_clock::now();
        std::cout << "File mapped in "
            << std::chrono::duration_cast<std::chrono::milliseconds>(mid_time - start_time).count()
            << " ms" << std::endl;

        parseTextData();

        auto end_time = std::chrono::high_resolution_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        std::cout << "\n=== Parsing Statistics ===" << std::endl;
        std::cout << "Total time: " << total_duration.count() << " ms" << std::endl;
        std::cout << "Data points: " << data_count << std::endl;
        std::cout << "Throughput: " << (data_count / 1000000.0) / (total_duration.count() / 1000.0)
            << " million numbers/second" << std::endl;

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error opening file: " << e.what() << std::endl;
        return false;
    }
}

void TextDataReader::parseMetadata() {
    const char* file_start = mapped_file.data();
    size_t file_size = mapped_file.size();

    metadata.clear();
    metadata_size = 0;

    int lines_found = 0;
    size_t line_start = 0;

    for (size_t i = 0; i < file_size && lines_found < 5; ++i) {
        if (file_start[i] == '\n') {
            std::string line(file_start + line_start, i - line_start);
            metadata.push_back(line);
            lines_found++;
            line_start = i + 1;
        }
    }

    metadata_size = line_start;
    std::cout << "Found " << metadata.size() << " metadata lines" << std::endl;
}

void TextDataReader::parseTextData() {
    std::cout << "\n=== Parsing Text Data ===" << std::endl;
    std::cout << "Text data size: " << text_data_size << " bytes" << std::endl;

    auto start_time = std::chrono::high_resolution_clock::now();

    size_t estimated_numbers = text_data_size / 8;
    data_vector.reserve(estimated_numbers);

    const char* ptr = text_data_ptr;
    const char* end = text_data_ptr + text_data_size;

    while (ptr < end) {
        while (ptr < end && std::isspace(*ptr)) ptr++;
        if (ptr >= end) break;

        char* next_ptr = nullptr;
        double value = std::strtod(ptr, &next_ptr);

        if (next_ptr > ptr) {
            data_vector.push_back(value);
            ptr = next_ptr;
        }
        else {
            ptr++;
        }

        if (data_vector.size() % 1000000 == 0) {
            std::cout << "\rParsed " << data_vector.size() / 1000000 << " million numbers..." << std::flush;
        }
    }

    data_count = data_vector.size();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "\rParsed " << data_count << " numbers in " << duration.count() << " ms" << std::endl;

    if (data_count > 0) {
        std::cout << "\nFirst 10 data points for verification:" << std::endl;
        for (size_t i = 0; i < std::min((size_t)10, data_count); ++i) {
            std::cout << "  [" << i << "] = " << std::fixed << std::setprecision(6)
                << data_vector[i] << std::endl;
        }
    }
}

void TextDataReader::computeStatistics() {
    if (data_vector.empty()) {
        std::cout << "No data loaded." << std::endl;
        return;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    double total_sum = 0.0;
    double min_val = data_vector[0];
    double max_val = data_vector[0];

    int count = static_cast<int>(data_count);

#ifdef _OPENMP
#pragma omp parallel
    {
        double local_sum = 0.0;
        double local_min = data_vector[0];
        double local_max = data_vector[0];

#pragma omp for nowait
        for (int i = 0; i < count; ++i) {
            double val = data_vector[i];
            local_sum += val;
            if (val < local_min) local_min = val;
            if (val > local_max) local_max = val;
        }

#pragma omp atomic
        total_sum += local_sum;

#pragma omp critical
        {
            if (local_min < min_val) min_val = local_min;
            if (local_max > max_val) max_val = local_max;
        }
    }
#else
    for (int i = 0; i < count; ++i) {
        double val = data_vector[i];
        total_sum += val;
        if (val < min_val) min_val = val;
        if (val > max_val) max_val = val;
    }
#endif

    double average = total_sum / data_count;

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "\n=== Complete Statistics ===" << std::endl;
    std::cout << "Computation time: " << duration.count() << " ms" << std::endl;
    std::cout << "Total sum: " << std::fixed << std::setprecision(6) << total_sum << std::endl;
    std::cout << "Average: " << std::fixed << std::setprecision(6) << average << std::endl;
    std::cout << "Min: " << std::fixed << std::setprecision(6) << min_val << std::endl;
    std::cout << "Max: " << std::fixed << std::setprecision(6) << max_val << std::endl;
    std::cout << "Data count: " << data_count << std::endl;
}

template<typename Func>
void TextDataReader::processInBatches(size_t batch_size, Func&& processor) {
    if (data_vector.empty()) {
        std::cerr << "Error: No data loaded!" << std::endl;
        return;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    size_t num_batches = (data_count + batch_size - 1) / batch_size;
    std::vector<double> batch_sums(num_batches, 0.0);

    std::cout << "\nProcessing " << data_count << " data points in "
        << num_batches << " batches..." << std::endl;

    int num_batches_int = static_cast<int>(num_batches);

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
    for (int batch = 0; batch < num_batches_int; ++batch) {
        size_t start = static_cast<size_t>(batch) * batch_size;
        size_t end = std::min(start + batch_size, data_count);

        double batch_sum = 0.0;
        for (size_t i = start; i < end; ++i) {
            batch_sum += data_vector[i];
        }

        batch_sums[batch] = batch_sum;

        if (batch % 100 == 0) {
#ifdef _OPENMP
#pragma omp critical
#endif
            {
                double progress = (batch * 100.0) / num_batches_int;
                std::cout << "\rProgress: " << std::fixed << std::setprecision(1)
                    << progress << "%" << std::flush;
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "\rProgress: 100.0%" << std::endl;

    std::cout << "\n=== Batch Processing Results ===" << std::endl;
    std::cout << "Total time: " << duration.count() << " ms" << std::endl;
    std::cout << "Time per million points: "
        << (duration.count() * 1000000.0 / data_count) << " \xC2\xB5s" << std::endl;
    std::cout << "Throughput: " << (data_count / 1000000.0) / (duration.count() / 1000.0)
        << " million numbers/second" << std::endl;

    for (size_t batch = 0; batch < num_batches; ++batch) {
        processor(batch_sums[batch], batch);
    }
}

template void TextDataReader::processInBatches(size_t, std::function<void(double, size_t)>&&);

const std::vector<double>& TextDataReader::getData() const {
    return data_vector;
}

size_t TextDataReader::getDataCount() const {
    return data_count;
}

void TextDataReader::printBasicInfo() const {
    std::cout << "\n=== Basic Information ===" << std::endl;
    std::cout << "Data points: " << data_count << std::endl;
    std::cout << "Data memory: " << (data_count * sizeof(double) / (1024.0 * 1024.0 * 1024.0))
        << " GB" << std::endl;

    if (data_count > 0) {
        std::cout << "First value: " << data_vector[0] << std::endl;
        std::cout << "Last value: " << data_vector[data_count - 1] << std::endl;
    }
}
