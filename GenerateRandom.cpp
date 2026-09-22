#include <random>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <type_traits>
#include <stdexcept>

class RandomGenerator {
private:
    std::mt19937_64 engine;

    // 使用默认种子初始化引擎
    void init_engine(unsigned int seed = 0) {
        if (seed == 0) {
            seed = static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count());
        }
        engine.seed(seed);
    }
public:
    // 构造函数，可选种子
    RandomGenerator(unsigned int seed = 0) {
        init_engine(seed);
    }
    // 重新设置种子
    void reseed(unsigned int seed) {
        init_engine(seed);
    }

    // 生成整数随机数向量
    template<typename IntType = int>
    std::vector<IntType> generateIntegers(size_t count,
        IntType min = std::numeric_limits<IntType>::min(),
        IntType max = std::numeric_limits<IntType>::max()) {
        static_assert(std::is_integral<IntType>::value, "IntType must be an integral type");

        if (min > max) {
            throw std::invalid_argument("min must be less than or equal to max");
        }

        std::vector<IntType> result;
        result.reserve(count);

        if constexpr (std::is_same<IntType, bool>::value) {
            // 布尔类型特殊处理
            std::uniform_int_distribution<int> dist(0, 1);
            for (size_t i = 0; i < count; ++i) {
                result.push_back(static_cast<bool>(dist(engine)));
            }
        }
        else {
            std::uniform_int_distribution<IntType> dist(min, max);
            for (size_t i = 0; i < count; ++i) {
                result.push_back(dist(engine));
            }
        }

        return result;
    }
    // 生成浮点数随机数向量
    template<typename FloatType = double>
    std::vector<FloatType> generateFloats(size_t count,FloatType min = 0.0,FloatType max = 1.0) {
        static_assert(std::is_floating_point<FloatType>::value,
            "FloatType must be a floating point type");

        if (min > max) {
            throw std::invalid_argument("min must be less than or equal to max");
        }

        std::vector<FloatType> result;
        result.reserve(count);

        std::uniform_real_distribution<FloatType> dist(min, max);
        for (size_t i = 0; i < count; ++i) {
            result.push_back(dist(engine));
        }

        return result;
    }

    // 生成字符串随机数向量
    std::vector<std::string> generateStrings(size_t count,
        size_t length = 10,
        const std::string& charset =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789") {
        if (charset.empty()) {
            throw std::invalid_argument("charset must not be empty");
        }

        std::vector<std::string> result;
        result.reserve(count);

        std::uniform_int_distribution<size_t> char_dist(0, charset.size() - 1);

        for (size_t i = 0; i < count; ++i) {
            std::string str;
            str.reserve(length);

            for (size_t j = 0; j < length; ++j) {
                str.push_back(charset[char_dist(engine)]);
            }

            result.push_back(std::move(str));
        }

        return result;
    }

    // 生成随机字节向量
    std::vector<uint8_t> generateBytes(size_t count) {
        std::vector<uint8_t> result;
        result.reserve(count);

        std::uniform_int_distribution<uint16_t> dist(0, 255);
        for (size_t i = 0; i < count; ++i) {
            result.push_back(static_cast<uint8_t>(dist(engine)));
        }

        return result;
    }

    // 通用生成函数，根据类型自动选择合适的方法
    template<typename T>
    std::vector<T> generate(size_t count) {
        if constexpr (std::is_integral<T>::value) {
            return generateIntegers<T>(count);
        }
        else if constexpr (std::is_floating_point<T>::value) {
            return generateFloats<T>(count);
        }
        else if constexpr (std::is_same<T, std::string>::value) {
            return generateStrings(count);
        }
        else {
            static_assert(sizeof(T) == 0, "Unsupported type for RandomGenerator::generate");
        }
    }

    // 带范围的通用生成函数
    template<typename T>
    std::vector<T> generate(size_t count, T min, T max) {
        if constexpr (std::is_integral<T>::value) {
            return generateIntegers<T>(count, min, max);
        }
        else if constexpr (std::is_floating_point<T>::value) {
            return generateFloats<T>(count, min, max);
        }
        else {
            static_assert(sizeof(T) == 0,
                "Range parameters only supported for numeric types");
        }
    }

    // 生成不重复的随机数（适合小范围）
    template<typename IntType = int>
    std::vector<IntType> generateUnique(size_t count,
        IntType min = 0,
        IntType max = 100) {
        static_assert(std::is_integral<IntType>::value, "IntType must be an integral type");

        if (static_cast<size_t>(max - min + 1) < count) {
            throw std::invalid_argument("Range too small for unique values");
        }

        std::vector<IntType> pool;
        for (IntType i = min; i <= max; ++i) {
            pool.push_back(i);
        }

        std::shuffle(pool.begin(), pool.end(), engine);
        pool.resize(count);

        return pool;
    }

    // 生成正态分布的随机数
    template<typename FloatType = double>
    std::vector<FloatType> generateNormal(size_t count,
        FloatType mean = 0.0,
        FloatType stddev = 1.0) {
        static_assert(std::is_floating_point<FloatType>::value,
            "FloatType must be a floating point type");

        std::vector<FloatType> result;
        result.reserve(count);

        std::normal_distribution<FloatType> dist(mean, stddev);
        for (size_t i = 0; i < count; ++i) {
            result.push_back(dist(engine));
        }

        return result;
    }
};
