#include "External/Catch/catch_v2.hpp"
#include "UT_Config.hpp"

#if (HASHING_CHECKING == 1)
/// System headers
#include <stdint.h> /// uint32_t,...
#include <unordered_map>
/// Library headers
#include "Common/CommonDefines.hpp"
#include "DataType/HashedStringId.hpp"
#include "Hashing/StaticStringHash.hpp"
#include "Hashing/RuntimeHash.hpp"


#define CHECK_STATIC_HASH_32(string_array, idx)                                   \
{                                                                                 \
  const uint32_t buildtime_hash = STATIC_STRING_HASH_32 (seed, string_array[idx]);\
  const uint32_t runtime_hash   = RUNTIME_STRING_HASH_32(seed, string_array[idx]);\
  /* 注意: INFO()必须在REQURE()之前调用 */                                           \
  INFO("Buildtime Hash != Runtime Hash for string at index " << idx << ".")       \
  REQUIRE((buildtime_hash == runtime_hash));                                      \
}

#define CHECK_STATIC_HASH_64(string_array, idx)                                   \
{                                                                                 \
  const uint64_t buildtime_hash = STATIC_STRING_HASH_64 (seed, string_array[idx]);\
  const uint64_t runtime_hash   = RUNTIME_STRING_HASH_64(seed, string_array[idx]);\
  /* 注意: INFO()必须在REQURE()之前调用 */                                           \
  INFO("Buildtime Hash != Runtime Hash for string at index " << idx << ".")       \
  REQUIRE((buildtime_hash == runtime_hash));                                      \
}


/// TEST_CASE( name [, tags ] )
TEST_CASE("Checking Compiletime/Runtime XXH3 Hash", "[Compiletime/Runtime XXH3 Hash]")
{

    /// Hash的32位Seed
    const uint32_t seed = FOUR_CC_32('S', 'e', 'e', 'D');


    SECTION("Checking Hash:")
    {
        static constexpr const char * const ASCII_STRINGS[] =
        {
            "name",
            "phone",
            "email",
            "address",
            "list",
            "country",
            "region",
            "postalZip",
            "text",
            "numberrange",
            "currency",
            "alphanumeric",
            "Geoffrey Schneider",
            "1-706-426-6433",
            "integer.eu.lacus@icloud.couk",
            "Ap #392-6663 Faucibus Ave",
            "17",
            "France",
            "Chiapas",
            "161221",
            "\"dictum. Phasellus in felis. Nulla tempor augue ac ipsum. Phasellus vitae mauris sit amet lorem semper auctor. Mauris vel turpis. Aliquam adipiscing lobortis risus. In mi pede",
            "nonummy ut",
            "molestie in",
            "tempus eu",
            "ligula. Aenean euismod mauris eu elit. Nulla facilisi. Sed neque. Sed eget lacus. Mauris non dui nec urna suscipit nonummy. Fusce fermentum fermentum arcu. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae Phasellus ornare. Fusce mollis. Duis sit amet diam eu dolor egestas rhoncus. Proin nisl sem",
            "consequat nec",
            "mollis vitae",
            "posuere at",
            "velit. Cras lorem lorem",
            "luctus ut",
            "pellentesque eget",
            "dictum\"",
            "45",
            "$55.96",
            "SSN01CYS2YM",
            "Vivian Pruitt",
            "(242) 460-3865",
            "eros.nam@google.ca",
            "759-2748 Nascetur St.",
            "15",
            "Pakistan",
            "North Gyeongsang",
            "12738",
            "\"Nunc sed orci lobortis augue scelerisque mollis. Phasellus libero mauris",
            "aliquam eu",
            "accumsan sed",
            "facilisis vitae",
            "orci. Phasellus dapibus quam quis diam. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Fusce aliquet magna a neque. Nullam ut nisi a odio semper cursus. Integer mollis. Integer tincidunt aliquam arcu. Aliquam ultrices iaculis odio. Nam interdum enim non nisi. Aenean eget metus. In nec orci. Donec nibh. Quisque nonummy ipsum non arcu. Vivamus sit amet risus. Donec egestas. Aliquam nec enim. Nunc ut erat. Sed nunc est",
            "mollis non",
            "cursus non",
            "egestas a",
            "dui. Cras pellentesque. Sed\"",
            "328",
            "$65.67",
            "USM88FIN4LC",
            "Isadora Velasquez",
            "(562) 898-5361",
            "ac.sem@yahoo.org",
            "Ap #522-7844 Faucibus. Av.",
            "5",
            "Australia",
            "Zhongnan",
            "34595",
            "\"turpis. In condimentum. Donec at arcu. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae Donec tincidunt. Donec vitae erat vel pede blandit congue. In scelerisque scelerisque dui. Suspendisse ac metus vitae velit egestas lacinia. Sed congue",
            "elit sed consequat auctor",
            "nunc nulla vulputate dui",
            "nec tempus mauris erat eget ipsum. Suspendisse sagittis. Nullam vitae diam. Proin dolor. Nulla semper tellus id nunc interdum feugiat. Sed nec metus facilisis lorem tristique aliquet. Phasellus fermentum convallis ligula. Donec luctus aliquet odio. Etiam ligula tortor",
            "dictum eu",
            "placerat eget",
            "venenatis a",
            "magna. Lorem ipsum dolor sit amet",
            "consectetuer\"",
            "411",
            "$23.09",
            "LHX52FJJ1WD",
            "Cairo Prince",
            "1-831-367-8675",
            "ligula@protonmail.ca",
            "3710 Leo",
            "Rd.\"",
            "9",
            "Mexico",
            "New South Wales",
            "14040",
            "\"id",
            "ante.Nunc mauris sapien",
            "cursus in",
            "hendrerit consectetuer",
            "cursus et",
            "magna.Praesent interdum ligula eu enim.Etiam imperdiet dictum magna.Ut tincidunt orci quis lectus.Nullam suscipit",
            "est ac facilisis facilisis",
            "magna tellus faucibus leo",
            "in lobortis tellus justo sit amet nulla.Donec non justo.Proin non massa non ante bibendum ullamcorper.Duis cursus",
            "diam at pretium aliquet",
            "metus urna convallis erat",
            "eget tincidunt dui augue eu tellus.Phasellus elit pede",
            "malesuada vel",
            "venenatis vel",
            "faucibus id",
            "libero.Donec consectetuer mauris id sapien.Cras dolor dolor",
            "Merritt Adams",
            "1-736-362-5791",
            "sem@outlook.ca",
            "\"1160 Pharetra",
            " Avenue\"",
            "17",
            "Russian Federation",
            "Kujawsko-pomorskie",
            "6556",
            "\"arcu. Morbi sit amet massa. Quisque porttitor eros nec tellus. Nunc lectus pede",
            " ultrices a",
            " auctor non",
            " feugiat nec",
            " diam. Duis mi enim",
            " condimentum eget",
            " volutpat ornare",
            " facilisis eget",
            " ipsum. Donec sollicitudin adipiscing ligula. Aenean gravida nunc sed pede. Cum sociis natoque penatibus et magnis dis parturient montes",
            " nascetur ridiculus mus. Proin vel arcu eu odio tristique pharetra. Quisque ac libero nec ligula consectetuer rhoncus. Nullam velit dui",
            " semper et",
            " lacinia vitae",
            " sodales at",
            " velit. Pellentesque ultricies dignissim lacus. Aliquam rutrum lorem ac risus. Morbi metus. Vivamus euismod urna. Nullam lobortis quam a felis ullamcorper viverra. Maecenas iaculis aliquet diam.\"",
            "447",
            "$20.41",
            "MYO12VBI1ES",
            "Halla Moreno",
            "1-752-770-7035",
            "consectetuer.adipiscing@outlook.net",
            "Ap #785-3683 Elit. Ave",
            "19",
            "South Africa",
            "Kocaeli",
            "928232",
            "\"metus eu erat semper rutrum. Fusce dolor quam",
            " elementum at",
            " egestas a",
            " scelerisque sed",
            " sapien. Nunc pulvinar arcu et pede. Nunc sed orci lobortis augue scelerisque mollis. Phasellus libero mauris",
            " aliquam eu",
            " accumsan sed",
            " facilisis vitae",
            " orci. Phasellus dapibus quam quis diam. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Fusce aliquet magna a neque. Nullam ut nisi a odio semper cursus. Integer mollis. Integer tincidunt aliquam arcu. Aliquam ultrices iaculis odio. Nam interdum enim non nisi. Aenean eget metus. In nec orci. Donec nibh. Quisque nonummy ipsum non arcu. Vivamus sit amet risus. Donec\"",
            "318",
            "$57.71",
            "YCT65HCF8EE",
            "Chantale Petty",
            "1-674-826-3459",
            "eu.nulla@outlook.couk",
            "309-8725 Lobortis Av.",
            "7",
            "Mexico",
            "Ontario",
            "18-846",
            "\"sed turpis nec mauris blandit mattis. Cras eget nisi dictum augue malesuada malesuada. Integer id magna et ipsum cursus vestibulum. Mauris magna. Duis dignissim tempor arcu. Vestibulum ut eros non enim commodo hendrerit. Donec porttitor tellus non magna. Nam ligula elit",
            " pretium et",
            " rutrum non",
            " hendrerit id",
            " ante. Nunc mauris sapien",
            " cursus in",
            " hendrerit consectetuer",
            " cursus et",
            " magna. Praesent interdum ligula eu enim. Etiam imperdiet dictum magna. Ut tincidunt orci quis lectus. Nullam suscipit",
            " est ac facilisis facilisis",
            " magna tellus faucibus leo",
            " in lobortis tellus justo sit amet nulla. Donec non justo. Proin non massa non ante bibendum ullamcorper. Duis\"",
            "114",
            "$59.67",
            "EIS56DXI7TN",
            "Quentin Acosta",
            "1-228-467-1545",
            "duis.risus@outlook.couk",
            "901-9559 Ridiculus St.",
            "5",
            "Chile",
            "Central Region",
            "97E 3Y7",
            "\"nulla vulputate dui",
            " nec tempus mauris erat eget ipsum. Suspendisse sagittis. Nullam vitae diam. Proin dolor. Nulla semper tellus id nunc interdum feugiat. Sed nec metus facilisis lorem tristique aliquet. Phasellus fermentum convallis ligula. Donec luctus aliquet odio. Etiam ligula tortor",
            " dictum eu",
            " placerat eget",
            " venenatis a",
            " magna. Lorem ipsum dolor sit amet",
            " consectetuer adipiscing elit. Etiam laoreet",
            " libero et tristique pellentesque",
            " tellus sem mollis dui",
            " in sodales elit erat vitae risus. Duis a mi fringilla mi lacinia mattis. Integer eu lacus. Quisque imperdiet",
            " erat nonummy ultricies ornare",
            " elit elit fermentum risus",
            " at fringilla purus mauris a nunc. In at\"",
            "8",
            "$6.50",
            "PND68MSK6PZ",
            "Iona Robinson",
            "1-833-366-1176",
            "aliquam.nisl@protonmail.edu",
            "\"P.O. Box 541",
            " 2492 Arcu. Av.\"",
            "1",
            "Ireland",
            "440953",
            "\"convallis ligula. Donec luctus aliquet odio. Etiam ligula tortor",
            " dictum eu",
            " placerat eget",
            " venenatis a",
            " magna. Lorem ipsum dolor sit amet",
            " consectetuer adipiscing elit. Etiam laoreet",
            " libero et tristique pellentesque",
            " tellus sem mollis dui",
            " in sodales elit erat vitae risus. Duis a mi fringilla mi lacinia mattis. Integer eu lacus. Quisque imperdiet",
            " erat nonummy ultricies ornare",
            " elit elit fermentum risus",
            " at fringilla purus mauris a nunc. In at pede. Cras vulputate velit eu sem. Pellentesque ut ipsum ac mi eleifend egestas. Sed pharetra",
            " felis eget varius ultrices",
            " mauris ipsum porta elit",
            " a feugiat tellus lorem eu metus. In lorem. Donec\"",
            "87",
            "$50.35",
            "0b406ce4-57c6-48ec-9fdb-9f4b338161f5",
            "0ead6676-2079-49c1-987e-f9d6e0a4a0ad",
        };

        static constexpr const char * const UTF8_STRINGS[] =
        {
            u8"編碼测试",
            u8"模式识别（官方标准）：人们通过大量的经验，得到结论，从而判断它就是猫。",
            u8"机器学习（数据学习）：人们通过阅读进行学习，观察它会叫、小眼睛、两只耳朵、四条腿、一条尾巴，得到结论，从而判断它就是猫。",
            u8"深度学习（深入数据）：人们通过深入了解它，发现它会'喵喵'的叫、与同类的猫科动物很类似，得到结论，从而判断它就是猫。（深度学习常用领域：语音识别、图像识别）",
            u8"模式识别（pattern recognition）: 模式识别是最古老的（作为一个术语而言，可以说是很过时的）。",
            u8"我们把环境与客体统称为“模式”，识别是对模式的一种认知，是如何让一个计算机程序去做一些看起来很“智能”的事情。",
            u8"通过融于智慧和直觉后，通过构建程序，识别一些事物，而不是人，例如: 识别数字。",
            u8"机器学习（machine learning）: 机器学习是最基础的（当下初创公司和研究实验室的热点领域之一）。",
            u8"在90年代初，人们开始意识到一种可以更有效地构建模式识别算法的方法，那就是用数据（可以通过廉价劳动力采集获得）去替换专家（具有很多图像方面知识的人）。",
            u8"“机器学习”强调的是，在给计算机程序（或者机器）输入一些数据后，它必须做一些事情，那就是学习这些数据，而这个学习的步骤是明确的。",
            u8"机器学习（Machine Learning）是一门专门研究计算机怎样模拟或实现人类的学习行为，以获取新的知识或技能，重新组织已有的知识结构使之不断改善自身性能的学科。",
            u8"深度学习（deep learning）: 深度学习是非常崭新和有影响力的前沿领域，我们甚至不会去思考-后深度学习时代。",
            u8"深度学习是机器学习研究中的一个新的领域，其动机在于建立、模拟人脑进行分析学习的神经网络，它模仿人脑的机制来解释数据，例如图像，声音和文本。",
            u8"参考地址：",
            u8"不同操作系统的 page size（内存页大小）通常如下。需要注意：现代系统很多已经支持多种 page size（如 huge page / large page），但这里列的是默认普通页大小。",
            u8"操作系统    CPU架构          默认 Page Size                备注",
            u8"iOS       ARM64            16 KB                        Apple 从 A7 开始基本使用 16K page",
            u8"macOS     Apple Silicon    16 KB                        与 iOS 一致",
            u8"macOS     Intel x86_64     4 KB                         Intel Mac 传统页大小",
            u8"Windows   x86 / x86_64     4 KB                         Windows 标准页",
            u8"Linux     x86 / x86_64     4 KB                         最常见配置",
            u8"Linux     ARM64            4 KB / 16 KB / 64 KB         编译内核时选择"
        };

        // --- 测试ASCII编码字符串 --- //
        std::printf("--- Checking Static/Runtime XXH3 32Bits Hash of ASCII strings...\n");
        for (uint64_t idx = 0; idx < ARRAY_LENGTH(ASCII_STRINGS); ++idx)
        {
            CHECK_STATIC_HASH_32(ASCII_STRINGS, idx);
        }
        std::printf("--- Checking Static/Runtime XXH3 32Bits Hash of ASCII strings: OK!\n");

        std::printf("--- Checking Static/Runtime XXH3 64Bits Hash of ASCII strings...\n");
        for (uint64_t idx = 0; idx < ARRAY_LENGTH(ASCII_STRINGS); ++idx)
        {
            CHECK_STATIC_HASH_64(ASCII_STRINGS, idx);
        }
        std::printf("--- Checking Static/Runtime XXH3 64Bits Hash of ASCII strings: OK!\n");


        // --- 测试UTF8编码字符串 --- //
        std::printf("--- Checking Static/Runtime XXH3 32Bits Hash of UTF8 strings...\n");
        for (uint64_t idx = 0; idx < ARRAY_LENGTH(UTF8_STRINGS); ++idx)
        {
            CHECK_STATIC_HASH_32(UTF8_STRINGS, idx);
        }
        std::printf("--- Checking Static/Runtime XXH3 32Bits Hash of UTF8 strings: OK!\n");

        std::printf("--- Checking Static/Runtime XXH3 64Bits Hash of UTF8 strings...\n");
        for (uint64_t idx = 0; idx < ARRAY_LENGTH(UTF8_STRINGS); ++idx)
        {
            CHECK_STATIC_HASH_64(UTF8_STRINGS, idx);
        }
        std::printf("--- Checking Static/Runtime XXH3 64Bits Hash of UTF8 strings: OK!\n");
    }

} /// TEST_CASE("Checking Compile/Runtime XXH3 Hash", "[XXH3 Hash]")


TEST_CASE("Checking Hashed String Id", "[Hashed String Id]")
{
    using TestIdT = HashedStringId< FOUR_CC_32('T', 'E', 'S', 'T') >;

    constexpr TestIdT invalid_id;
    constexpr TestIdT buildtime_id = TestIdT::buildtime_id("Player");

    std::printf("--- Checking Hashed String Id...\n");

    REQUIRE(invalid_id.is_not_valid());
    REQUIRE(buildtime_id.is_valid());
    REQUIRE((buildtime_id == TestIdT::buildtime_id("Player")));

    const TestIdT runtime_id = TestIdT::runtime_id((const uint8_t*)"Player");
    REQUIRE((runtime_id == buildtime_id));
    REQUIRE((runtime_id != TestIdT::runtime_id((const uint8_t*)"Enemy")));

    using IdTableT = std::unordered_map< TestIdT,
                                         uint32_t,
                                         TestIdT::StlHasher,
                                         TestIdT::StlComparer >;

    IdTableT id_table;
    id_table.emplace(buildtime_id, 7);

    REQUIRE((id_table.find(runtime_id) != id_table.end()));
    REQUIRE((id_table.find(runtime_id)->second == 7));

    std::printf("--- Checking Hashed String Id: OK!\n");
}

#endif /// (HASHING_CHECKING == 1)
