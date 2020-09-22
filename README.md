# SemVer
[![Cpp Standard](https://img.shields.io/badge/C%2B%2B-17-informational?style=for-the-badge)](https://en.wikipedia.org/wiki/C%2B%2B17)

Semantic Versioning 2.0 parser in pure C++17. Useful for generating compile-time version, for instance based on CMake project version info.

Example `CMakeLists.txt`:
```cmake
project(foo
  VERSION 6.2.15
  LANGUAGES CXX)

set(PROJECT_VERSION_STABILITY "-rc.3")

# ...

configure_file(version.hh.in include/foo/version.hh @ONLY)
```

Example `version.hh.in`:
```cxx
#include <semver/semver.hh>

namespace foo {
	inline constexpr semver::project_version version{
		"@PROJECT_VERSION_MAJOR@."
		"@PROJECT_VERSION_MINOR@."
		"@PROJECT_VERSION_PATCH@"
		"@PROJECT_VERSION_STABILITY@"
	};
	semver::project_version get_version() noexcept;
}
```
