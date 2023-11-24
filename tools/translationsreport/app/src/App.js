import { useState, useEffect } from 'react'

import { Grid, Segment, Header, Container, Divider } from 'semantic-ui-react'

import StringsList from "./StringsList.js"

// Data from latest main
const DEFAULT_DATA_URL = "TODO"

function App() {
  const [chosenTarget, setChosenTarget] = useState("main_app")
  const [isLoading, setIsLoading] = useState(true)
  const [error, setError] = useState("")
  const [data, setData] = useState({})

  useEffect(() => {
    fetch((new URL(document.location)).searchParams.get("data") || DEFAULT_DATA_URL)
      .then(response => {
        // Check if the request was successful (status code 200)
        if (!response.ok) {
          throw new Error(`Failed to download JSON file (HTTP status ${response.status})`)
        }

        // Parse the JSON data
        return response.json()
      })
      .then(jsonData => setData(jsonData))
      .catch(error => {
        setError(error.message)
        console.error('Error downloading JSON file:', error.message)
      })
  }, [])

  useEffect(() => {
    if (Object.keys(data).length > 0 || error) {
      setIsLoading(false)
    }
  }, [data, error])

  return (
    <Container style={{ margin: "50px 0" }}>
      <Header as='h1'>Mozilla VPN Translations Report</Header>
      {!isLoading && !error && (
        <p>This report was generated from branch {data.metadata.branch}, commit: {data.metadata.commit}.</p>
      )}
      <Divider style={{ margin: "30px 0" }}></Divider>
      {isLoading ? (
        <p>Loading</p>
      ) : error ? (
        <p>Error: {error}</p>
      ) : (
        <>
          <Grid>
            <Grid.Row>
              <Grid.Column width={6}>
              <Segment.Group>
                <Segment
                  className="clickable-segment"
                  onClick={() => setChosenTarget("main_app")}>Main app</Segment>
                {Object.keys(data.addon_translations).sort().map(addon => (
                  <Segment
                    className="clickable-segment"
                    onClick={() => setChosenTarget(addon)}>Addon: {addon}</Segment>
                ))}
              </Segment.Group>
              </Grid.Column>
              <Grid.Column width={10}>
                  {chosenTarget === "main_app"
                    ? <StringsList target={"Main app"} strings={data.main_app_translations} />
                    : <StringsList target={chosenTarget} strings={data.addon_translations[chosenTarget]} />
                  }
              </Grid.Column>
            </Grid.Row>
          </Grid>
        </>
      )}
    </Container>
  )
}

export default App
