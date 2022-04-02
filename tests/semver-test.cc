#include <cstdio>
#include <semver/version.hh>
#include <string>

int main() {
	auto const ver_data = std::string{"1.3.0-beta.5+something.mixed.5"};
	semver::project_version ver{ver_data};

	char const* verdict = semver::version.compatible_with(ver) ? "" : "in";
	printf("Compiled-in version %s is %scompatible with runtime version %s\n",
	       semver::version.to_string().c_str(), verdict,
	       ver.to_string().c_str());
	verdict =
	    semver::version.compatible_with(semver::get_version()) ? "" : "in";
	printf("Compiled-in version %s is %scompatible with runtime version %s\n",
	       semver::version.to_string().c_str(), verdict,
	       semver::get_version().to_string().c_str());
}
