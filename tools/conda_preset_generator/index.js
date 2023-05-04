/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


const commandLineArgs = require("command-line-args");
const term = require("terminal-kit").terminal;
const fs = require("fs");
const path = require("path");
const child_process = require("node:child_process");

const optionDefinitions = [
  { name: "name", alias: "n", type: String, defaultValue: "" },
  { name: "help", alias: "h", type: Boolean, defaultValue: false },
  { name: "env", alias: "e", type: String, defaultValue: "" },
];
let { name, help, env} = commandLineArgs(optionDefinitions);

if (help) {
  term.magenta("Conda Preset Generator \n");
  term(
    "\t Can generate a UserPresets.json File for your current Conda Env, to use outside of terminals \n",
  );
  term("\t Usage; \n");
  term("\t \t $: conda activate MY_Enviroment_To_export \n");
  term(
    "\t \t $: npm run conda_preset_generator --name <cmake_preset_name>  \n",
  );
  process.exit();
}

let env_info;
try {
    env_info = JSON.parse(child_process.execSync("conda info --json"));
} catch (error) {
  term.red("Failed to get info about your conda enviroment: \n");
  term.red(error.toString());
  process.exit();
}

if(env_info["active_prefix_name"] == "base"){
    if( env === ""){
        term.red("You are in your base env, please `conda activate ` the env you want to export! \n");
        term.red("Or Provide the env to export with --env <name> \n");
        process.exit();
    }
    const command = [
        "conda",
        "run", 
        "--name",
        env,
        "node",
        path.resolve(__dirname, 'index.js'),
    ]
    if(name !== ""){
        command.push("--name"),
        command.push(name);
    }
    console.log("Running: "+command.join(" "));
    child_process.execSync(command.join(" "));
    process.exit();
}
if( name === ""){
    name = "conda_"+env_info["active_prefix_name"]
}

term.green(`Exporting Conda Env "${env_info["active_prefix_name"]}" as cmake preset "${name}" \n`);

const configure_preset = {
    "name": name,
    "displayName": name,
    "description": `Exported Conda Env: ${env_info["active_prefix_name"]}`,
    "generator": "Ninja",
    "binaryDir": "${sourceDir}/build/"+name,
    "cacheVariables": {
    },
    "environment": process.env,
    "vendor": {}
}
const build_preset = {
    "name": name,
    "configurePreset": name,
    "targets":["mozillavpn","build_tests"]
}
  

const template_file = fs.readFileSync(path.resolve(__dirname, 'template.json'), 'UTF-8');
const template =  JSON.parse(template_file);
let preset; 
try {
    const preset_file = fs.readFileSync(path.resolve(__dirname, '../../CMakeUserPresets.json'), 'UTF-8');
    if(preset_file === ""){
        throw Error("CMakeUserPresets is Empty - overwriting \n")
    }
    preset = JSON.parse(preset_file);
    if(preset.version > template.version){
        term.red("Your CMakeUserPresets has a newer format-version then what the template is expecting \n");
        term.red("Please update this tool i guess .__.\" \n");
        process.exit();
    }
} catch (error) {
    term.red(error);
    preset = template;
}

preset.configurePresets.push(configure_preset);
preset.buildPresets.push(build_preset);

const encoder = new TextEncoder();
const data = encoder.encode(JSON.stringify(preset));
fs.writeFileSync(path.resolve(__dirname, '../../CMakeUserPresets.json'), data);

term.blue("YAY \n");