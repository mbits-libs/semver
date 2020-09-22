// Copyright (c) 2020 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <semver/version.hh>
#include <string>

namespace semver {
	semver::project_version get_version() noexcept { return version; }
}
