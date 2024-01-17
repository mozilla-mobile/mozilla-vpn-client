import repl from 'node:repl'

import { InspectorWebsocketClient } from './inspector/inspectorwebsocketclient.js'


const session = repl.start('> ')


const client = new InspectorWebsocketClient()
client.isConnected.subscribe(ok =>{
    console.log(`Connected -> ${ok}`)
    session.context["client"] = client; 

})


client.qWebChannel.subscribe(channel => {
    if(channel){
        session.context["objects"] = channel.objects
    }
})

