#!/bin/sh
curl -o root_certificates https://ccadb.my.salesforce-sites.com/mozilla/IncludedRootsPEMTxt?TrustBitsInclude=Websites
dos2unix root_certificates
./compress_asset.py root_certificates root_certificates.xzh
