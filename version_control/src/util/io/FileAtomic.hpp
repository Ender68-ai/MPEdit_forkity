#pragma once

#include <filesystem>

namespace git_editor {



bool replaceFileAtomic(std::filesystem::path const& from, std::filesystem::path const& to);

} // namespace git_editor
