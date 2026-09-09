# U-Boot Legacy Image Support Evidence

## Component Identification
- Component: U-Boot Legacy Image Support
- SBOM Version: Unknown
- Scope: ProductView
- Classification: Third-Party OSS
- Integration: Modified vendored source
- License: GPL-2.0-or-later
- Author: Wolfgang Denk
- Supplier: DENX Software Engineering

## Evidence Location
- Loaders/SpiLoader_gzip/cmd_bootm.c
- Loaders/SpiLoader_gzip/ziplib/image.h

## Identification Evidence
The source files identify Wolfgang Denk, DENX Software Engineering, U-Boot, U-Boot legacy image support, `image_header_t`, `IH_MAGIC`, and `do_bootm`. The boot-support source includes the U-Boot legacy image header interface.

## Version Evidence
No explicit upstream U-Boot release version was found in the vendored source files. Copyright years are not used to infer a release version. The SBOM version is therefore recorded as `Unknown`.

## License Evidence
The relevant source files contain GNU General Public License notices. The applicable SPDX identifier is `GPL-2.0-or-later`. The `image.h` notice also states that including the unmodified header as an interface to U-Boot is considered normal use and does not by itself constitute a derived work.

## Usage and Build-Scope Evidence
The component provides U-Boot legacy-image header parsing and compressed-image boot support under `Loaders/SpiLoader_gzip`.

Source-level compatibility with zlib and bzip2 decompression is present in the vendored source set. Inclusion of the bzip2 runtime in the inspected default GCC and KEIL project configurations was not demonstrated. The observed default runtime decompression path uses zlib. Source capability is therefore recorded separately from default-build inclusion.

## SHA-256 Evidence
- cmd_bootm.c: E1022289B538E54312E1AE40A0CF62B39A43EEB15DF0055F91BFE4613210F1AC
- image.h: 136D319651BBEABBE0BA1C9ED52105EF5E09ED1F4CB3A24774A8A29C71EDE980

## SBOM Mapping
- SCA Definition: Documents/SBOM/components/ProductView/sca_uboot_image.json
- Component Declaration: Documents/SBOM/components/ProductView/sca_uboot_image.md
- Package URL: pkg:generic/u-boot-legacy-image-support@unknown

## Review Result
- Component Identification: Approved
- License Review: Approved
- Upstream Version: Unknown with documented justification
- Evidence Hash Revalidation: Required against the selected clean release commit
- Associated bzip2 Status: Known issues accepted/deferred for release follow-up;
  Product Security approval is not recorded and an approved VEX has not been
  issued
