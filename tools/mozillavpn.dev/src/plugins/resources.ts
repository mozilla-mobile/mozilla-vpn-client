
/**
 * This plugin looks for the inspector in ../inspector.
 * It will invoke the build of the bundle and load it post build
 * so that the InspectorPage can import that.
 */
import 'node:process';

import {copyFile, readdirSync} from 'node:fs'
import {resolve} from 'node:path'
import * as url from 'url';

const __dirname = url.fileURLToPath(new URL('.', import.meta.url));
const PROJECT_ROOT = resolve(__dirname + '../../');
// The Mozilla VPN Resources
const RES_FOLDER =
    resolve(PROJECT_ROOT.toString() + '../../../src/ui/resources');
const RES_OUT_FOLDER = resolve(PROJECT_ROOT.toString() + '/static/res');

export default function(context, options) {
  return {
    name: 'copy-res-plugin',
    loadContent() {
      const f = readdirSync(RES_FOLDER);
        const copyJobs = f.map( file => new Promise(r=>{
          console.log(`Copy ${RES_FOLDER}/${file} to ${RES_OUT_FOLDER} \n`)
          return copyFile(`${RES_FOLDER}/${file}`,`${RES_OUT_FOLDER}/${file}`,r)
        }))
          return Promise.all(copyJobs);
    },
  };
}
