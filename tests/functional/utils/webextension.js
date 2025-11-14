const net = require('node:net');
const {Buffer} = require('node:buffer');
const {spawn} = require('child_process');

const app = process.env.MVPN_BIN;

class ExtensionMessage{
  t= ""
}

function makeMessage(aT, extra) {
  return {
    ...extra, 
    t: aT
  }
};

/**
 * Connects to the Client's default WebExtension
 * API Socket
 *
 * @returns {Promise<ChildProcess>} a node:net Socket with an established
 *     connection
 */
async function connectExtension() {
  let p = spawn(app, ['webext', 'mozillavpn.json', 'vpn@mozilla.com']);
  p.stderr.on('data', (data) => {
    console.log("bridge stderr:", data.toString());
  });

  // await a status message indicating the VPN client is up.
  let stream = getMessageStream(p);
  const reader = stream.getReader();
  while (true) {
    const {value} = await reader.read();
    if (value.status == "vpn-client-up") {
      reader.releaseLock()
      break;
    }
  }

  return p;
};

/**
 * Encodes and sends a Message to the Client
 * @param {ExtensionMessage} message - Message :)
 * @param {ClientProcess} bridge - an web extension bridge process
 *
 */
function sentToClient(message, bridge) {
  const resEncodedMessage = new TextEncoder().encode(JSON.stringify(message));
  const b = new Uint32Array([resEncodedMessage.length]);
  const header = new Uint8Array(b.buffer);

  var raw_message = new Uint8Array([...header, ...resEncodedMessage]);
  bridge.stdin.write(raw_message);
}

/**
 * Returns a generator Yieling Messages from the Webextension
 * The Generator is valid as long as the socket is open
 * @param {*} bridge - an web extension bridge process
 * @returns {ReadableStream<Message>}- A ReadableStream yieling Messages
 */
function getMessageStream(bridge) {
  return new ReadableStream({
    start(cntrl) {
      bridge.stdout.setEncoding('binary');
      bridge.stdout.on('data', (buff) => {
        let data = buff;
        // Pop out the first X bytes
        // from data and return them
        const nom = (len) => {
          const out = data.slice(0, len)
          data = data.slice(len)
          return Buffer.from(out);
        };
        while (data.length != 0) {
          const header = nom(4)
          const len = header.readUint32LE(0);
          const payload = nom(len);
          const text = new TextDecoder().decode(payload)
          const blob = JSON.parse(text);
          cntrl.enqueue(blob);
        }
      });
    }
  });
}


/**
 * Returns a Promise that Resolves if a Message of the
 * provided type is returned
 * @param {String} type
 * @param {ReadableStream<Message>} messagePipe
 *
 * @returns {Promise<ExtensionMessage>} - The Message
 */
async function readResponseOfType(type = '', messagePipe) {
  const reader = messagePipe.getReader();
  while (true) {
    const {value} = await reader.read();
    if (value.t == type) {
      reader.releaseLock()
      return value
    }
  }
}

module.exports = {
  getMessageStream,
  sentToClient,
  connectExtension,
  ExtensionMessage,
  readResponseOfType,
  makeMessage,
}
