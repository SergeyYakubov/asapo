import React from 'react'
import InitCodeBlock from '@theme-init/CodeBlock'
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';



const requireContext = require.context('../../examples/', true, /(\.sh|\.py|\.cpp|\.c|\.txt|Makefile)$/);

const noteStyle: React.CSSProperties = {
    textAlign: 'right',
    fontSize: '.8em',
    marginTop: '-20px'
}

export interface State {
    isCancelled: boolean
}

function getVal(name: string, props: any) {
    const codeRegex = new RegExp("(?:" + name + "=\")(.*?)(\")")

    let val = undefined
    if (props.metastring && codeRegex.test(props.metastring)) {
        val = props.metastring.match(codeRegex)[1];
    }
    return val;
}

function ReferenceCode(props: any) {
    const codeBlockContent = getVal("content", props)

    if (!codeBlockContent) {
        return (
            <InitCodeBlock {...props}/>
        );
    }
    const {siteConfig} = useDocusaurusContext();
    const version = siteConfig.customFields.version;
    console.log(siteConfig);
    const urlLink = "https://stash.desy.de/projects/ASAPO/repos/asapo/browse/examples/for_site/" + codeBlockContent + "?at=" + version

    const snippetTag = getVal("snippetTag", props)
    if (codeBlockContent) {
        const c = codeBlockContent.replace('@ASAPO_EXAMPLES_DIR@/', '')
        const res = requireContext('./'+c)
        let body = res.default.split('\n')
        const fromLine = body.indexOf(snippetTag + " snippet_start") + 1;
        const toLine = body.indexOf(snippetTag + " snippet_end", fromLine) - 1;
        if (fromLine > 0) {
            body = body.slice(fromLine, (toLine>-1?toLine:fromLine) + 1)
        }
        const fromLineRemove = body.indexOf(snippetTag + " snippet_start_remove");
        const toLineRemove = body.indexOf(snippetTag + " snippet_end_remove", fromLineRemove);
        if (fromLineRemove>-1) {
            body.splice(fromLineRemove, toLineRemove>-1?toLineRemove-fromLineRemove + 1:2)
        }
        body = body.filter(a => !a.includes("snippet_start_remove") && !a.includes("snippet_end_remove"))
        body = body.join('\n')


        const customProps = {
            ...props,
            children: body,
        }

        return (
            <div>
                <InitCodeBlock {...customProps}/>
                <div style={noteStyle}>See full example on <a href={urlLink} target="_blank">BitBucket</a></div>
            </div>
        );
    }
}

export default function CodeBlock(props) {
    return (
        <ReferenceCode {...props} />
    );
}