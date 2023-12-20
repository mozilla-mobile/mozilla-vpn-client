
/**
 * This plugin looks for the inspector in ../inspector.
 * It will invoke the build of the bundle and load it post build
 * so that the InspectorPage can import that.
 */
import 'node:process';

const dir = process.cwd();

console.log('HELLO WORLD');



export default function(context, options) {
  return {
    name: 'inspector-plugin',
    async loadContent() {
      return 1 + Math.floor(Math.random() * 10);
    },
  };
}
