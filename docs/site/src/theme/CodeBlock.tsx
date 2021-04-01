import { URL } from 'url'
import React, { useReducer } from 'react'
import InitCodeBlock from '@theme-init/CodeBlock'
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import type { ReferenceCodeBlockProps, GitHubReference, DispatchMessage } from './types'
import {DispatchTypes} from "./types";
import useBaseUrl from '@docusaurus/useBaseUrl';

const initialFetchResult = 'loading...';

const noteStyle: React.CSSProperties = {
    textAlign: 'right',
    fontSize: '.8em',
    marginTop: '-20px'
}

export interface State {
    isCancelled: boolean
}

async function fetchCode (url: string,snippetTag:string, state: State,setFetchResultState: React.Dispatch<React.SetStateAction<string>>) {
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
    const fromLine = body.indexOf(snippetTag+" start") + 1  || 0;
    const toLine = body.indexOf(snippetTag+ " end",fromLine) -1 || undefined;
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

function getVal(name: string,props: any) {
    const codeRegex = new RegExp("(?:"+name+"=\")(.*?)(\")")

    let val = undefined
    if (props.metastring && codeRegex.test(props.metastring)) {
        val = props.metastring.match(codeRegex)[1];
    }
    return val;
}

function ReferenceCode(props: any) {

    const [fetchResult, setFetchResultState] = React.useState(initialFetchResult);

    const codeBlockLink = getVal("link",props)
    const localLink = useBaseUrl("examples/"+codeBlockLink)
    const {siteConfig} = useDocusaurusContext();
    const version  = siteConfig.customFields.version;
    const urlLink = "https://stash.desy.de/projects/ASAPO/repos/asapo/browse/examples/for_site/"+codeBlockLink+"?at="+version
    if (!codeBlockLink) {
        return (
                <InitCodeBlock {...props}/>
        );
    }
    const snippetTag = getVal("snippetTag",props)

    React.useEffect(() => {
        let isCancelled = {isCancelled:false};
        fetchCode(localLink,snippetTag,isCancelled,setFetchResultState)
        return () => {
            isCancelled.isCancelled = true;
        };
    }, []);


    const customProps = {
        ...props,
        children: fetchResult
    }

    return (
        <div>
            <InitCodeBlock {...customProps}/>
            <div style={noteStyle}>See full example on <a href={urlLink} target="_blank">BitBucket</a></div>
        </div>
    );
}


export default function CodeBlock(props) {
    return (
            <ReferenceCode {...props} />
    );
}