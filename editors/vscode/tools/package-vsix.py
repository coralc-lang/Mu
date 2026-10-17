#!/usr/bin/env python3
"""Package the extension as a .vsix without needing vsce or network access.

A .vsix is an OPC zip: a content-types map, a manifest, and the extension
files under an `extension/` prefix. `npx @vscode/vsce package` is the normal
route, but that pulls npm packages over the network, so this builds the same
archive directly.
"""
import json, pathlib, sys, zipfile, datetime

ROOT = pathlib.Path(__file__).resolve().parent.parent
OUT = ROOT / 'mu-0.1.0.vsix'

pkg = json.loads((ROOT / 'package.json').read_text())
name, version, publisher = pkg['name'], pkg['version'], pkg['publisher']
files = [
    'package.json',
    'language-configuration.json',
    'syntaxes/mu.tmLanguage.json',
    'README.md',
]
for rel in files:
    if not (ROOT / rel).is_file():
        sys.exit(f'missing {rel}')

CONTENT_TYPES = """<?xml version="1.0" encoding="utf-8"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension="json" ContentType="application/json"/>
  <Default Extension="md" ContentType="text/markdown"/>
  <Default Extension="xml" ContentType="text/xml"/>
</Types>
"""

MANIFEST = f"""<?xml version="1.0" encoding="utf-8"?>
<PackageManifest Version="2.0.0" \
xmlns="http://schemas.microsoft.com/developer/vsx-schema/2011">
  <Metadata>
    <Identity Language="en-US" Id="{name}" Version="{version}" Publisher="{publisher}"/>
    <DisplayName>{pkg['displayName']}</DisplayName>
    <Description xml:space="preserve">{pkg['description']}</Description>
    <Tags>mu,language,programming-language</Tags>
    <Categories>{pkg['categories'][0]}</Categories>
    <GalleryFlags>Public</GalleryFlags>
    <Properties>
      <Property Id="Microsoft.VisualStudio.Code.Engine" \
Value="^{pkg['engines']['vscode'].lstrip('^')}"/>
      <Property Id="Microsoft.VisualStudio.Code.ExtensionDependencies" Value=""/>
      <Property Id="Microsoft.VisualStudio.Code.ExtensionPack" Value=""/>
      <Property Id="Microsoft.VisualStudio.Code.ExtensionKind" Value="workspace"/>
      <Property Id="Microsoft.VisualStudio.Services.Links.Source" \
Value="https://github.com/coralc-lang/Mu"/>
    </Properties>
  </Metadata>
  <Installation>
    <InstallationTarget Id="Microsoft.VisualStudio.Code"/>
  </Installation>
  <Dependencies/>
  <Assets>
    <Asset Type="Microsoft.VisualStudio.Code.Manifest" \
Path="extension/package.json" Addressable="true"/>
    <Asset Type="Microsoft.VisualStudio.Services.Content.Details" \
Path="extension/README.md" Addressable="true"/>
  </Assets>
</PackageManifest>
"""

with zipfile.ZipFile(OUT, 'w', zipfile.ZIP_DEFLATED) as z:
    z.writestr('[Content_Types].xml', CONTENT_TYPES)
    z.writestr('extension.vsixmanifest', MANIFEST)
    for rel in files:
        z.write(ROOT / rel, f'extension/{rel}')

print(f'wrote {OUT} ({OUT.stat().st_size} bytes)')
with zipfile.ZipFile(OUT) as z:
    for n in z.namelist():
        print(f'  {n}')
