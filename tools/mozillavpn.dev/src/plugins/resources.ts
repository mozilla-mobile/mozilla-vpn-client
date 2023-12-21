
/**
 * This plugin looks for the inspector in ../inspector.
 * It will invoke the build of the bundle and load it post build
 * so that the InspectorPage can import that.
 */
import 'node:process';

import {copyFile, readdirSync, mkdirSync, existsSync} from 'node:fs'
import {resolve, sep} from 'node:path'
import * as url from 'url';

const __dirname = url.fileURLToPath(new URL('.', import.meta.url));
const PROJECT_ROOT = resolve(__dirname + '../../');
// The Mozilla VPN Resources
const RES_FOLDER =
    resolve(PROJECT_ROOT.toString() + '../../../src/ui/resources');
    
const RES_OUT_FOLDER = resolve(PROJECT_ROOT.toString() + '/static/res');
if(!existsSync(RES_OUT_FOLDER)){
  mkdirSync(RES_OUT_FOLDER)
}

export default function(context, options) {
  return {
    name: 'copy-res-plugin',
    loadContent() {
      /**
       * Copy the Constents of the clients ui/res to 
       * static/res 
       */
      const f = readdirSync(RES_FOLDER);
        const copyJobs = f.map( name => new Promise(r=>{
          const file = resolve(`${RES_FOLDER}/${name}`)
          const outFile = `${RES_OUT_FOLDER}${sep}${name}`
          return copyFile(file,outFile,r)
        }))
          return Promise.all(copyJobs);
    },
  };
}
