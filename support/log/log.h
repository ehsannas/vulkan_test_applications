/* Copyright 2016 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <memory>
#include <string>
#include <sstream>

namespace logging {

// Logging class base. It provides the functionality to generate log messages
// for use by any inherited classes.
class Logger {
public:
  // Logs a set of values to the error stream of the logger.
  template <typename... Args> void LogError(Args... args) {
    std::ostringstream str;
    LogHelper(&str, args...);
    str << "\n";
    LogErrorString(str.str().c_str());
  }

  // Logs a set of values to the info stream of the logger.
  template <typename... Args> void LogInfo(Args... args) {
    std::ostringstream str;
    LogHelper(&str, args...);
    str << "\n";
    LogInfoString(str.str().c_str());
  }

private:
  // Helper function, the recursive base of LogHelper.
  template <typename T> void LogHelper(std::ostringstream *stream, const T &val) {
    *stream << val;
  }

  // Helper function to recursively add elements from Args to the stream.
  template <typename T, typename... Args>
  void LogHelper(std::ostringstream *stream, const T &val, Args... args) {
    *stream << val;
    LogHelper(stream, args...);
  }

  // This should be overriden by child classes to log the input null-terminated
  // string to the STDERR equivalent.
  virtual void LogErrorString(const char *str) = 0;
  // This should be overriden by child classes to log the input null-terminated
  // string to the STDOUT equivalent.
  virtual void LogInfoString(const char *str) = 0;
};

// Returns a platform-specific logger.
std::unique_ptr<Logger> GetLogger();
}