# U-Boot Legacy Image Support Component Declaration

## Component Information

- Name: U-Boot Legacy Image Support
- SBOM Version: Unknown
- Component Type: Library
- Scope: ProductView
- Integration: Vendored source
- Modification Status: Modified
- License: GPL-2.0-or-later
- Author: Wolfgang Denk
- Supplier: DENX Software Engineering

## Source Location

- `Loaders/SpiLoader_gzip/cmd_bootm.c`
- `Loaders/SpiLoader_gzip/ziplib/image.h`

## Identification Evidence

The source files contain the following identifiers:

- Wolfgang Denk
- DENX Software Engineering
- U-Boot
- U-Boot legacy image definitions
- `image_header_t`
- `IH_MAGIC`
- `do_bootm`

The implementation directly includes and uses the U-Boot legacy image
header interface.

## Version Evidence

No explicit upstream U-Boot release version was identified in the
vendored source files.

The source files contain copyright ranges but copyright years are not
used to infer a software release version.

The SBOM version is therefore recorded as:

- `Unknown`

## Modification Evidence

The U-Boot-derived boot support is adapted for the N9H20
`SpiLoader_gzip` implementation.

The adapted implementation directly integrates:

- zlib decompression
- bzip2 decompression
- U-Boot legacy image header parsing
- Compressed-image boot support

## License Evidence

The relevant source files contain GNU General Public License notices:

- `Loaders/SpiLoader_gzip/cmd_bootm.c`
- `Loaders/SpiLoader_gzip/ziplib/image.h`

The applicable SPDX license identifier is:

- `GPL-2.0-or-later`

The `image.h` notice also states that including the unmodified header
as an interface to U-Boot is considered normal use and does not by
itself constitute a derived work.

## Usage

The component provides legacy image-header parsing and compressed-image
boot support for:

- `Loaders/SpiLoader_gzip`

## SBOM Mapping

- SCA Definition: `Documents/SBOM/components/ProductView/sca_uboot_image.json`
- Package URL: `pkg:generic/u-boot-legacy-image-support@unknown`
- Component Origin: Third-party
- Required Scope: ProductView

## Review Result

- Classification: Third-Party OSS
- Status: Approved