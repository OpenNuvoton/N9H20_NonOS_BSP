# zlib Component Declaration

## Component Information

- Name: zlib
- SBOM Version: 0.95P
- Upstream Distribution: zlib-0.95
- Component Type: Library
- Scope: ProductView
- Integration: Vendored source
- Modification Status: Modified
- License: Zlib
- Primary Authors: Jean-loup Gailly and Mark Adler
- Additional Contributor: Paul Mackerras

## Source Location

- `Loaders/SpiLoader_gzip/ziplib/zlib.c`
- `Loaders/SpiLoader_gzip/ziplib/zlib.h`

## Version Evidence

The vendored `zlib.h` defines:

- `ZLIB_VERSION` as `0.95P`

The source files state that the implementation is derived from the
zlib-0.95 distribution.

## Modification Evidence

The source files document additions by Paul Mackerras for Deflate
compression and decompression of PPP packets.

Documented modifications include:

- Added support for `Z_PACKET_FLUSH`
- Added `inflateIncomp`
- Added a `minCompression` parameter
- Changed internal functions not used outside the implementation to local functions

## License Evidence

The zlib license notice is contained in:

- `Loaders/SpiLoader_gzip/ziplib/zlib.h`

The notice permits use, modification, and redistribution subject to the
stated attribution and source-notice requirements.

## Usage

The component provides Deflate decompression support for:

- `Loaders/SpiLoader_gzip`

The component is used by the compressed-image boot implementation.

## SBOM Mapping

- SCA Definition: `Documents/SBOM/components/ProductView/sca_zlib.json`
- Package URL: `pkg:generic/zlib@0.95P`
- Component Origin: Third-party
- Required Scope: ProductView

## Review Result

- Classification: Third-Party OSS
- Status: Approved