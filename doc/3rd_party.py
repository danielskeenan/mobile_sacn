import json
import re
import shutil
import subprocess
import sys
from argparse import ArgumentParser
from dataclasses import dataclass
from pathlib import Path
from shutil import unpack_archive
from tempfile import TemporaryDirectory
from typing import TextIO, Optional
from urllib.parse import urlsplit
from urllib.request import urlretrieve

from natsort import natsorted

SPDX_FIELDNOTPRESENT = frozenset(("NOASSERTION", "NONE"))


@dataclass
class PackageInfo:
    name: str
    homepage: Optional[str] = None
    license_text: Optional[str] = None


class LicenseFileFinder:
    """
    Find the project's license file.

    Port of https://github.com/licensee/licensee/blob/master/lib/licensee/project_files/license_file.rb
    """

    # List of extensions to give preference to
    PREFERRED_EXT_REGEX = r"\.(md|markdown|txt|html)\z"

    # Regex to match any extension except .spdx or .header
    LICENSE_EXT_REGEX = r"\.(?!spdx|header)[^./]+"

    # Regex to match any extension except a few unlikely as license texts with complex filenames
    OTHER_EXT_REGEX = r"\.(?!xml|go|gemspec)[^./]+"

    # Regex to match any extension
    ANY_EXT_REGEX = r"\.[^./]+"

    # Regex to match, LICENSE, LICENCE, unlicense, etc.
    LICENSE_REGEX = r"(un)?licen[sc]e"

    # Regex to match COPYING, COPYRIGHT, etc.
    COPYING_REGEX = r"copy(ing|right)"

    # Regex to match OFL.
    OFL_REGEX = r"ofl"

    # BSD + PATENTS patent file
    PATENTS_REGEX = r"patents"

    # List of Regex, score pairs with which to score potential license files
    FILENAME_REGEXES = (
        (re.compile(f"\\A{LICENSE_REGEX}\\z", re.IGNORECASE), 1.00),  # LICENSE
        (re.compile(f"\\A{LICENSE_REGEX}{PREFERRED_EXT_REGEX}\\z", re.IGNORECASE), 0.95),  # LICENSE.md
        (re.compile(f"\\A{COPYING_REGEX}\\z", re.IGNORECASE), 0.90),  # COPYING
        (re.compile(f"\\A{COPYING_REGEX}{PREFERRED_EXT_REGEX}\\z", re.IGNORECASE), 0.85),  # COPYING.md
        (re.compile(f"\\A{LICENSE_REGEX}{LICENSE_EXT_REGEX}\\z", re.IGNORECASE), 0.80),  # LICENSE.textile
        (re.compile(f"\\A{COPYING_REGEX}{ANY_EXT_REGEX}\\z", re.IGNORECASE), 0.75),  # COPYING.textile
        (re.compile(f"\\A{LICENSE_REGEX}[-_][^.]*{OTHER_EXT_REGEX}?\\z", re.IGNORECASE), 0.70),  # LICENSE-MIT
        (re.compile(f"\\A{COPYING_REGEX}[-_][^.]*{OTHER_EXT_REGEX}?\\z", re.IGNORECASE), 0.65),  # COPYING-MIT
        (re.compile(f"\\A\\w+[-_]{LICENSE_REGEX}[^.]*{OTHER_EXT_REGEX}?\\z", re.IGNORECASE), 0.60),  # MIT-LICENSE-MIT
        (re.compile(f"\\A\\w+[-_]{COPYING_REGEX}[^.]*{OTHER_EXT_REGEX}?\\z", re.IGNORECASE), 0.55),  # MIT-COPYING
        (re.compile(f"\\A{OFL_REGEX}{PREFERRED_EXT_REGEX}", re.IGNORECASE), 0.50),  # OFL.md
        (re.compile(f"\\A{OFL_REGEX}{OTHER_EXT_REGEX}", re.IGNORECASE), 0.45),  # OFL.textile
        (re.compile(f"\\A{OFL_REGEX}\\z", re.IGNORECASE), 0.40),  # OFL
        (re.compile(f"\\A{PATENTS_REGEX}\\z", re.IGNORECASE), 0.35),  # PATENTS
        (re.compile(f"\\A{PATENTS_REGEX}{OTHER_EXT_REGEX}\\z", re.IGNORECASE), 0.30),  # PATENTS.txt
    )

    @classmethod
    def find_license_path(cls, search_path: Path) -> Optional[Path]:
        high_score = 0
        likely_path = None
        for (dirpath, dirnames, filenames) in search_path.walk(top_down=True):
            for filename in filenames:
                for regex, score in cls.FILENAME_REGEXES:
                    if re.search(regex, filename) is not None and score > high_score:
                        high_score = score
                        likely_path = Path(dirpath) / filename
        if high_score > 0:
            return likely_path
        return None


def get_spdx_packages(sbom_path: Path) -> list[PackageInfo]:
    with sbom_path.open("rt") as sbom_file:
        sbom = json.load(sbom_file)

    packages: list[PackageInfo] = []
    for package in sbom["packages"]:
        if "primaryPackagePurpose" in package and package["primaryPackagePurpose"] == "APPLICATION":
            # Skip this program.
            continue

        print(f"Working on {package["name"]}")
        package_info = PackageInfo(package["name"])

        # Homepage
        if "homepage" in package and package["homepage"] not in SPDX_FIELDNOTPRESENT:
            package_info.homepage = package["homepage"]

        # License
        # Check for a local override for the license - some packages (e.g. Qt) have special requirements that need
        # to be assembled manually.
        license_path = (sbom_path.parent / f"{package["name"]}-license.txt")
        if license_path.is_file():
            with license_path.open("rt") as license_file:
                package_info.license_text = license_file.read()
        elif "downloadLocation" in package and package["downloadLocation"] not in SPDX_FIELDNOTPRESENT:
            download_url = urlsplit(package["downloadLocation"])
            path = Path(download_url[2])
            with TemporaryDirectory(prefix="3rdParty") as dl_dir:
                try:
                    dl_path = Path(dl_dir) / f"archive{path.suffix}"
                    urlretrieve(package["downloadLocation"], filename=dl_path)
                    with TemporaryDirectory() as extract_dir:
                        unpack_archive(dl_path, extract_dir)
                        license_path = LicenseFileFinder.find_license_path(Path(extract_dir))
                        if license_path is not None:
                            with license_path.open("rt") as license_file:
                                package_info.license_text = license_file.read()
                        else:
                            print(f"Could not find license file for {package["name"]}")
                except RuntimeError:
                    print(f"Error downloading package {package["name"]}")
        else:
            print(f"No download available for {package["name"]}")

        packages.append(package_info)
    return packages


def get_npm_packages(npm_project_root: Path) -> list[PackageInfo]:
    # Ask npm about production deps.
    print("Loading npm packages...")
    npm = shutil.which("npm")
    if npm is None:
        raise RuntimeError("Cannot find npm")
    npm_sbom_result = subprocess.run([
        npm, "sbom",
        "--omit", "dev",
        "--sbom-format", "spdx",
        "--sbom-type", "application",
    ], cwd=npm_project_root, stderr=sys.stderr, stdout=subprocess.PIPE, check=True, encoding="UTF-8")
    sbom = json.loads(npm_sbom_result.stdout)

    packages = []
    for package in sbom["packages"]:
        if "primaryPackagePurpose" in package and package["primaryPackagePurpose"] == "APPLICATION":
            # Skip this program.
            continue
        package_info = PackageInfo(package["name"])

        # Homepage
        if "homepage" in package and package["homepage"] not in SPDX_FIELDNOTPRESENT:
            package_info.homepage = package["homepage"]

        # License
        package_dir = npm_project_root / "node_modules" / package["name"]
        if not package_dir.is_dir():
            raise RuntimeError(f"Could not find installed dir for {package["name"]}")
        license_path = LicenseFileFinder.find_license_path(Path(package_dir))
        if license_path is not None:
            with license_path.open("rt") as license_file:
                package_info.license_text = license_file.read()
        else:
            print(f"Could not find license file for {package["name"]}")

        packages.append(package_info)

    return packages


SKIP_PATTERNS = (
    re.compile(r"-?vcpkg-?", re.IGNORECASE),
    re.compile(r"-?cmake-?", re.IGNORECASE),
)


def should_skip_vcpkg_package(package_name: str):
    for skip_pattern in SKIP_PATTERNS:
        if re.search(skip_pattern, package_name) is not None:
            return True
    return False


def get_vcpkg_packages(vcpkg_install_root: Path) -> list[PackageInfo]:
    print("Loading vcpkg packages...")
    port_dirs = []
    status_path = vcpkg_install_root / "vcpkg" / "status"
    with status_path.open("rt") as status_file:
        package_name = ""
        package_arch = ""
        for line in status_file:
            if (match := re.match(r"Package: (.+)", line)) is not None:
                package_name = match.group(1)
            elif (match := re.match(r"Architecture: (.+)", line)) is not None:
                package_arch = match.group(1)
            elif line == "\n":
                if should_skip_vcpkg_package(package_name):
                    # Skip vcpkg helpers.
                    continue
                port_dir = vcpkg_install_root / package_arch / "share" / package_name
                port_dirs.append(port_dir)

    packages = []
    for port_dir in port_dirs:
        if (port_dir / "vcpkg.spdx.json").is_file() and (port_dir / "copyright").is_file():
            with (port_dir / "vcpkg.spdx.json").open("rt") as sbom_file:
                sbom = json.load(sbom_file)
                name: str = sbom["name"]
                name = name[:name.find(":")]
                package_info = PackageInfo(name)
                # The package entry with the homepage is the real package.
                for subpackage in sbom["packages"]:
                    if "homepage" in subpackage:
                        package_info.homepage = subpackage["homepage"]
                    break
            with (port_dir / "copyright").open("rt") as copyright_file:
                package_info.license_text = copyright_file.read()
            packages.append(package_info)

    return packages


def write_header(out: TextIO):
    out.write(
        "# 3rd Party Software\n"
        "\n"
        "The following 3rd party software is included in this project.\n"
        "\n"
    )


def write_packages(packages: list[PackageInfo], out: TextIO):
    for package in packages:
        out.write(
            f"## {package.name}\n"
            "\n"
        )
        if package.homepage is not None:
            out.write(
                f"[{package.homepage}]({package.homepage})\n"
                "\n"
            )
        if package.license_text is not None:
            out.write(
                "``````````text\n"
                f"{package.license_text}\n"
                "``````````\n"
                "\n"
            )


def dedupe_packages(packages: list[PackageInfo]):
    package_names = set()
    package_ix = 0
    while package_ix < len(packages):
        package = packages[package_ix]
        if package.name in package_names:
            del packages[package_ix]
        else:
            package_names.add(package.name)
            package_ix += 1

def main():
    parser = ArgumentParser(description="Generate a page of 3rd party software with links and their licenses.")
    parser.add_argument("-o", "--output", required=True, type=lambda p: Path(p), help="Path to output file")
    parser.add_argument("--vcpkg_install_root", type=lambda p: Path(p),
                        help="Path to vcpkg package install root to search for SBOMs")
    parser.add_argument("--npm_project_root", type=lambda p: Path(p),
                        help="Path to a directory containing package.json and node_modules.")
    parser.add_argument("spdx", nargs="*", type=lambda p: Path(p), help="Input SPDX file(s)")
    args = parser.parse_args()

    with args.output.open("wt") as out:
        write_header(out)
        packages: list[PackageInfo] = []
        for sbom_path in args.spdx:
            packages.extend(get_spdx_packages(sbom_path))
        if args.vcpkg_install_root:
            packages.extend(get_vcpkg_packages(args.vcpkg_install_root))
        if args.npm_project_root:
            packages.extend(get_npm_packages(args.npm_project_root))

        dedupe_packages(packages)
        packages = natsorted(packages, key=lambda p: p.name.casefold())
        write_packages(packages, out)


if __name__ == "__main__":
    main()
