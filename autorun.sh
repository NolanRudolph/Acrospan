#!/bin/bash
# Setup file for extracting OG webpage and saturating buffer etc.

# Usage
if [[ $# -lt 4 ]]; then
  printf "\nUsage: sudo bash ./%s <OG Page> <user> <pass> <pairs>\n\n" $0
  printf "\tOG Page: The URL of the page to translate\n"
  printf "\tuser   : The username for PostgreSQL\n"
  printf "\tpass   : The password for PostgreSQL\n"
  printf "\tpairs  : Space separated acronym:expansion (e.g. API:ApplicationProgrammingInterface)\n\n"
  exit 1
fi

# Set variables
PAGE=$1
USER=$2
PASS=$3

# Get webpage
wget --recursive --no-parent $PAGE
mv $PAGE/index.html .

# Compile C++
g++ -std=c++17 translate.cpp -o translate -lpqxx -lpq

./translate "index.html" $USER $PASS ${@:4:$#}
mv index.html $PAGE

# Publish back to server
ssh -i ~/Desktop/Acrospand.pem ech2-user@54.215.192.103 'sudo rm -rf /var/www/html/*'
scp -r -i ~/Desktop/Acrospand.pem $PAGE/* ec2-user@54.215.192.103:/var/www/html
