#!/bin/sh
curl -o root_certificates https://ccadb.my.salesforce-sites.com/mozilla/IncludedRootsPEMTxt?TrustBitsInclude=Websites
dos2unix root_certificates
