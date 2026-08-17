# zlib Evidence

## Component Identification

- Component: zlib
- SBOM Version: 0.95P
- Upstream Distribution: zlib-0.95
- Scope: ProductView
- Classification: Third-Party OSS
- Integration: Vendored source
- Modification Status: Modified
- License: Zlib

## Evidence Location

- `Loaders/SpiLoader_gzip/ziplib/zlib.c`
- `Loaders/SpiLoader_gzip/ziplib/zlib.h`

## Version Evidence

The vendored header defines:

- `ZLIB_VERSION` as `0.95P`

The source header also states that the implementation is derived from
the zlib-0.95 distribution.

The SBOM version is therefore recorded as `0.95P`, while the upstream
distribution is recorded separately as `zlib-0.95`.

## Modification Evidence

The source files document additions by Paul Mackerras for PPP packet
Deflate compression and decompression.

The documented modifications include `Z_PACKET_FLUSH`,
`inflateIncomp`, and other integration changes.

## License Evidence

The Zlib license text is contained in:

- `Loaders/SpiLoader_gzip/ziplib/zlib.h`

## Usage Evidence

The component is integrated into:

- `Loaders/SpiLoader_gzip`

The component provides Deflate decompression for compressed-image boot
support.

## SHA-256 Evidence

- `zlib.c`: `656C1AAA27BBD928A575FF77C3E701659A9F51B56E35B665362121C07F60E67A`
- `zlib.h`: `C69FD9FD640F01C27F08DAB0BAFBC667A45B5AB0CD293D1918461020CA57CD77`

## SBOM Mapping

- SCA Definition: `Documents/SBOM/components/ProductView/sca_zlib.json`
- Component Declaration: `Documents/SBOM/components/ProductView/sca_zlib.md`
- Package URL: `pkg:generic/zlib@0.95P`

## Review Result

- Status: Approved