#include <gtest/gtest.h>
#include <libgsp/utils/Base64.h>
#include <string>
#include <vector>

using namespace gsp::utils;
using namespace std;

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
    base64_encode(result, emptyString);
    EXPECT_EQ(result, emptyStringEncoded);
}

TEST_F(Base64Test, EncodeString) {
    string result;
    base64_encode(result, testString);
    EXPECT_EQ(result, testStringEncoded);
}

TEST_F(Base64Test, EncodeBinaryVector) {
    string result;
    base64_encode(result, testBinary);
    EXPECT_EQ(result, testBinaryEncoded);
}

TEST_F(Base64Test, EncodeBinaryData) {
    string result;
    base64_encode(result, testBinary.data(), testBinary.size());
    EXPECT_EQ(result, testBinaryEncoded);
}

TEST_F(Base64Test, DecodeToVector) {
    vector<uint8_t> result;
    base64_decode(result, testBinaryEncoded);
    EXPECT_EQ(result, testBinary);
}

TEST_F(Base64Test, DecodeToString) {
    string result;
    base64_decode(result, testStringEncoded);
    EXPECT_EQ(result, testString);
}

TEST_F(Base64Test, EncodeDecodeRoundTripString) {
    string encoded, decoded;
    base64_encode(encoded, testString);
    base64_decode(decoded, encoded);
    EXPECT_EQ(decoded, testString);
}

TEST_F(Base64Test, EncodeDecodeRoundTripBinary) {
    string encoded;
    vector<uint8_t> decoded;
    base64_encode(encoded, testBinary);
    base64_decode(decoded, encoded);
    EXPECT_EQ(decoded, testBinary);
}

TEST_F(Base64Test, DecodeInvalidBase64) {
    // Test with invalid Base64 characters
    string invalidBase64 = "SGVsbG8sIFdvcmxkIQ==!@#";
    string result;
    
    // Should throw an exception or handle it gracefully
    EXPECT_NO_THROW(base64_decode(result, invalidBase64));
    
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
    
    base64_encode(encoded, allBytes);
    base64_decode(decoded, encoded);
    
    EXPECT_EQ(decoded, allBytes);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
