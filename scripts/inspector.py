#!/usr/bin/env python3
import sys
import argparse
import asyncio
import json
import websockets

parser = argparse.ArgumentParser(description='Send commands to the MozillaVPN inspector')
parser.add_argument('command', metavar='ARG', type=str, nargs='*',
                    help='inspector command to send')
parser.add_argument('-l', '--log', action='store_true',
                    help='print logs to stdout')
parser.add_argument('-q', '--quiet', action='store_true',
                    help='suppress command output')
args = parser.parse_args()

## Handle responses from the inspector
async def handle(ws, msgtype, value):
    if (msgtype == 'unknown'):
        print("Unknown inspector command: " + args.command[0], file=sys.stderr)
        await ws.close()
        sys.exit(1)
    elif (msgtype == 'log'):
        if (args.log):
            print(value)
    elif (msgtype == args.command[0]):
        await ws.close()
        if (value is not None) and not args.quiet:
            print(value)
        if (type(value) is bool):
           sys.exit(0 if value is True else 1)
    else:
        print("Unexpected message: " + msgtype, file=sys.stderr)
        if (value and not args.quiet):
            print(value)

## Run the websocket client
async def run():
    uri = "ws://localhost:8765"
    payload = ' '.join(args.command)
    async with websockets.connect(uri) as ws:
        await ws.send(payload)

        async for msg in ws:
            reply = json.loads(msg)
            if "error" in reply:
                print(reply['error'], file=sys.stderr)
            await handle(ws, reply['type'], reply.get('value'))

asyncio.get_event_loop().run_until_complete(run())
