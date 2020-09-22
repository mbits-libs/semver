// Copyright (c) 2020 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

// #include <charconv>
// #include <optional>
#include <string_view>
#include <utility>
#include <variant>

// Source: https://semver.org/spec/v2.0.0.html
namespace semver {
	class component {
	public:
		constexpr component() noexcept = default;
		~component() noexcept = default;
		constexpr component(unsigned val) noexcept
		    : value_{std::in_place_type<unsigned>, val} {}
		constexpr component(std::string_view val) noexcept
		    : value_{std::in_place_type<std::string_view>, val} {}

		static constexpr component parse(std::string_view value) noexcept {
			if (value.empty()) return {};

			unsigned result{};
			for (auto c : value) {
				switch (c) {
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						result *= 10;
						result += c - '0';
						break;
					default:
						return value;
				}
			}
			return result;
		}

		std::string to_string() const;

		constexpr bool empty() const noexcept {
			if (holds_number()) return false;
			return get_alnum().empty();
		}

		constexpr bool operator<(component const& right) const noexcept {
			// 11.4.3 Numeric identifiers always have lower precedence than
			//        non-numeric identifiers.
			if (holds_number() != right.holds_number()) return holds_number();

			// 11.4.1 Identifiers consisting of only digits are compared
			//        numerically.
			if (holds_number()) return get_number() < right.get_number();

			// 11.4.2 Identifiers with letters or hyphens are compared lexically
			//        in ASCII sort order.
			return get_alnum() < right.get_alnum();
		}

		constexpr bool operator==(component const& right) const noexcept {
			return value_ == right.value_;
		}

		constexpr bool operator!=(component const& right) const noexcept {
			return value_ != right.value_;
		}

		constexpr bool operator>(component const& right) const noexcept {
			return right < *this;
		}

		constexpr bool operator<=(component const& right) const noexcept {
			return !(right < *this);
		}

		constexpr bool operator>=(component const& right) const noexcept {
			return !(*this < right);
		}

		constexpr bool holds_number() const noexcept {
			return std::holds_alternative<unsigned>(value_);
		}

		constexpr bool get_number() const noexcept {
			return std::get<unsigned>(value_);
		}

		constexpr std::string_view get_alnum() const noexcept {
			return std::get<std::string_view>(value_);
		}

	private:
		std::variant<std::string_view, unsigned> value_{};
	};

	class component_list {
	public:
		using value_type = component;

		constexpr component_list() noexcept = default;
		constexpr component_list(value_type const* list, size_t count) noexcept
		    : begin_{list}, end_{list + count} {}
		constexpr component_list(
		    std::initializer_list<value_type> braces) noexcept
		    : component_list{braces.begin(), braces.size()} {}

		std::string to_string() const;

		constexpr size_t size() const noexcept {
			return static_cast<size_t>(end_ - begin_);
		}

		constexpr bool empty() const noexcept { return end_ == begin_; }

		constexpr auto begin() const noexcept { return begin_; }

		constexpr auto end() const noexcept { return end_; }

		constexpr bool operator<(component_list const& right) const noexcept {
			auto const left_size = size();
			auto const right_size = right.size();

			// 11.3 When major, minor, and patch are equal, a pre-release
			//      version has lower precedence than a normal version
			if ((left_size == 0) != (right_size == 0)) return right_size == 0;

			// 11.4 Precedence for two pre-release versions with the same major,
			//      minor, and patch version MUST be determined by comparing
			//      each dot separated identifier from left to right until a
			//      difference is found
			auto left_it = begin(), right_it = right.begin(),
			     left_end = begin() + std::min(left_size, right_size);
			for (; left_it != left_end; ++left_it, ++right_it) {
				if (*left_it != *right_it) return *left_it < *right_it;
			}

			// 11.4.3 A larger set of pre-release fields has a higher precedence
			//        than a smaller set, if all of the preceding identifiers
			//        are equal.
			return left_size < right_size;
		}

		constexpr bool operator==(component_list const& right) const noexcept {
			if (size() != right.size()) return false;

			auto left_it = begin(), right_it = right.begin(), left_end = end();
			for (; left_it != left_end; ++left_it, ++right_it) {
				if (*left_it != *right_it) return false;
			}

			return true;
		}

		constexpr bool operator!=(component_list const& right) const noexcept {
			return !(*this == right);
		}

		constexpr bool operator>(component_list const& right) const noexcept {
			return right < *this;
		}

		constexpr bool operator<=(component_list const& right) const noexcept {
			return !(right < *this);
		}

		constexpr bool operator>=(component_list const& right) const noexcept {
			return !(*this < right);
		}

	private:
		value_type const* begin_{};
		value_type const* end_{};
	};

	struct version_view {
		unsigned major{};
		unsigned minor{};
		unsigned patch{};
		component_list prerelease{};
		std::string_view build{};

		std::string to_string() const;

		constexpr bool is_public() const noexcept {
			// 5. Version 1.0.0 defines the public API.
			return !!major;
		}

		constexpr bool operator<(version_view const& right) const noexcept {
			// 11. Precedence refers to how versions are compared to each other
			// when ordered.
			//   1 Precedence MUST be calculated by separating the version into
			//     major, minor, patch and pre-release identifiers in that order
			//     (Build metadata does not figure into precedence).
			//   2 Precedence is determined by the first difference when
			//     comparing each of these identifiers from left to right as
			//     follows: Major, minor, and patch versions are always compared
			//     numerically.

			if (major != right.major) return major < right.major;
			if (minor != right.minor) return minor < right.minor;
			if (patch != right.patch) return major < right.patch;

			return prerelease < right.prerelease;
		}

		constexpr bool operator==(version_view const& right) const noexcept {
			return major == right.major && minor == right.minor &&
			       patch == right.patch && prerelease == right.prerelease;
			// 11.1 [...] (Build metadata does not figure into precedence).
		}

		constexpr bool operator!=(version_view const& right) const noexcept {
			return !(*this == right);
		}

		constexpr bool operator>(version_view const& right) const noexcept {
			return right < *this;
		}

		constexpr bool operator<=(version_view const& right) const noexcept {
			return !(right < *this);
		}

		constexpr bool operator>=(version_view const& right) const noexcept {
			return !(*this < right);
		}

		constexpr bool compatible_with(
		    version_view const& runtime) const noexcept {
			// 8. Major version X (X.y.z | X > 0) MUST be incremented if any
			//    backwards incompatible changes are introduced to the public
			//    API.
			//
			// SELF: this and point 4. indicate any change in major versions
			//       means incompatibility.
			if (major != runtime.major) return false;

			// 6. Patch version Z (x.y.Z | x > 0) MUST be incremented if only
			//    backwards compatible bug fixes are introduced. A bug fix is
			//    defined as an internal change that fixes incorrect behavior.
			//
			// 7. Minor version Y (x.Y.z | x > 0) MUST be incremented if new,
			//    backwards compatible functionality is introduced to the public
			//    API. It MUST be incremented if any public API functionality is
			//    marked as deprecated. It MAY be incremented if substantial new
			//    functionality or improvements are introduced within the
			//    private code. It MAY include patch level changes. Patch
			//    version MUST be reset to 0 when minor version is incremented.
			//
			// SELF: The above is taken to mean, that if runtime version is
			//       smaller, than checked version there may be expectations,
			//       which runtime version may not be able to meet.
			if (major) return *this <= runtime;

			// 4. Major version zero (0.y.z) is for initial development.
			//    Anything MAY change at any time. The public API SHOULD NOT be
			//    considered stable.
			// SELF: only the same version guarantees compatibility.
			return *this == runtime;
		}
	};

	struct project_version {
		constexpr project_version(std::string_view version) noexcept
		    : project_version{strings{version}} {}

		std::string to_string() const;

		constexpr bool operator<(project_version const& right) const noexcept {
			return semver_ < right.semver_;
		}

		constexpr bool operator==(project_version const& right) const noexcept {
			return semver_ == right.semver_;
		}

		constexpr bool operator!=(project_version const& right) const noexcept {
			return semver_ != right.semver_;
		}

		constexpr bool operator>(project_version const& right) const noexcept {
			return semver_ > right.semver_;
		}

		constexpr bool operator<=(project_version const& right) const noexcept {
			return semver_ <= right.semver_;
		}

		constexpr bool operator>=(project_version const& right) const noexcept {
			return semver_ >= right.semver_;
		}

		constexpr bool compatible_with(
		    project_version const& runtime) const noexcept {
			return semver_.compatible_with(runtime.semver_);
		}

	private:
		struct strings {
			std::string_view major;
			std::string_view minor;
			std::string_view patch;
			std::string_view prerelease;
			std::string_view counter;
			std::string_view build;
			constexpr explicit strings(std::string_view version) noexcept {
				auto npos = std::string_view::npos;

				auto sep = version.find('+');
				if (sep != npos) {
					build = version.substr(sep + 1);
					version = version.substr(0, sep);
				}

				sep = version.find('-');
				if (sep != npos) {
					prerelease = version.substr(sep + 1);
					major = version.substr(0, sep);
				} else {
					major = version;
				}

				sep = prerelease.find('.');
				if (sep != npos) {
					counter = prerelease.substr(sep + 1);
					prerelease = prerelease.substr(0, sep);
				}

				sep = major.find('.');
				if (sep != npos) {
					minor = major.substr(sep + 1);
					major = major.substr(0, sep);
				}

				sep = minor.find('.');
				if (sep != npos) {
					patch = minor.substr(sep + 1);
					minor = minor.substr(0, sep);
				}

				sep = patch.find('.');
				if (sep != npos) {
					patch = patch.substr(0, sep);
				}
			}
		};

		constexpr project_version(strings const& broken) noexcept
		    : prerelease_{{component::parse(broken.prerelease),
		                   component::parse(broken.counter)}}
		    , semver_{parse(broken.major), parse(broken.minor),
		              parse(broken.patch), prerelease_.to_list(),
		              broken.build} {}

		static constexpr unsigned parse(std::string_view value) noexcept {
			unsigned result{};
			for (auto c : value) {
				switch (c) {
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						result *= 10;
						result += c - '0';
						break;
					default:
						return result;
				}
			}
			return result;
		}

		struct prerelase_storage {
			component items[2]{};

			constexpr component_list to_list() const noexcept {
				if (items[0].empty()) return {};
				if (items[1].empty()) return {items, 1};
				return {items, 2};
			}
		};

		prerelase_storage prerelease_;
		version_view semver_;
	};
}  // namespace semver
