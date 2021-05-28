// Copyright (C) 2020  Matthew Glazar
// See end of file for extended copyright information.

#include <gtest/gtest.h>
#include <quick-lint-js/configuration-loader.h>
#include <quick-lint-js/configuration.h>
#include <quick-lint-js/file.h>
#include <quick-lint-js/options.h>
#include <quick-lint-js/temporary-directory.h>
#include <quick-lint-js/warning.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

QLJS_WARNING_IGNORE_GCC("-Wmissing-field-initializers")

#define EXPECT_DEFAULT_CONFIG(config)                                  \
  do {                                                                 \
    EXPECT_TRUE((config).globals().find(u8"Array"sv));                 \
    EXPECT_TRUE((config).globals().find(u8"console"sv));               \
    EXPECT_FALSE((config).globals().find(u8"variableDoesNotExist"sv)); \
  } while (false)

using namespace std::literals::string_view_literals;

namespace quick_lint_js {
namespace {
class test_configuration_loader : public ::testing::Test {
 public:
  std::string make_temporary_directory() {
    std::string temp_dir = quick_lint_js::make_temporary_directory();
    this->temporary_directories_.emplace_back(temp_dir);
    return temp_dir;
  }

 protected:
  void TearDown() override {
    for (const std::string& temp_dir : this->temporary_directories_) {
      delete_directory_recursive(temp_dir);
    }
  }

 private:
  std::vector<std::string> temporary_directories_;
};

TEST_F(test_configuration_loader,
       file_with_no_config_file_gets_default_config) {
  configuration_loader loader;
  configuration* config = loader.load_for_file(file_to_lint{
      .path = "hello.js",
      .config_file = nullptr,
  });
  EXPECT_DEFAULT_CONFIG(*config);
}

TEST_F(test_configuration_loader, file_with_config_file_gets_loaded_config) {
  std::string temp_dir = this->make_temporary_directory();
  std::string config_file = temp_dir + "/config.json";
  write_file(config_file, u8R"({"globals": {"testGlobalVariable": true}})"sv);

  configuration_loader loader;
  configuration* config = loader.load_for_file(file_to_lint{
      .path = "hello.js",
      .config_file = config_file.c_str(),
  });

  EXPECT_TRUE(config->globals().find(u8"testGlobalVariable"sv));
}

TEST_F(test_configuration_loader,
       files_with_same_config_file_get_same_loaded_config) {
  std::string temp_dir = this->make_temporary_directory();
  std::string config_file = temp_dir + "/config.json";
  write_file(config_file, u8R"({"globals": {"testGlobalVariable": true}})"sv);

  configuration_loader loader;
  configuration* config_one = loader.load_for_file(file_to_lint{
      .path = "one.js",
      .config_file = config_file.c_str(),
  });
  configuration* config_two = loader.load_for_file(file_to_lint{
      .path = "two.js",
      .config_file = config_file.c_str(),
  });

  EXPECT_EQ(config_one, config_two) << "pointers should be the same";
}

TEST_F(test_configuration_loader,
       files_with_different_config_files_get_different_loaded_config) {
  std::string temp_dir = this->make_temporary_directory();
  std::string config_file_one = temp_dir + "/config-one.json";
  write_file(config_file_one,
             u8R"({"globals": {"testGlobalVariableOne": true}})"sv);
  std::string config_file_two = temp_dir + "/config-two.json";
  write_file(config_file_two,
             u8R"({"globals": {"testGlobalVariableTwo": true}})"sv);

  configuration_loader loader;
  configuration* config_one = loader.load_for_file(file_to_lint{
      .path = "one.js",
      .config_file = config_file_one.c_str(),
  });
  configuration* config_two = loader.load_for_file(file_to_lint{
      .path = "two.js",
      .config_file = config_file_two.c_str(),
  });

  EXPECT_NE(config_one, config_two) << "pointers should be different";
  EXPECT_TRUE(config_one->globals().find(u8"testGlobalVariableOne"sv));
  EXPECT_FALSE(config_one->globals().find(u8"testGlobalVariableTwo"sv));
  EXPECT_FALSE(config_two->globals().find(u8"testGlobalVariableOne"sv));
  EXPECT_TRUE(config_two->globals().find(u8"testGlobalVariableTwo"sv));
}
}
}

// quick-lint-js finds bugs in JavaScript programs.
// Copyright (C) 2020  Matthew Glazar
//
// This file is part of quick-lint-js.
//
// quick-lint-js is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// quick-lint-js is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with quick-lint-js.  If not, see <https://www.gnu.org/licenses/>.