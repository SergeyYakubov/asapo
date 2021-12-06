#!/bin/bash

if [[ -z "${DOCS_VERSION}" ]]; then
    echo No version specified

    exit 1
fi

echo Freezing version $DOCS_VERSION

npm run docusaurus docs:version $DOCS_VERSION

VERSIONED_EXAMPLES="versioned_examples/version-$DOCS_VERSION"
VERSIONED_EXAMPLES_ESCAPED="versioned_examples\\/version-$DOCS_VERSION"

# special case for pip version, which should not contain leading zeroes
VERSION_FOR_PIP=$DOCS_VERSION
# remove the leading zero in the version, if present
[[ $VERSION_FOR_PIP =~ (.+)\.(0.+) ]] && VERSION_FOR_PIP="${BASH_REMATCH[1]}.${BASH_REMATCH[2]:1}"

mkdir $VERSIONED_EXAMPLES

cp -r examples/* $VERSIONED_EXAMPLES

CONTENT='content=\"\.\/'

#replace the links to the code examples to the frozen copies
for file in $(find ./versioned_docs/version-$DOCS_VERSION -type f)
do
ed -s $file <<ED_COMMANDS > /dev/null 2>&1
,s/content=\"\?\.\/examples/content=\".\/${VERSIONED_EXAMPLES_ESCAPED}/g
w
ED_COMMANDS
done

#replace the links to the dev-packages to the versioned ones
for file in $(find ./${VERSIONED_EXAMPLES} -type f)
do
ed -s $file <<ED_COMMANDS > /dev/null 2>&1
,s/asapo-cluster-dev:100\.0\.develop/asapo-cluster:${DOCS_VERSION}/g
,s/==100\.0\.dev0/==${VERSION_FOR_PIP}/g
,s/100\.0[~.]develop/${DOCS_VERSION}/g
,s/100\.0[~.]dev0/${DOCS_VERSION}/g
w
ED_COMMANDS
done

exit 0
