import { useState, useEffect, useCallback } from 'react'

import { Table, Icon, Input, Progress } from 'semantic-ui-react'
import LanguageDropdown, { LANGUAGES } from './LanguageDropdown.js'

function StringsList({ target, strings }) {
  const defaultExpectedLanguages = LANGUAGES.map(l => l.value)

  const [searchTerm, setSearchTerm] = useState("")
  const [expectedLanguages, setExpectedLanguages] = useState(defaultExpectedLanguages)

  const filterLanguagesPerString = useCallback(() => {
    return Object.entries(strings).reduce((result, [string, languages]) => {
        return {
            ...result,
            [string]: languages.filter(l => expectedLanguages.includes(l))
        }
      }, {})
  }, [strings, expectedLanguages])
  const [parsedStrings, setParsedStrings] = useState(filterLanguagesPerString())

  const calculateCompleteness = useCallback(() => {
    const [percentagesSum, stringsSum] = Object.entries(parsedStrings).reduce(([p, s], [_, languages]) => {
        // How much is this specific string translated?
        const localCompleteness = expectedLanguages.filter(l => languages.includes(l)).length / expectedLanguages.length
        return [ p + localCompleteness, s + 1 ]
      }, [0, 0])

    return (percentagesSum / stringsSum) * 100;
  }, [parsedStrings, expectedLanguages])
  const [completeness, setCompleteness] = useState(calculateCompleteness())

  const onSearch = event => {
    event.preventDefault();
    setSearchTerm(event.target.value)
  }

  useEffect(() => {
    setParsedStrings(filterLanguagesPerString())
  }, [strings, expectedLanguages, filterLanguagesPerString])

  useEffect(() => {
    setCompleteness(calculateCompleteness())
  }, [parsedStrings, calculateCompleteness])

  return (
    <>
        <h1>{target}</h1>
        <div style={{ display: "flex", justifyContent: "space-between" }}>
            <Input icon placeholder='Search...' value={searchTerm} onChange={onSearch}>
                <input />
                <Icon name='search' />
            </Input>
            <LanguageDropdown onChange={l => setExpectedLanguages(l ? [l] : defaultExpectedLanguages)} />
        </div>

        <Progress
            style={{ marginBottom: "7px" }}
            percent={completeness.toFixed(2)}
            progress
            color={completeness >= 70 ? 'green' : 'orange'}
        />
        <p style={{ fontSize: "10px", color: "#999" }}>Completeness score for {expectedLanguages.sort().join(", ").trim()}</p>

        {Object.keys(parsedStrings).length === 0 ? (
            <p style={{ marginTop: "30px" }}>No translations found.</p>
        ) : (
            <Table striped>
            <Table.Header>
                <Table.Row>
                    <Table.HeaderCell>String</Table.HeaderCell>
                    <Table.HeaderCell>Translation Ratio</Table.HeaderCell>
                </Table.Row>
            </Table.Header>

            <Table.Body>
                {Object.entries(parsedStrings).map(([string, languages]) => {
                if (searchTerm && !string.toLowerCase().includes(searchTerm.toLowerCase())) {
                    return null;
                }

                return (
                    <Table.Row>
                    <Table.Cell>
                        {string}&nbsp;
                        {languages.length === expectedLanguages.length
                            ? <Icon color='green' name='checkmark' title="All translations present"/>
                            : <Icon color='red' name='question circle outline'  title={`Missing translations for ${expectedLanguages.filter(l => !languages.includes(l)).sort().join(", ").trim()}`} />
                        }
                    </Table.Cell>
                    <Table.Cell>{languages.length} of {expectedLanguages.length} translations</Table.Cell>
                    <Table.Cell></Table.Cell>
                    </Table.Row>
                )
                })}
            </Table.Body>
            </Table>
        )}
    </>
  );
}

export default StringsList;
