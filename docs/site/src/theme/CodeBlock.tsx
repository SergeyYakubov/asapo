import React from 'react'
import InitCodeBlock from '@theme-init/CodeBlock'
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';

const requireContext = require.context('../../../../examples/for_site/', true, /\.(sh|py|cpp)$/);

const noteStyle: React.CSSProperties = {
    textAlign: 'right',
    fontSize: '.8em',
    marginTop: '-20px'
}

export interface State {
    isCancelled: boolean
}

async function fetchCode(url: string, snippetTag: string, state: State, setFetchResultState: React.Dispatch<React.SetStateAction<string>>) {
    let res: Response
    try {
        if (!state.isCancelled) {
            res = await fetch(url);
        }
    } catch (err) {
        if (!state.isCancelled) {
            setFetchResultState("cannot fetch code: " + err.toString());
        }
    }

    if (state.isCancelled) {
        return;
    }

    if (res.status !== 200) {
        const error = await res.text()
        setFetchResultState("cannot fetch code: " + error);
    }

    let body = (await res.text()).split('\n')
    const fromLine = body.indexOf(snippetTag + " start") + 1 || 0;
    const toLine = body.indexOf(snippetTag + " end", fromLine) - 1 || undefined;
    body = body.slice(fromLine, (toLine || fromLine) + 1)

    const preceedingSpace = body.reduce((prev: number, line: string) => {
        if (line.length === 0) {
            return prev
        }

        const spaces = line.match(/^\s+/)
        if (spaces) {
            return Math.min(prev, spaces[0].length)
        }

        return 0
    }, Infinity)

    setFetchResultState(body.map((line) => line.slice(preceedingSpace)).join('\n'));
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
    const urlLink = "https://stash.desy.de/projects/ASAPO/repos/asapo/browse/examples/for_site/" + codeBlockContent + "?at=" + version

    const snippetTag = getVal("snippetTag", props)
    if (codeBlockContent) {
        const res = requireContext('./'+codeBlockContent)
        let body = res.default.split('\n')
        const fromLine = body.indexOf(snippetTag + " start") + 1 || 0;
        const toLine = body.indexOf(snippetTag + " end", fromLine) - 1 || undefined;
        body = body.slice(fromLine, (toLine || fromLine) + 1).join('\n')

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