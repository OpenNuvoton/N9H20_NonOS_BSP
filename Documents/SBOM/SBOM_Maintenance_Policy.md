# BSP SBOM Maintenance Policy

## Purpose

This policy defines the maintenance, review, validation, and release
requirements for N9H20 BSP Software Bill of Materials deliverables.

## Scope

This policy applies to:

- N9H20 ProductView SBOM
- N9H20 TestSampleView SBOM
- Manual SCA component declarations
- Component evidence documents
- Exclusion records
- Vulnerability-scan results
- SBOM release packages

## Maintenance Triggers

The SBOM baseline must be reviewed and regenerated when any of the
following conditions occurs:

- A BSP release tag or Git commit changes
- A third-party component is added or removed
- A third-party component version changes
- Component source code is modified
- Component license information changes
- ProductView or TestSampleView scope changes
- Binary libraries or executable files change
- The SBOM generation toolchain changes
- A vulnerability finding requires metadata correction
- Evidence files or exclusion decisions change

## Required Activities for Every BSP Release

1. Confirm the BSP Git tag and full Git commit.
2. Confirm that the Git working tree used for scanning is appropriate.
3. Review ProductView and TestSampleView scope definitions.
4. Regenerate ProductView and TestSampleView SBOM files.
5. Merge all approved manual SCA component declarations.
6. Apply binary hash and license enrichment.
7. Apply license enrichment matching.
8. Validate the final CycloneDX SBOM files.
9. Generate the SBOM manifest.
10. Run vulnerability scans for both SBOM views.
11. Review newly introduced or changed vulnerabilities.
12. Update component evidence and SHA-256 values.
13. Update exclusions when first-party classification changes.
14. Update the component inventory and review record.
15. Generate release-package SHA-256 checksums.
16. Update the latest-release pointer after approval.

## Manual SCA Maintenance

Each manually declared component must include:

- Component name
- Component version, or `Unknown` with justification
- Component type
- Scope
- Supplier or author
- Package URL
- License identifier
- Source location
- Version evidence
- License evidence
- Modification status
- Component origin
- Stable BOM reference

Each SCA JSON file must have a corresponding Markdown declaration and
an approved evidence document.

## Evidence Requirements

Evidence documents must identify:

- Source files used for component identification
- Version determination method
- License determination method
- Integration and usage location
- Modification status
- SHA-256 values for primary evidence files
- SCA definition mapping
- Review status

Evidence SHA-256 values must be regenerated when an evidence source
file changes.

## Exclusion Requirements

An exclusion may be recorded only after manual review confirms that the
item is first-party, proprietary, generated, or otherwise outside the
third-party component inventory.

Each exclusion must document:

- Component or tool name
- Source path
- Classification
- Exclusion reason
- Evidence path when available

An exclusion must not be used to suppress an unresolved third-party or
license finding.

## Vulnerability Management

Vulnerability scans must be executed against both:

- ProductView SBOM
- TestSampleView SBOM

Release criteria:

- Vulnerability scanning completes successfully
- Critical findings are reviewed before release
- Accepted findings have an approved disposition
- Suppressions include justification and review ownership
- Vulnerability database failure is reported separately from SBOM
  generation failure

SBOM generation and vulnerability scanning are separate DevSecOps
stages. A vulnerability database network failure must not invalidate a
successfully generated and validated SBOM, but the release package must
record the incomplete scan status.

## Validation Requirements

Before release:

- CycloneDX validation must pass
- Manual SCA merge results must be confirmed
- ProductView and TestSampleView components must match the inventory
- Required licenses must be present
- Developer-specific absolute paths must not appear in final SBOM files
- Manifest file references must resolve
- Release checksums must be generated and verified

## Release Package Requirements

Each SBOM release package must contain:

- ProductView CycloneDX SBOM
- TestSampleView CycloneDX SBOM
- SBOM manifest
- README
- SHA256SUMS file
- ProductView vulnerability-scan result
- TestSampleView vulnerability-scan result
- Vulnerability-scan summary

The release directory name must use the Git-derived BSP version.

## Roles and Responsibilities

### BSP Maintainer

- Maintains source and release information
- Confirms BSP tag and commit
- Reviews scope changes
- Supports component-source identification

### SBOM Maintainer

- Generates and validates SBOM files
- Maintains SCA definitions and evidence
- Maintains inventory and exclusions
- Prepares the SBOM release package

### Security Reviewer

- Reviews vulnerability results
- Reviews license and component-classification exceptions
- Approves security dispositions when required

## Review Frequency

The SBOM must be reviewed:

- For every BSP release
- When a third-party component changes
- When a security advisory affects an included component
- When the SBOM generation pipeline changes
- At least once during each maintained release cycle

## Current N9H20 Baseline

- BSP Release Tag: V1.03.000
- BSP Git Commit: 49b4adaba4c2e1e5cf556ab055217be5c4049342
- ProductView third-party components: 3
- TestSampleView third-party components: 1
- Reviewed first-party exclusions: 5