# bzip2 Evidence

## Component Identification

- Component: bzip2
- SBOM Version: 1.0.2
- Upstream Distribution: bzip2-1.0.2
- Scope: ProductView
- Classification: Third-Party OSS
- Integration: Vendored source
- Modification Status: Modified
- License: LicenseRef-bzip2-1.0.2

## Evidence Location

- `Loaders/SpiLoader_gzip/ziplib/bzlib.c`
- `Loaders/SpiLoader_gzip/ziplib/bzlib.h`
- `Loaders/SpiLoader_gzip/ziplib/bzlib_private.h`
- `Loaders/SpiLoader_gzip/ziplib/bzlib_decompress.c`
- `Loaders/SpiLoader_gzip/ziplib/bzlib_huffman.c`
- `Loaders/SpiLoader_gzip/ziplib/bzlib_crctable.c`
- `Loaders/SpiLoader_gzip/ziplib/bzlib_randtable.c`

## Version Evidence

The vendored `bzlib.h` and `bzlib.c` explicitly state that the files
are modified versions from the bzip2-1.0.2 distribution.

The historical 0.9.0 entries in `bzlib.c` are source change-history
entries and are not used as the component version.

## Modification Evidence

The source files explicitly identify themselves as modified.

The embedded integration also defines:

- `BZ_NO_STDIO`
- `BZ_NO_COMPRESS`

These settings configure the component for embedded decompression use.

## License Evidence

The bzip2 license notice is contained in:

- `Loaders/SpiLoader_gzip/ziplib/bzlib.h`
- `Loaders/SpiLoader_gzip/ziplib/bzlib.c`

The SPDX license identifier used in the SBOM is `LicenseRef-bzip2-1.0.2`. This is
the SPDX license identifier and does not represent the component version.

## Usage Evidence

The component is integrated into:

- `Loaders/SpiLoader_gzip`

The component provides bzip2 decompression for compressed-image boot
support.

## SHA-256 Evidence

- `bzlib.c`: `816207EF420EBA950AE4BB36A05B52DDF45BC0E1F2B406E85DE6026C175A06BB`
- `bzlib.h`: `EACBCBFD2D544A431CD2E31C943F47931CE6A95A2892277DAFEE3296A8A586DF`
- `bzlib_private.h`: `FA2AD087DEF6DE1A1DFF41B39ED01034369011996BCCF84D473103243AC683B9`

## SBOM Mapping

- SCA Definition: `Documents/SBOM/components/ProductView/sca_bzip2.json`
- Component Declaration: `Documents/SBOM/components/ProductView/sca_bzip2.md`
- Package URL: `pkg:generic/bzip2@1.0.2`

## Review Result

- Status: Approved

## Canonical Source Closure Hash

- Algorithm: SHA-256
- Value: `faee3239c5e950f7b788e76ac095c0012d5488373eede7efbe65518cd0bdabba`
- Hash Method: Sorted UTF-8 normalized relative paths followed by exact file bytes
- Coverage: Seven bzip2 source and header files listed in this evidence
