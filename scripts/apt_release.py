#! /usr/bin/python3
# Create the apt repo Release file.

import enum
import hashlib
import os
import shutil
import subprocess
import sys
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from tempfile import NamedTemporaryFile
from typing import Iterator


class ChecksumAlgorithm(enum.Enum):
    MD5 = ('MD5Sum', hashlib.md5)
    SHA1 = ('SHA1', hashlib.sha1)
    SHA256 = ('SHA256', hashlib.sha256)

    def __init__(self, label: str, hasher):
        """
        A checksum algorithm.
        :param label: The label to use in the Release file
        :param hasher: Hash function.  Should follow the hashlib interface,
        """
        self.label = label
        self.hasher = hasher


@dataclass
class FileInfo:
    path: Path
    size: int
    checksum: str


def get_fileinfo(path: Path, algorithm: ChecksumAlgorithm) -> Iterator[FileInfo]:
    for item in path.iterdir():
        if item.is_dir():
            yield from get_fileinfo(item, algorithm)
        elif item.is_file() and item.name != 'Release':
            # Calculate checksum
            with item.open('rb') as file:
                hasher = algorithm.hasher()
                while item_bytes := file.read1():
                    hasher.update(item_bytes)
                yield FileInfo(item, item.stat().st_size, hasher.hexdigest())


def main():
    if len(sys.argv) != 2:
        print('Usage: {} <signing key id>'.format(sys.argv[0]), file=sys.stderr)
        exit(1)

    # Iterate over each section.
    dists_dir = Path('./dists')
    if not dists_dir.is_dir():
        print('dists directory does not exist.  Run this script from the root of the apt repository.', file=sys.stderr)
        exit(1)

    for suite_dir in filter(lambda p: p.is_dir(), dists_dir.iterdir()):
        # Use a temporary file to avoid incrementally writing to a file on S3 storage.
        release_file = NamedTemporaryFile('wt', delete=False)
        components = []
        for component_dir in filter(lambda p: p.is_dir(), suite_dir.iterdir()):
            components.append(component_dir.stem)

        release_file.writelines([
            'Origin: Mobile sACN Apt Repository\n',
            'Label: Mobile sACN Apt Repository\n',
            'Suite: {}\n'.format(suite_dir.stem),
            'Architecture: amd64\n',
            'Description: Mobile sACN Apt Repository\n',
            'Date: {}\n'.format(datetime.now(timezone.utc).strftime('%a, %d %b %Y %H:%M:%S %Z')),
            'Components: {}\n'.format(' '.join(components))
        ])
        for checksum_algo in ChecksumAlgorithm:
            release_file.write('{}:\n'.format(checksum_algo.label))
            for fileinfo in get_fileinfo(suite_dir, checksum_algo):
                release_file.write('\t{}\t{}\t{}\n'.format(fileinfo.checksum,
                                                           fileinfo.size,
                                                           fileinfo.path.relative_to(suite_dir)))

        release_file.flush()
        release_gpg_file = NamedTemporaryFile('wt', delete=False, encoding='ascii')
        release_file.seek(0)
        subprocess.run(['gpg', '--default-key', sys.argv[1], '--armor', '--sign', '--detach-sign'],
                       stdin=release_file, stdout=release_gpg_file, check=True)
        inrelease_file = NamedTemporaryFile('wt', delete=False, encoding='ascii')
        release_file.seek(0)
        subprocess.run(['gpg', '--default-key', sys.argv[1], '--armor', '--sign', '--detach-sign', '--clearsign'],
                       stdin=release_file, stdout=inrelease_file, check=True)
        release_file.close()
        release_gpg_file.close()
        inrelease_file.close()
        shutil.copy(release_file.name, suite_dir / 'Release')
        os.unlink(release_file.name)
        shutil.copy(release_gpg_file.name, suite_dir / 'Release.gpg')
        os.unlink(release_gpg_file.name)
        shutil.copy(inrelease_file.name, suite_dir / 'InRelease')
        os.unlink(inrelease_file.name)


if __name__ == '__main__':
    main()
