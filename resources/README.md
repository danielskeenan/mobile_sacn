# Logo asset notes

Logos are in a couple formats:

- `logo_source.svg`
    - Source SVG for use in Inkscape. All other formats derive from this.
    - The "1.31" text is in [BPreplay Normal](https://www.dafont.com/bpreplay.font)
- `logo.svg`
    - Export as Plain SVG from Inkscape, then run through [svgo](https://github.com/svg/svgo).
- `logo.ico`
    - Open the svg in Gimp in these sizes:
        - 512x512
        - 256x256
        - 128x128
        - 96x96
        - 64x64
        - 48x48
        - 32x32
        - 24x24
        - 16x16
- `logo_maskable.svg`
    - Manually tweaked from `logo.svg` to move content into a 40% radius to
      permit [icon masking](https://web.dev/learn/pwa/web-app-manifest/#maskable_icons) on some platforms.

Web UI:

- `mobile_sacn.svg`, `mobile_sacn_maskable.svg`, and `favicon.ico`
    - Copied by CMake from `logo.svg`, `logo_maskable.svg`, and `logo.ico` respectively. See above for their contents.
    - Referenced in `site.webmanifest`.
- `android-chrome-###x###.png`
    - Export from Inkscape as png in those sizes. Android wants at least 2 icon sizes even though we already give it
      SVG.
  - Referenced in `site.webmanifest`.
- `apple-touch-icon.png`
    - Export from Inkscape as png 180x180
      per [Apple specs](https://developer.apple.com/library/archive/documentation/AppleApplications/Reference/SafariWebContent/ConfiguringWebApplications/ConfiguringWebApplications.html).
    - Referenced in `index.html`.
- `favicon-##x##.png`
  - Export from Inkscape as png in those sizes.
  - Referenced in `index.html`.
