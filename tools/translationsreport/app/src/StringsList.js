import { useState } from 'react'

import { Table, Icon, Input } from 'semantic-ui-react'

function StringsList({ target, strings }) {
  const [searchTerm, setSearchTerm] = useState("")

  const expectedLanguages = [
    "bg", "co", "cs", "cy", "da", "de", "dsb", "el", "en", "en_CA", "en_GB", "es_AR", "es_CL",
    "es_ES", "es_MX", "fa", "fi", "fr", "fy_NL", "hsb", "hu", "ia", "id", "is", "it", "ja", "lo",
    "nl", "oc", "pa_IN", "pl", "pt_BR", "pt_PT", "ru", "sk", "sl", "sq", "sv_SE", "tr", "uk",
    "zh_CN", "zh_TW"
  ]

  const onSearch = event => {
    event.preventDefault();
    setSearchTerm(event.target.value)
  }

  console.log(strings)

  return (
    <>
        <h1>{target}</h1>
        <Input icon placeholder='Search...' value={searchTerm} onChange={onSearch}>
        <input />
        <Icon name='search' />
        </Input>
        {Object.keys(strings).length == 0 ? (
            <p style={{ marginTop: "30px" }}>No translations found.</p>
        ) : (
            <Table striped>
            <Table.Header>
                <Table.Row>
                <Table.HeaderCell rowSpan='2'>String</Table.HeaderCell>
                <Table.HeaderCell colSpan='2'>Stats</Table.HeaderCell>
                </Table.Row>
                <Table.Row>
                <Table.HeaderCell>Translation Percentage</Table.HeaderCell>
                <Table.HeaderCell>Translated Languages</Table.HeaderCell>
                </Table.Row>
            </Table.Header>

            <Table.Body>
                {Object.entries(strings).map(([string, languages]) => {
                if (searchTerm && !searchTerm.startsWith("language:") && !string.toLowerCase().includes(searchTerm.toLowerCase())) {
                    return;
                }

                if (searchTerm.startsWith("language:")) {
                    const [ _, languageList ] = searchTerm.split("language:")
                    const searchedLanguages = languageList.split(",");

                    if(!searchedLanguages.find(searchedLanguage => languages.map(l => l.toLocaleLowerCase()).toLowerCase().includes(searchedLanguage.toLowerCase()))) {
                    return;
                    }
                }

                return (
                    <Table.Row>
                    <Table.Cell>
                        {string}
                        {languages.length === expectedLanguages.length
                        ? <Icon color='green' name='checkmark' title="All translations present"/>
                        : <Icon color='red' name='x'  title={`Missing translations for ${expectedLanguages.filter(l => !languages.includes(l)).sort().join(", ").trim()}`} />
                        }
                    </Table.Cell>
                    <Table.Cell>{Math.floor((languages.length / expectedLanguages.length) * 100)}%</Table.Cell>
                    <Table.Cell>{languages.sort().join(", ").trim()}</Table.Cell>
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
