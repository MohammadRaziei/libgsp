#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <toml++/toml.h>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <filesystem>
#include "base64.h"

using namespace std;
namespace fs = std::filesystem;

class Base64Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up common test data
        emptyString = "";
        testString = "Hello, World!";
        testBinary = {0x00, 0x01, 0x02, 0x7F, 0x80, 0xFF};
        
        // Pre-computed expected values
        emptyStringEncoded = "";
        testStringEncoded = "SGVsbG8sIFdvcmxkIQ==";
        testBinaryEncoded = "AAECf4D/";
    }

    string emptyString;
    string testString;
    vector<uint8_t> testBinary;
    
    string emptyStringEncoded;
    string testStringEncoded;
    string testBinaryEncoded;
};

TEST_F(Base64Test, EncodeEmptyString) {
    string result;
    b2t::base64_encode(result, emptyString);
    EXPECT_EQ(result, emptyStringEncoded);
}

TEST_F(Base64Test, EncodeString) {
    string result;
    b2t::base64_encode(result, testString);
    EXPECT_EQ(result, testStringEncoded);
}

TEST_F(Base64Test, EncodeBinaryVector) {
    string result;
    b2t::base64_encode(result, testBinary);
    EXPECT_EQ(result, testBinaryEncoded);
}

TEST_F(Base64Test, EncodeBinaryData) {
    string result;
    b2t::base64_encode(result, testBinary.data(), testBinary.size());
    EXPECT_EQ(result, testBinaryEncoded);
}

TEST_F(Base64Test, DecodeToVector) {
    vector<uint8_t> result;
    b2t::base64_decode(result, testBinaryEncoded);
    EXPECT_EQ(result, testBinary);
}

TEST_F(Base64Test, DecodeToString) {
    string result;
    b2t::base64_decode(result, testStringEncoded);
    EXPECT_EQ(result, testString);
}

TEST_F(Base64Test, EncodeDecodeRoundTripString) {
    string encoded, decoded;
    b2t::base64_encode(encoded, testString);
    b2t::base64_decode(decoded, encoded);
    EXPECT_EQ(decoded, testString);
}

TEST_F(Base64Test, EncodeDecodeRoundTripBinary) {
    string encoded;
    vector<uint8_t> decoded;
    b2t::base64_encode(encoded, testBinary);
    b2t::base64_decode(decoded, encoded);
    EXPECT_EQ(decoded, testBinary);
}

TEST_F(Base64Test, DecodeInvalidBase64) {
    // Test with invalid Base64 characters
    string invalidBase64 = "SGVsbG8sIFdvcmxkIQ==!@#";
    string result;

    // Should throw an exception or handle it gracefully
    EXPECT_NO_THROW(b2t::base64_decode(result, invalidBase64));

    // The result should be truncated at the first invalid character
    EXPECT_EQ(result, "Hello, World!");
}

TEST_F(Base64Test, EncodeDecodeAllBytes) {
    // Test all possible byte values
    vector<uint8_t> allBytes(256);
    for (int i = 0; i < 256; ++i) {
        allBytes[i] = static_cast<uint8_t>(i);
    }

    string encoded;
    vector<uint8_t> decoded;

    b2t::base64_encode(encoded, allBytes);
    b2t::base64_decode(decoded, encoded);

    EXPECT_EQ(decoded, allBytes);
}



struct Base64TestCase {
    std::string description;
    std::vector<uint8_t> input;
    std::string expected_encoded;
};

class Base64TestP : public ::testing::TestWithParam<Base64TestCase> {};

TEST_P(Base64TestP, EncodeMatchesExpected) {
    const auto& test_case = GetParam();
    std::string actual_encoded;
    b2t::base64_encode(actual_encoded, test_case.input);
    EXPECT_EQ(actual_encoded, test_case.expected_encoded);
}

TEST_P(Base64TestP, DecodeRoundtrip) {
    const auto& test_case = GetParam();
    std::string encoded = test_case.expected_encoded;
    std::vector<uint8_t> decoded;
    b2t::base64_decode(decoded, encoded);
    EXPECT_EQ(decoded, test_case.input);
}
std::vector<Base64TestCase> LoadBase64TestCases() {
    std::vector<Base64TestCase> cases;

    // Load TOML file
    auto config = toml::parse_file((fs::path(__FILE__).parent_path().parent_path() / "data/utils.toml").string());
    auto base64_array = config["base64"].as_array();
    if (!base64_array) {
        throw std::runtime_error("Missing or invalid [base64] array in utils.toml");
    }

    for (const auto& entry_node : *base64_array) {
        // Each entry must be a table
        auto entry = entry_node.as_table();
        if (!entry) {
            throw std::runtime_error("Base64 test case is not a table");
        }

        std::string desc = entry->at("description").value_or("unnamed");
        std::string hex = entry->at("input_hex").value_or("");
        std::string expected = entry->at("expected_encoded").value_or("");

        // Convert hex string to vector<uint8_t>
        std::vector<uint8_t> input;
        if (!hex.empty()) {
            if (hex.length() % 2 != 0) {
                throw std::runtime_error("Invalid hex string (odd length)");
            }
            for (size_t i = 0; i < hex.length(); i += 2) {
                std::string byte_str = hex.substr(i, 2);
                uint8_t byte = static_cast<uint8_t>(std::stoi(byte_str, nullptr, 16));
                input.push_back(byte);
            }
        }

        cases.push_back({desc, input, expected});
    }

    return cases;
}
INSTANTIATE_TEST_SUITE_P(
    Base64Parameterized,
    Base64TestP,
    ::testing::ValuesIn(LoadBase64TestCases()),
    [](const ::testing::TestParamInfo<Base64TestCase>& info) {
        // Use description as test name (sanitize it)
        std::string name = info.param.description;
        // Replace non-alphanumeric chars with underscores
        for (auto& c : name) {
            if (!std::isalnum(c)) c = '_';
        }
        return name;
    }
);
