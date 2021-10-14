#!/bin/bash

if [[ -z "${DOCS_VERSION}" ]]; then
    echo No version specified

    exit 1
fi

echo Freezing version $DOCS_VERSION

npm run docusaurus docs:version $DOCS_VERSION

VERSIONED_EXAMPLES="versioned_examples/version-$DOCS_VERSION"
VERSIONED_EXAMPLES_ESCAPED="versioned_examples\\/version-$DOCS_VERSION"

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
,s/100\.0[~.]develop/${DOCS_VERSION}/g
w
ED_COMMANDS
done

exit 0
