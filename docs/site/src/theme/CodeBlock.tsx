import React from 'react'
import InitCodeBlock from '@theme-init/CodeBlock'

const requireContext = require.context('../../', true, /(\.sh|\.py|\.cpp|\.c|\.txt|Makefile)$/);

const noteStyle: React.CSSProperties = {
    textAlign: 'right',
    fontSize: '.8em',
    marginTop: '-20px'
}

export interface State {
    isCancelled: boolean
}

function ReferenceCode(props: any) {
    let codeBlockContent = props.content

    if (codeBlockContent == undefined) {
        return (
            <InitCodeBlock {...props}/>
        );
    }
    codeBlockContent = codeBlockContent.replace(/"/g,'')
    
    const urlLink = "https://stash.desy.de/projects/ASAPO/repos/asapo/browse/docs/site/" + codeBlockContent

    let snippetTag = props.snippetTag
    if (snippetTag !== undefined) {
        snippetTag = snippetTag.replace(/"/g,'')
    }
    
    if (codeBlockContent) {
        const res = requireContext(codeBlockContent)
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
