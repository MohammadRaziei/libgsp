//
// Created by Mohammad on 9/26/2025.
//


#include <string>
#include <gtest/gtest.h>

#include "libgsp/utils/GspInfo.h"

using namespace gsp;

namespace {

// Snapshot of fields that have public getters
struct InfoSnapshot {
    std::string name;
    std::string author_name;
    std::string author_email;
    std::string site_url;
    std::string version;
    std::string language;
    std::string os;
};

} // namespace

// Test fixture to isolate tests by saving/restoring Info singleton state.
class InfoTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto& info = Info::instance();
        snap.name = info.name();
        snap.author_name = info.authorName();
        snap.author_email = info.authorEmail();
        snap.site_url     = info.siteUrl();
        snap.version      = info.version();
        snap.language     = info.language();
        snap.os           = info.os();
    }

    void TearDown() override {
        auto& info = Info::instance();
        info.setName(snap.name)
            .setAuthorName(snap.author_name)
            .setAuthorEmail(snap.author_email)
            .setSiteUrl(snap.site_url)
            .setLanguage(snap.language)
            .setVersion(snap.version)
            .setOs(snap.os);
    }

    InfoSnapshot snap;
};

TEST_F(InfoTest, SingletonIdentity) {
    auto& a = Info::instance();
    auto& b = Info::instance();
    EXPECT_EQ(&a, &b) << "Info::instance() must return the same object (singleton).";
}

TEST_F(InfoTest, DefaultsArePopulatedAndSane) {
    auto& info = Info::instance();

    // Version must be provided by build macro and not fall back.
    EXPECT_FALSE(info.version().empty()) << "Version must not be empty.";
    EXPECT_NE(info.version(), "__dev__") << "Version fallback '__dev__' must not be used in tests.";

    // Site/author/email must be provided (from CMake/pyproject injection).
    EXPECT_FALSE(info.siteUrl().empty())     << "Site URL must not be empty.";
    EXPECT_FALSE(info.authorName().empty())  << "Author name must not be empty.";
    EXPECT_FALSE(info.authorEmail().empty()) << "Author email must not be empty.";

    // Language default is hard-coded to cpp.
    EXPECT_EQ(info.language(), "cpp") << "Default language must be 'cpp'.";

    // OS should be detected; unknown means detection failed.
    EXPECT_NE(info.os(), "unknown") << "OS detection returned 'unknown'. Check toolchain macros.";
}

TEST_F(InfoTest, SettersRoundTrip) {
    auto& info = Info::instance();
    const std::string new_author   = "Unit Test Author";
    const std::string new_email    = "unit@test.example";
    const std::string new_site     = "https://unit.test";
    const std::string new_language = "rust";
    const std::string new_os       = "plan9";

    info.setAuthorName(new_author)
        .setAuthorEmail(new_email)
        .setSiteUrl(new_site)
        .setLanguage(new_language)
        .setOs(new_os);
    // .setName("UnitTestLib");  // No getter available in current header to verify.

    EXPECT_EQ(info.authorName(),  new_author);
    EXPECT_EQ(info.authorEmail(), new_email);
    EXPECT_EQ(info.siteUrl(),     new_site);
    EXPECT_EQ(info.language(),    new_language);
    EXPECT_EQ(info.os(),          new_os);
}

TEST_F(InfoTest, Chainability) {
    auto& info = Info::instance();

    // Chain a different set of values to ensure fluent API is preserved.
    info.setAuthorName("A")
        .setAuthorEmail("a@example.com")
        .setSiteUrl("https://a.example")
        .setLanguage("zig")
        .setOs("amiga");

    EXPECT_EQ(info.authorName(),  "A");
    EXPECT_EQ(info.authorEmail(), "a@example.com");
    EXPECT_EQ(info.siteUrl(),     "https://a.example");
    EXPECT_EQ(info.language(),    "zig");
    EXPECT_EQ(info.os(),          "amiga");
}

TEST_F(InfoTest, NameSetterWhenGetterIsAvailable) {
     auto& info = Info::instance();
     EXPECT_EQ(info.name(), "libgsp");
}
