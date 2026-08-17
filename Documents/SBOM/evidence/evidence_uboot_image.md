# U-Boot Legacy Image Support Evidence

## Component Identification

- Component: U-Boot Legacy Image Support
- SBOM Version: Unknown
- Scope: ProductView
- Classification: Third-Party OSS
- Integration: Vendored source
- Modification Status: Modified
- License: GPL-2.0-or-later
- Author: Wolfgang Denk
- Supplier: DENX Software Engineering

## Evidence Location

- `Loaders/SpiLoader_gzip/cmd_bootm.c`
- `Loaders/SpiLoader_gzip/ziplib/image.h`

## Identification Evidence

The source files identify:

- Wolfgang Denk
- DENX Software Engineering
- U-Boot
- U-Boot legacy image support

The implementation uses U-Boot legacy-image identifiers including:

- `image_header_t`
- `IH_MAGIC`
- `do_bootm`

The boot-support source directly includes `image.h`, `zlib.h`, and
`bzlib.h`.

## Version Evidence

No explicit upstream U-Boot release version was found in the vendored
source files.

Copyright years are not used to infer a release version. The SBOM
version is therefore recorded as `Unknown`.

## License Evidence

The relevant source files contain GNU General Public License notices:

- `Loaders/SpiLoader_gzip/cmd_bootm.c`
- `Loaders/SpiLoader_gzip/ziplib/image.h`

The applicable SPDX identifier is `GPL-2.0-or-later`.

The `image.h` notice also states that including the unmodified header
as an interface to U-Boot is considered normal use and does not by
itself constitute a derived work.

## Usage Evidence

The component provides:

- U-Boot legacy-image header parsing
- Compressed-image boot support
- Integration with zlib decompression
- Integration with bzip2 decompression

The integration is located under:

- `Loaders/SpiLoader_gzip`

## SHA-256 Evidence

- `cmd_bootm.c`: `C8E57121DF274114E8D739539CB0779889209103C86D32BF0C1B301257939605`
- `image.h`: `21DED2C41FBBAA11767B7D0122C88EE7BC00A733970126C9ADB615B1C94E2BE1`

## SBOM Mapping

- SCA Definition: `Documents/SBOM/components/ProductView/sca_uboot_image.json`
- Component Declaration: `Documents/SBOM/components/ProductView/sca_uboot_image.md`
- Package URL: `pkg:generic/u-boot-legacy-image-support@unknown`

## Review Result

- Status: Approved