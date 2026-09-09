# N9H20 Current Published SBOM

## Publication Authority

- Release ID: `V1.03.000-10-g2ae21690`
- Source commit: `2ae216907a7053d50f9b986d959a7f7f2107c8f0`
- Source describe: `V1.03.000-10-g2ae21690`
- SVN revision: `r54`
- SVN release locator:
  `http://nthcrdvss01.nuvoton.com:9443/svn/SBOM/bsp/n9h20/V1.03.000-10-g2ae21690`

The SVN directory above is the immutable release authority. The three
fixed-name files at the Git repository root are current publication copies.
Product and Test Sample are byte-identical to their formal SVN artifacts. The
current manifest is a semantic projection that changes only current filenames
and adds the immutable SVN locator and formal artifact hashes. A Git commit
that updates these copies records artifact publication only and does not change
the release source identity recorded in the manifest.

## Current Git Copies

| Git current artifact | Current SHA-256 | Formal SVN artifact | Formal SHA-256 | Consistency |
|---|---|---|---|---|
| `n9h20bsp_Product_SBOM_cdx.json` | `4c3f2b9a27c46ca3e1d7375bd3e620d18969b1de0fad7d96e764b6125e313164` | `n9h20bsp_Product_SBOM_V1.03.000-10-g2ae21690.cdx.json` | `4c3f2b9a27c46ca3e1d7375bd3e620d18969b1de0fad7d96e764b6125e313164` | Byte-identical |
| `n9h20bsp_TestSample_SBOM_cdx.json` | `d8fb2df21554fd104be55af69c859f837f37813a30d29d795971696a0034340c` | `n9h20bsp_TestSample_SBOM_V1.03.000-10-g2ae21690.cdx.json` | `d8fb2df21554fd104be55af69c859f837f37813a30d29d795971696a0034340c` | Byte-identical |
| `n9h20bsp_SBOM_Manifest.json` | `d59f1b339c6bf40db520d7e2b5eac5e3f53d1412d25bf83bc02c8adf38652794` | `n9h20bsp_SBOM_Manifest_V1.03.000-10-g2ae21690.json` | `f135403e7396001b18fb20ab184a97e235f30aaa0be4b2766565cfabeab96dd0` | Fixed-name/provenance projection |

## Generation Boundary

These current copies are not inputs to the next SBOM generation. Product and
Test Sample views are created only from the include paths in
`Documents/SBOM/product-scope.yaml` and
`Documents/SBOM/test-sample-scope.yaml`; neither scope includes repository-root
SBOM artifacts.
