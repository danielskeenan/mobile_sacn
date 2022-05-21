#! /usr/bin/python3
# Create the release manifest used by the website to show release info.
import argparse
import json
import re
import shutil
import subprocess
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Optional, Any


@dataclass
class AssetInfo:
    filename: str
    size: int
    kind: str
    platform: str
    version: str
    dsa: Optional[str]

    @staticmethod
    def for_file(path: Path):
        if match := re.match(r'^.+[_-](?P<version>[\d.]+)[_-](?:Linux\.tar\.gz|\w+\.deb)$', path.name):
            # Linux
            if path.suffix == '.deb':
                kind = 'Debian Package'
            else:
                kind = 'Tarball'
            return AssetInfo(path.name, path.stat().st_size, kind, 'ubuntu', match.group('version'), None)
        elif match := re.match(r'^.+-(?P<version>[\d.]+)-Windows\.(?:msi|zip)$', path.name):
            # Windows
            if path.suffix == '.msi':
                kind = 'Installer'
            else:
                kind = 'Portable'
            return AssetInfo(path.name, path.stat().st_size, kind, 'windows', match.group('version'), None)
        elif match := re.match(r'^.+-(?P<version>[\d.]+)-Darwin\.dmg$', path.name):
            return AssetInfo(path.name, path.stat().st_size, 'Disk Image', 'macos', match.group('version'), None)

        return None

    def to_dict(self) -> dict:
        return {
            'filename': self.filename,
            'size': self.size,
            'kind': self.kind,
            'platform': self.platform,
            'dsa': self.dsa,
        }


@dataclass
class ReleaseInfo:
    title: str
    version: str
    published: datetime
    channel: str
    assets: list[AssetInfo]

    def to_dict(self) -> dict:
        return {
            'title': self.title,
            'version': self.version,
            'published': self.published.isoformat(),
            'channel': self.channel,
            'assets': self.assets,
        }


def main():
    argparser = argparse.ArgumentParser(description='Create the release manifest used by the website to show release '
                                                    'info.')
    argparser.add_argument('title', type=str, help='Release title.')
    argparser.add_argument('channel', type=str, help='Release channel.')
    argparser.add_argument('--dsa_key', type=str, required=True,
                           help='Path to private key for DSA signature in PEM format.')
    argparser.add_argument('output', type=argparse.FileType('wt'), help='Path to created manifest file.')
    args = argparser.parse_args()

    assets = []
    for file in Path('.').iterdir():
        asset = AssetInfo.for_file(file)
        if asset is None:
            continue
        asset.dsa = get_dsa_signature(args.dsa_key, file)
        assets.append(asset)
    if len(assets) == 0:
        raise RuntimeError('No assets found!')
    release = ReleaseInfo(args.title, assets[0].version, datetime.now(timezone.utc), args.channel,
                          assets)
    json.dump(release, args.output, ensure_ascii=True, cls=ReleaseManifestJsonEncoder)


def get_dsa_signature(private_key_path: str, file_path: Path) -> str:
    """
    Follows the same process as the sign_update script from Sparkle.
    :param private_key_path:
    :param file_path:
    :return:
    """
    openssl = shutil.which('openssl')
    if openssl is None:
        raise RuntimeError('OpenSSL must be installed and available on the PATH.')

    openssl_digest = subprocess.run([openssl, 'dgst', '-sha1', '-binary', file_path.absolute()], capture_output=True,
                                    text=False, check=True)
    openssl_sign = subprocess.run([openssl, 'dgst', '-sha1', '-sign', private_key_path],
                                  input=openssl_digest.stdout, capture_output=True, text=False, check=True)
    openssl_enc = subprocess.run([openssl, 'enc', '-base64'], input=openssl_sign.stdout, capture_output=True,
                                 check=True)
    return str(openssl_enc.stdout, encoding='ascii')


class ReleaseManifestJsonEncoder(json.JSONEncoder):
    """
    JSON Encoder that knows how to encode release manifest objects.
    """

    def default(self, o: Any) -> Any:
        if isinstance(o, ReleaseInfo):
            return o.to_dict()
        elif isinstance(o, AssetInfo):
            return o.to_dict()
        return super().default(o)


if __name__ == '__main__':
    main()
