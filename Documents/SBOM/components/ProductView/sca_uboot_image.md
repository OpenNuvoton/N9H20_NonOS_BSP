# U-Boot Legacy Image Support Component Declaration

## Component Information
- Name: U-Boot Legacy Image Support
- SBOM Version: Unknown
- Component Type: Library
- Scope: ProductView
- Integration: Modified vendored source
- License: GPL-2.0-or-later
- Author: Wolfgang Denk
- Supplier: DENX Software Engineering

## Source Location
- Loaders/SpiLoader_gzip/cmd_bootm.c
- Loaders/SpiLoader_gzip/ziplib/image.h

## Identification Evidence
The source files contain identifiers for Wolfgang Denk, DENX Software Engineering, U-Boot, U-Boot legacy image definitions, `image_header_t`, `IH_MAGIC`, and `do_bootm`. The implementation includes and uses the U-Boot legacy image header interface.

## Version Evidence
No explicit upstream U-Boot release version was identified in the vendored source files. Copyright years are not used to infer a software release version. The SBOM version is therefore recorded as `Unknown`.

## Modification Evidence
The U-Boot-derived boot support is adapted for the N9H20 `SpiLoader_gzip` implementation. The adapted source provides U-Boot legacy image-header parsing and compressed-image boot support.

Source-level compatibility with zlib and bzip2 decompression is present in the vendored source set. Inclusion of the bzip2 runtime in the inspected default GCC and KEIL project configurations was not demonstrated. The observed default runtime decompression path uses zlib. Source capability is recorded separately from default-build inclusion.

## License Evidence
The relevant source files contain GNU General Public License notices:
- Loaders/SpiLoader_gzip/cmd_bootm.c
- Loaders/SpiLoader_gzip/ziplib/image.h

The applicable SPDX license identifier is `GPL-2.0-or-later`. The `image.h` notice also states that including the unmodified header as an interface to U-Boot is considered normal use and does not by itself constitute a derived work.

## Usage
The component provides legacy image-header parsing and compressed-image boot support for:
- Loaders/SpiLoader_gzip

## SBOM Mapping
- SCA Definition: Documents/SBOM/components/ProductView/sca_uboot_image.json
- Evidence: Documents/SBOM/evidence/evidence_uboot_image.md
- Package URL: pkg:generic/u-boot-legacy-image-support@unknown
- Component Origin: Third-party
- Required Scope: ProductView

## Review Result
- Classification: Third-Party OSS
- Component Identification: Approved
- Upstream Version: Unknown with documented justification
- Product Security Approval: Pending for the associated bzip2 vulnerability dispositions
