// Copyright (c) 2020 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <semver/semver.hh>
#include <string>

namespace semver {
	std::string component::to_string() const {
		if (holds_number()) return std::to_string(get_number());
		auto view = get_alnum();
		return {view.data(), view.size()};
	}
	std::string component_list::to_string() const {
		auto pre = '-';
		std::string result{};
		for (auto const& comp : *this) {
			result.push_back(pre);
			result += comp.to_string();
			pre = '.';
		}
		return result;
	}

	std::string version_view::to_string() const {
		auto result = std::to_string(major) + "." + std::to_string(minor) +
		              "." + std::to_string(patch) + prerelease.to_string();
		if (!build.empty()) {
			result.reserve(result.size() + build.size() + 1);
			result.push_back('+');
			result.append(build);
		}
		return result;
	}

	std::string project_version::to_string() const {
		return semver_.to_string();
	}
}  // namespace semver