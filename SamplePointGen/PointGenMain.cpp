/// System headers
#include <algorithm> /// std::find
#include <cassert>   /// assert macro
#include <chrono>    /// std::chrono::high_resolution_clock
#include <cmath>     /// std::acos, std::sqrt
#include <cstdio>    /// std::snprintf
#include <fstream>   /// std::fstream
#include <iomanip>   /// std::uppercase
#include <iostream>  /// std::cout
#include <limits>    /// std::numeric_limits
#include <random>    /// std::mt19937, std::uniform_int_distribution
#include <stdint.h>  /// uint16_t
#include <vector>
/// Lib headers
#include "Common/CommonDefines.hpp"
#include "DataType/Float2D.hpp"
#include "Math/MathDefines.hpp" /// TWO_PI


#define USE_RANDOM_SEED 1


/// Hammersley四个Sample
/// NOTE: 使用4x4的格子
///
///     ^ Y
///     |
/// 4/4 +----+----+----+----+
///     |    |    |    |    |
/// 3/4 +----+----+----*----+
///     |    |    |    |    |
/// 2/4 +----*----+----+----+
///     |    |    |    |    |
/// 1/4 +----+----*----+----+
///     |    |    |    |    |
/// 0   *----+----+----+----+--> X
///     0    1/4  2/4  3/4  4/4
///
///                    i
/// Pᵢ = (xᵢ, yᵢ) = ( ───, Φ₂(i) )
///                    n
///
///  i    i/n      Phi(i)
/// ---------------------
///  0    0.00        0.00
///  1    0.25        0.50   <-- 0000 0001.0 --> 0000 0000.1000 := 1*2^-1
///  2    0.50        0.25   <-- 0000 0010.0 --> 0000 0000.0100 := 1*2^-2
///  3    0.75        0.75   <-- 0000 0011.0 --> 0000 0000.1100 := 1*2^-1 + 1*2^-2
/// n: 为单一SET中Sample的总数
///
class PointGenerator
{
public:
    PointGenerator (
       const char * const output_file_name)
    :
        m_is_ready(false)
    {
        if (output_file_name && output_file_name[0])
        {
            char file_name[1024];

            std::snprintf(
                file_name, sizeof(file_name), "%sDefines.hpp", output_file_name);
            m_define_file_stream.open(
                file_name, std::fstream::out | std::fstream::trunc);
            if (m_define_file_stream.is_open())
            {
                std::cout << "Sample define file: " << file_name << std::endl;
            }

            std::snprintf(
                file_name, sizeof(file_name), "%sData.hpp", output_file_name);
            m_data_file_stream.open(
                file_name, std::fstream::out | std::fstream::trunc);
            if (m_data_file_stream.is_open())
            {
                std::cout << "Sample data file: " << file_name << std::endl;
            }

            m_is_ready =
                m_data_file_stream.is_open() && m_define_file_stream.is_open();
            if(m_is_ready)
            {
                std::cout << std::endl;
            }
        }
        m_sample_array.reserve(SAMPLE_TOTAL_COUNT);
    }

    ~PointGenerator ()
    {
        m_data_file_stream.close();
        m_define_file_stream.close();
    }

    bool
    is_ready () const
    {
        return m_is_ready;
    }

    /// 生成Unit Square上的所有Sample
    void
    generate_unit_square_samples ()
    {
#if (USE_RANDOM_SEED == 1)
        using namespace std::chrono;
        static const uint32_t HAMMERSLEY_SEED =
            (uint32_t)high_resolution_clock::now().time_since_epoch().count();
#else
        static constexpr uint32_t HAMMERSLEY_SEED = FOUR_CC_32('H', 'M', 'L', 'Y');
#endif // #if (USE_RANDOM_SEED == 1)
        m_hammersley_seed = HAMMERSLEY_SEED;

        /// 创建Randome Number Generator以及Distribution
        std::mt19937 random_num_gen(m_hammersley_seed);
        /// 使用16位随机Scramble: [0, 2^16): 因为#Sample最多为2^16
        std::uniform_int_distribution<uint32_t> random_num_dist(
            0, std::numeric_limits<uint16_t>::max());

        std::cout << "Generating Hammersley Sample Points..." << std::endl;
        std::cout << "[Seed]: "        << m_hammersley_seed   << std::endl;
        std::cout << "[#Sample]: "     << SAMPLE_TOTAL_COUNT  << ", ";
        std::cout << "[#Set]: "        << SAMPLE_SET_COUNT    << ", ";
        std::cout << "[#Sample/Set]: " << SET_SAMPLE_COUNT    << std::endl;

        /// 清除旧数据
        m_sample_array.clear();
        m_scramble_array.clear();

        /// 清除所有Mapping数据
        m_spherical_mapping.clear();

        /// 生成Sample Point
        /// 遍历所有SET
        for (uint32_t set_idx = 0; set_idx < SAMPLE_SET_COUNT; ++set_idx)
        {
            /// 计算每个SET使用的唯一Scramble(对整数i按位进行随机XOR)
            uint32_t scramble;
            do
            {
                scramble = random_num_dist(random_num_gen);
            }
            while (std::find(m_scramble_array.begin(), m_scramble_array.end(), scramble)
                   != m_scramble_array.end());
            std::cout << "[Scramble|Set(" << set_idx << ")]: "
                      << "0x" << std::hex << std::uppercase << std::setfill('0')
                      << std::setw(8) << scramble << std::dec << std::endl;
            m_scramble_array.push_back(scramble);

            /// 遍历每组中的所有Sample
            for (uint32_t sample_idx = 0;
                 sample_idx < SET_SAMPLE_COUNT; ++sample_idx)
            {
                ///                    i
                /// Pᵢ = (xᵢ, yᵢ) = ( ───, Φ₂(i XOR scramble) )
                ///                    n
                const float xi = (float)sample_idx / (float)SET_SAMPLE_COUNT;
                const float yi = Phi(sample_idx ^ scramble);
                m_sample_array.push_back(float_2{ xi, yi });
            }
        }

        /// 生成调试用的CSV Sample Point文件
        char buffer[128];
        for (uint32_t set_idx = 0; set_idx < SAMPLE_SET_COUNT; ++set_idx)
        {
            std::fstream csv_file_stream;
            std::snprintf(buffer, sizeof(buffer),
                          "./Hammersley_Sample_Point_Array_SET_%u.csv", set_idx);
            csv_file_stream.open(buffer, std::fstream::out | std::fstream::trunc);
            if (csv_file_stream.is_open())
            {
                ///输出CSV表头
                csv_file_stream << "x, y" << std::endl;

                for (uint32_t sample_idx = 0;
                     sample_idx < SET_SAMPLE_COUNT; ++sample_idx)
                {
                    /// Sample的全局索引
                    const uint32_t global_sample_idx =
                        set_idx * SET_SAMPLE_COUNT + sample_idx;
                    std::snprintf(buffer, sizeof(buffer), "%.8f, %.8f",
                                  m_sample_array[global_sample_idx].x,
                                  m_sample_array[global_sample_idx].y);
                    csv_file_stream << buffer << std::endl;
                }
                csv_file_stream.close();
            }
            else
            {
                break;
            }
        }
        std::cout << "Generation Done." << std::endl;
    }

    /// 将Unit Square上的Sample映射到Hemi-Sphere上
    void map_onto_hemisphere ()
    {
        assert(m_sample_array.size() == SAMPLE_TOTAL_COUNT);

        std::cout << "Mappinging Sample Points onto Hemi-Sphere..." << std::endl;

        /// 清除Mapping数据
        m_spherical_mapping.clear();

        for (uint32_t set_idx = 0; set_idx < SAMPLE_SET_COUNT; ++set_idx)
        {
            for (uint32_t sample_idx = 0; sample_idx < SET_SAMPLE_COUNT; ++sample_idx)
            {
                /// Sample的全局索引
                const uint32_t global_sample_idx =
                    set_idx * SET_SAMPLE_COUNT + sample_idx;
                /// 提取Unit Square上的Sample: (ξ1, ξ2)
                const float_2 sample_point = m_sample_array[global_sample_idx];
                /// Mapping
                /// φ = 2π * ξ1
                /// θ = arcos[√(1 − ξ2)]
                const float phi   = TWO_PI * sample_point.x;
                const float theta =
                    std::acos(std::sqrt(1.0f - sample_point.y));
                /// 存储顺序: (φ, θ)
                m_spherical_mapping.push_back(float_2{ phi, theta });
            }
        }

        /// 生成GeoGebra 3D调试用的Sample Point文件(TXT文件):
        /// L =
        /// {
        ///     (x0, y0, z0),
        ///     (x1, y1, z1),
        ///         ...
        ///     (xn, yn, zn)
        /// }
        ///
        char buffer[128];
        for (uint32_t set_idx = 0; set_idx < SAMPLE_SET_COUNT; ++set_idx)
        {
            std::fstream pnt_file_stream;
            std::snprintf(buffer, sizeof(buffer),
                          "./Hammersley_Spherical_Sample_Point_Array_SET_%u.txt",
                          set_idx);
            pnt_file_stream.open(buffer, std::fstream::out | std::fstream::trunc);
            if (pnt_file_stream.is_open())
            {
                pnt_file_stream << "L =" << std::endl;
                pnt_file_stream << "{"   << std::endl;

                for (uint32_t sample_idx = 0;
                     sample_idx < SET_SAMPLE_COUNT; ++sample_idx)
                {
                    /// Sample的全局索引
                    const uint32_t global_idx = set_idx * SET_SAMPLE_COUNT + sample_idx;
                    /// (φ, θ)
                    const float_2 sample_point = m_spherical_mapping[global_idx];
                    /// 计算(x, y, z)
                    /// NOTE: GeoGebra 3D使用如下坐标系, φ从Y轴向X轴绕
                    ///
                    /// ^ Z
                    /// |   / Y
                    /// |  /
                    /// | /
                    /// |/
                    /// o-------- X
                    ///
                    /// x = sin(θ)sin(φ)
                    /// y = sin(θ)cos(φ)
                    /// z = cos(θ)
                    ///
                    const float sin_phi   = std::sin(sample_point.x);
                    const float cos_phi   = std::cos(sample_point.x);
                    const float sin_theta = std::sin(sample_point.y);
                    const float cos_theta = std::cos(sample_point.y);
                    const float x = sin_theta * sin_phi;
                    const float y = sin_theta * cos_phi;
                    const float z = cos_theta;
                    std::snprintf(buffer, sizeof(buffer),
                                  "    (%.8f, %.8f, %.8f)", x, y, z);
                    /// (x0, y0, z0)
                    pnt_file_stream << buffer;
                    /// 判断是否输出尾部的逗号','
                    if (sample_idx < (SET_SAMPLE_COUNT - 1))
                    {
                        pnt_file_stream << ",";
                    }
                    pnt_file_stream << std::endl;
                }
                pnt_file_stream << "}" << std::endl;
                pnt_file_stream.close();
            }
            else
            {
                break;
            }
        }
        std::cout << "Mapping Done." << std::endl;
    }

    void
    generate_output_files ()
    {
        /// 输出的列表中每行最多多少个Sample数据
        static constexpr uint32_t SAMPLE_COUNT_PER_OUTPUT_LINE = 3;

        char buffer[128];

        assert(m_sample_array.size()   == SAMPLE_TOTAL_COUNT);
        assert(m_scramble_array.size() == SAMPLE_SET_COUNT);

        /// 输出Sample定义文件
        generate_output_file_header(m_define_file_stream);
        m_define_file_stream << "/// System headers\n";
        m_define_file_stream << "#include <stdint.h> /// uint32_t\n";
        m_define_file_stream << "\n\n";
        m_define_file_stream << "/// Hammersley采样组(SET)的总数\n";
        m_define_file_stream <<
            "static constexpr uint32_t HAMMERSLEY_SAMPLE_SET_COUNT = "
                              << SAMPLE_SET_COUNT << "U;\n";
        m_define_file_stream << "/// Hammersley每组中采样点(SAMPLE)的总数\n";
        m_define_file_stream <<
            "static constexpr uint32_t HAMMERSLEY_SET_SAMPLE_COUNT = "
                              << SET_SAMPLE_COUNT << "U;\n";
        m_define_file_stream << "/// Hammersley采样点(SAMPLE)的总数\n";
        m_define_file_stream <<
            "static constexpr uint32_t HAMMERSLEY_TOTAL_SAMPLE_COUNT = "
                              << SAMPLE_TOTAL_COUNT << "U;\n";

        /// 输出Sample数据文件
        generate_output_file_header(m_data_file_stream);
        m_data_file_stream << "/// Lib headers\n";
        m_data_file_stream << "#include \"DataType/Float2D.hpp\"\n";
        m_data_file_stream << "\n\n";

        /// 输出Unit Square上的Sample
        m_data_file_stream <<
            "/// Hammersley单位正方形(Unit Square)上所有采样点(SAMPLE)的数据\n";
        m_data_file_stream << "/// Note: 所有采样组(SAMPLE SET)连续存储:\n";
        m_data_file_stream << "/// SET0                      SET10\n";
        m_data_file_stream <<"/// +----+----+----+----+     +----+----+----+----+\n";
        m_data_file_stream <<"/// | S0 | S1 | S2 | S3 | ... | P0 | P1 | P2 | P3 |\n";
        m_data_file_stream <<"/// +----+----+----+----+     +----+----+----+----+\n";
        m_data_file_stream << "///\n";
        m_data_file_stream << "/// Hammersley Seed: " << m_hammersley_seed << std::endl;
        m_data_file_stream << "static constexpr float_2 ";
        m_data_file_stream << "HAMMERSLEY_UNIT_SQUARE_SAMPLE_ARRAY[] =\n";
        m_data_file_stream << "{\n";
        for (uint32_t set_idx = 0; set_idx < SAMPLE_SET_COUNT; ++set_idx)
        {
            std::snprintf(buffer, sizeof(buffer),
                          "    /// Sample Set(%u)|Scramble(0x%08X)\n",
                          set_idx, m_scramble_array[set_idx]);
            m_data_file_stream << buffer;
            for (uint32_t sample_idx = 0; sample_idx < SET_SAMPLE_COUNT; ++sample_idx)
            {
                /// 为每行输出开始
                if ((sample_idx % SAMPLE_COUNT_PER_OUTPUT_LINE) == 0)
                {
                    if (sample_idx)
                    {
                        m_data_file_stream << "\n";
                    }

                    m_data_file_stream << "    ";
                }

                /// Sample的全局索引
                const uint32_t global_sample_idx =
                    set_idx * SET_SAMPLE_COUNT + sample_idx;
                std::snprintf(buffer, sizeof(buffer), "{ % .8ff, % .8ff }",
                              m_sample_array[global_sample_idx].x,
                              m_sample_array[global_sample_idx].y);
                m_data_file_stream << buffer << ", ";
            }

            m_data_file_stream << "\n";

            if (set_idx != (SAMPLE_SET_COUNT-1))
            {
                m_data_file_stream << "\n";
            }
        }
        m_data_file_stream << "};\n";

        /// 输出所有Mapping数据
        if (m_spherical_mapping.size() == SAMPLE_TOTAL_COUNT)
        {
            m_data_file_stream << "\n\n";
            m_data_file_stream <<
                "/// Hammersley半球(Hemi-Sphere)上所有采样点(SAMPLE)的数据\n";
            m_data_file_stream <<
                "/// Note: 所有采样组(SAMPLE SET)连续存储, 存储顺序(φ, θ):\n";
            m_data_file_stream << "/// - (φ1, θ1) ... (φn, θn)\n";
            m_data_file_stream << "static constexpr float_2 ";
            m_data_file_stream << "HAMMERSLEY_SPHERICAL_SAMPLE_ARRAY[] =\n";
            m_data_file_stream << "{\n";
            for (uint32_t set_idx = 0; set_idx < SAMPLE_SET_COUNT; ++set_idx)
            {
                std::snprintf(
                    buffer, sizeof(buffer), "    /// Sample Set(%u)\n", set_idx);
                m_data_file_stream << buffer;
                for (uint32_t sample_idx = 0;
                     sample_idx < SET_SAMPLE_COUNT; ++sample_idx)
                {
                    /// 为每行输出开始
                    if ((sample_idx % SAMPLE_COUNT_PER_OUTPUT_LINE) == 0)
                    {
                        if (sample_idx)
                        {
                            m_data_file_stream << "\n";
                        }

                        m_data_file_stream << "    ";
                    }

                    /// Sample的全局索引
                    const uint32_t global_sample_idx =
                        set_idx * SET_SAMPLE_COUNT + sample_idx;
                    std::snprintf(buffer, sizeof(buffer), "{ % .8ff, % .8ff }",
                                  m_spherical_mapping[global_sample_idx].x,
                                  m_spherical_mapping[global_sample_idx].y);
                    m_data_file_stream << buffer << ", ";
                }

                m_data_file_stream << "\n";

                if (set_idx != (SAMPLE_SET_COUNT-1))
                {
                    m_data_file_stream << "\n";
                }
            }
            m_data_file_stream << "};\n";
        }
    }


private:
    static
    void
    generate_output_file_header (
        std::fstream & file_stream)
    {
        file_stream <<
        "//------------------------------------------------------------------------------//\n";
        file_stream <<
        "//------------------------------------------------------------------------------//\n";
        file_stream <<
        "//              AUTOMATICALLY GENERATED BY SAMPLE POINT GENERATOR               //\n";
        file_stream <<
        "//------------------------------------------------------------------------------//\n";
        file_stream <<
        "//------------------------------------------------------------------------------//\n";
        file_stream << "\n\n";
        file_stream << "#pragma once\n";
        file_stream << "\n\n";
    }

    /// 计算i的Φ函数(Radical Inverse Function): 即将i相对于小数点翻转
    ///
    /// 整数i表示为:
    ///     ──┐k=m
    /// i = >     aₖ * 2ᵏ
    ///     ──┘k=0
    ///
    ///         ──┐k=m
    /// Φ₂(i) = >     aₖ * 2⁻⁽ᵏ⁺¹⁾
    ///         ──┘k=0
    ///
    /// 例如: 0001 1011.0000 0000 --> 0000 0000.1101 1000
    static
    float
    Phi (
        const uint32_t i)
    {
        uint32_t j = i;
        float    x = 0.0f;
        float    f = 0.5f;

        while (j)
        {
            x += f * (float)(j & 1);
            j = j >> 1;
            f *= 0.5f;
        }

        return x;
    }

private:
    typedef std::vector<float_2>  SampleArrayT;
    typedef std::vector<uint32_t> ScrambleArrayT;

    /// 采样组(SET)的总数
    static constexpr uint32_t SAMPLE_SET_COUNT = 8;
    /// 每组中Sample的总数
    /// NOTE: 每组最多65536(2^16)个Sample
    static constexpr uint32_t SET_SAMPLE_COUNT = 1024;
    /// 所有组中Sample的总数
    static constexpr uint32_t SAMPLE_TOTAL_COUNT = SAMPLE_SET_COUNT * SET_SAMPLE_COUNT;
    static_assert(
        SAMPLE_SET_COUNT <= 65536,
        "At most 65536 sample sets are supported!!");
    static_assert(
        SET_SAMPLE_COUNT <= 65536,
        "Each sample set supports at most 65536 samples!!");

    /// Sample数组(所有组连续存储):
    /// SET0                      SETn
    /// +----+----+----+----+     +----+----+----+----+
    /// | S0 | S1 | S2 | S3 | ... | P0 | P1 | P2 | P3 |
    /// +----+----+----+----+     +----+----+----+----+
    SampleArrayT   m_sample_array;
    /// 每个SET使用的Scramble数值
    ScrambleArrayT m_scramble_array;
    /// --- 各种Mapping的Sample数组 ---
    /// 存储顺序: (φ, θ)
    SampleArrayT   m_spherical_mapping;
    /// 输出文件
    std::fstream   m_data_file_stream;
    std::fstream   m_define_file_stream;
    /// 计算Hammersley采样数据使用的随机Seed
    uint32_t       m_hammersley_seed;
    /// 标记输出文件是否成功打开
    bool           m_is_ready;
};



//-------------------------------------------------------------------------------------
// MAIN ENTRY
//-------------------------------------------------------------------------------------
// MARK: - 程序主入口

int32_t
main (
    const int32_t      argc,
    const char * const argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: SamplePointGen Output" << std::endl;
        return -1;
    }

    PointGenerator pnt_generator(argv[1]);
    if (pnt_generator.is_ready())
    {
        /// 创建Unit Square上所有Sample
        pnt_generator.generate_unit_square_samples();

        /// Mapping到Hemi-Sphere
        pnt_generator.map_onto_hemisphere();

        /// 输出生成结果
        pnt_generator.generate_output_files();
        std::cout << "Writing Done." << std::endl;
        return 0;
    }
    else
    {
        std::cout << "Can not generate data into file: '" << argv[1] << std::endl;
        return -1;
    }
}
