// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>

#include "base/filesystem/path.h"
#include "base/text/code_convert.h"
#include "base/containers/vector.h"

namespace base {
namespace {
base::Vector<base::String> TokenizeCommandLine(const base::StringRefU8 command_line) {
  base::Vector<base::String> tokens;
  size_t start = 0, end = 0;
  bool in_double_quotes = false;
  bool in_single_quotes = false;

  for (size_t i = 0; i < command_line.size(); i++) {
    char c = command_line[i];
    switch (c) {
      case ' ':
        if (!in_double_quotes && !in_single_quotes) {
          if (start != end) {
            tokens.push_back(command_line.substr(start, end - start));
          }
          start = end = i + 1;
        } else {
          end++;
        }
        break;
      case '\"':
        if (!in_single_quotes) {
          in_double_quotes = !in_double_quotes;
          end++;
        }
        break;
      case '\'':
        if (!in_double_quotes) {
          in_single_quotes = !in_single_quotes;
          end++;
        }
        break;
      default:
        end++;
        break;
    }
  }
  if (start != end) {
    tokens.push_back(command_line.substr(start, end - start));
  }
  return tokens;
}
}  // namespace

bool SpawnProcess(const Path& path_to_executable,
                  const base::StringRefU8 command_line) {
  // Make sure the command_line is null-terminated
  if (command_line[command_line.length() - 1] != '\0') {
    // Handle the error (throw an exception or return false)
    return false;
  }

  pid_t pid = fork();
  if (pid == -1) {
    // Forking failed
    return false;
  } else if (pid == 0) {
    // We're in the child process

    // Convert command_line string to a list of arguments
    // For simplicity, this assumes arguments are separated by spaces.
    // You might need a more robust solution for real-world usage.
    base::Vector<char*> args;
    char* token = std::strtok(&command_line[0], " ");
    while (token != nullptr) {
      args.push_back(token);
      token = std::strtok(nullptr, " ");  // is that portable enugh
    }
    args.push_back(nullptr);  // exec() expects a nullptr at the end

    execvp(path_to_executable.c_str(), &args[0]);

    // execvp() will only return if there's an error
    _exit(1);
  } else {
    // We're in the parent process
    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) && (WEXITSTATUS(status) == 0);
  }
}
}  // namespace base