const net = require('node:net');
const {Buffer} = require('node:buffer');


const SERVER_PORT = 8754;


function ExtensionMessage(aT) {
  if (!aT) {
    this.t = 'help'
    return;
  }
  this.t = aT
};

/**
 * Connects to the Client's default WebExtension
 * API Socket
 *
 * @returns {Promise<net.Socket>} a node:net Socket with an established
 *     connection
 */
async function connectExtension() {
  const sock = new net.Socket();
  await new Promise(r => sock.connect(8754, '127.0.0.1', r));
  return sock;
};

/**
 * Encodes and sends a Message to the Client
 * @param {ExtensionMessage} message - Message :)
 * @param {net.Socket} socket - Socket
 *
 */
function sentToClient(message, sock) {
  const resEncodedMessage = new TextEncoder().encode(JSON.stringify(message))
  const b = new Uint32Array([resEncodedMessage.length]);
  const header = new Uint8Array(b.buffer);


  var raw_message = new Uint8Array([...header, ...resEncodedMessage]);
  sock.write(raw_message);
}

/**
 * Returns a generator Yieling Messages from the Webextension
 * The Generator is valid as long as the socket is open
 * @param {*} sock - an Open Socket
 * @returns {ReadableStream<Message>}- A ReadableStream yieling Messages
 */
function getMessageStream(sock = new net.Socket) {
  return new ReadableStream({
    start(cntrl) {
      sock.on('data', (buff) => {
        let data = buff;
        // Pop out the first X bytes
        // from data and return them
        const nom = (len) => {
          const out = data.slice(0, len)
          data = data.slice(len)
          return out;
        } while (data.length != 0) {
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
  readResponseOfType
}
