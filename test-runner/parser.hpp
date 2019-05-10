#pragma once

#include <mbgl/util/filesystem.hpp>
#include <mbgl/util/rapidjson.hpp>
#include <mbgl/util/variant.hpp>

#include <tuple>
#include <string>
#include <vector>

struct TestMetadata;
struct TestStatistics;

using ErrorMessage = std::string;
using JSONReply = mbgl::variant<mbgl::JSDocument, ErrorMessage>;

using ArgumentsTuple = std::tuple<bool, bool, uint32_t, std::string, std::vector<std::string>>;

JSONReply readJson(const mbgl::filesystem::path&);
std::string serializeJsonValue(const mbgl::JSValue&);

ArgumentsTuple parseArguments(int argc, char** argv);
std::vector<std::pair<std::string, std::string>> parseIgnores();
TestMetadata parseTestMetadata(const mbgl::filesystem::path& path);

std::string createResultPage(const TestStatistics&, const std::vector<TestMetadata>&, bool shuffle, uint32_t seed);

std::string localizeURL(const std::string& url);

void localizeSourceURLs(mbgl::JSValue& root, mbgl::JSDocument& document);
void localizeStyleURLs(mbgl::JSValue& root, mbgl::JSDocument& document);