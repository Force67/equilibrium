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
base::Vector<base::StringU8> TokenizeCommandLine(const base::StringRefU8 command_line) {
  base::Vector<base::StringU8> tokens;
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

bool SpawnProcess(const Path& path_to_executable, const base::StringRefU8 command_line) {
  DCHECK(command_line[command_line.length() - 1] != '\0',
         "Command line must have a null terminator at the end");

  auto tokens = TokenizeCommandLine(command_line);

  // yes the c functions only take a char* ptr even though the encoding can be
  // anything...
  base::Vector<char*> pointers(tokens.size() + 1,
                               base::VectorReservePolicy::kForPushback);
  for (const base::StringU8& token : tokens) {
    pointers.push_back(const_cast<char*>(reinterpret_cast<const char*>(token.c_str())));
  }
  pointers.push_back(nullptr);  // exec() expects a nullptr at the end

  pid_t pid = fork();
  if (pid == -1) {
    // Forking failed
    return false;
  } else if (pid == 0) {
    // We're in the child process
    execvp(reinterpret_cast<const char*>(path_to_executable.c_str()), pointers.data());
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