/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 export function parseLogLine(line){
    if (line.length === 0 || line[0] !== '[') return
    const dateEndPos = line.indexOf(']')
    if (dateEndPos === -1) return
    const dateString = line.slice(1, dateEndPos)
    line = line.slice(dateEndPos + 1).trim()

    const categoryStartPos = line.indexOf('(')
    if (categoryStartPos === -1) return

    line = line.slice(categoryStartPos)
    const categoryEndPos = line.indexOf(')')
    if (categoryEndPos === -1) return
    const categoryString = line.slice(1, categoryEndPos)
    const log = line.slice(categoryEndPos + 1).trim()

    return {
      date: parseDate(dateString),
      category: categoryString,
      text: log
    }
}




function parseDate (str) {
  const parts = str.split(' ')
  const date = parts[0].split('.')
  return new Date(`${date[2]}-${date[1]}-${date[0]}T${parts[1]}`)
}


