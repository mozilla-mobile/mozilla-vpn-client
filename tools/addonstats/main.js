/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// This function parses a RCC file. It follows the `qresource.cpp`
// implementation. See
// https://code.qt.io/cgit/qt/qtbase.git/tree/src/corelib/io/qresource.cpp
async function loadAddon(addonUrl) {
  console.log(`Loading addon ${addonUrl}`);
  const rcc = [];

  function findOffset(node) {
    return node * (14 + (version > 2 ? 8 : 0))
  }

  const data = await fetch(addonUrl)
                   .then(r => r.arrayBuffer())
                   .then(r => new DataView(r));
  if ([data.getUint8(0), data.getUint8(1), data.getUint8(2), data.getUint8(3)]
          .map(a => String.fromCharCode(a))
          .join('') !== 'qres') {
    throw new Error('Unsupported format');
  }
  let offset = 4;

  const version = data.getInt32(offset);
  offset += 4;

  const tree_offset = data.getInt32(offset);
  offset += 4;

  const payload_offset = data.getInt32(offset);
  offset += 4;

  const names_offset = data.getInt32(offset);
  offset += 4;

  let file_flags = 0;
  if (version >= 3) {
    file_flags = data.getInt32(offset);
    offset += 4;
  }

  if (data.byteLength >= 0 &&
      (tree_offset >= data.byteLength || payload_offset >= data.byteLength ||
       names_offset >= data.byteLength)) {
    throw new Error('Invalid parameters');
  }

  if (version < 1 || version > 3) {
    throw new Error('Unsupported version');
  }

  const child_count = data.getInt32(tree_offset + 6);
  const child = data.getInt32(tree_offset + 10);

  function readFolder(path, child_count, child) {
    const decoder = new TextDecoder('UTF-8');

    for (let i = 0; i < child_count; ++i) {
      console.log(` - Child ${i} in path ${path}`);

      let offset = findOffset(child + i);
      let name_offset = data.getInt32(tree_offset + offset);
      offset += 4;

      let name_length = data.getUint16(names_offset + name_offset);

      // The official QT implementation does these +2, +4 too. Let's keep them
      // here to keep the implementation in sync with `qresource.cpp`.
      name_offset += 2;
      name_offset += 4;

      const name_array = [];
      for (let j = 0; j < name_length; ++j) {
        name_array.push(data.getUint16(names_offset + name_offset + (j * 2)));
      }

      const fileName = decoder.decode(new Uint8Array(name_array));
      console.log(` -- name: ${fileName}`);

      const flags = data.getInt16(tree_offset + offset);
      offset += 2;

      if (flags & 0x02 /* Directory */) {
        console.log(' -- type: Directory!');

        const sub_child_count = data.getInt32(tree_offset + offset);

        offset += 4;  // some flags we ignore.

        const sub_child = data.getInt32(tree_offset + offset);
        readFolder(path + fileName + '/', sub_child_count, sub_child);
        continue;
      }

      console.log(' -- type: File!');

      offset += 4;  // some flags we ignore.

      const data_offset = data.getInt32(tree_offset + offset);
      let payload_data_offset = payload_offset + data_offset;

      const data_length = data.getUint32(payload_data_offset);
      payload_data_offset += 4;

      let content = null;

      // The file content is compressed with zlib
      if (flags & 0x01) {
        const size = data.getUint32(payload_data_offset);
        payload_data_offset += 4;

        const buffer = [];
        for (let a = 0; a < data_length - 4; ++a) {
          buffer.push(data.getInt8(payload_data_offset + a));
        }

        content = window.pako.inflate(new Uint8Array(buffer));
      } else {
        const buffer = [];
        for (let a = 0; a < data_length; ++a) {
          buffer.push(data.getInt8(payload_data_offset + a));
        }
        content = new Uint8Array(buffer);
      }

      rcc.push({
        name: path + fileName,
        buffer: decoder.decode(content),
      });
    }
  }

  readFolder('/', child_count, child);
  return rcc;
}

async function loadManifestIndex() {
  const data =
      await fetch(
          'https://mozilla-mobile.github.io/mozilla-vpn-client/addons/manifest.json')
          .then(r => r.json());

  const addons = [];
  for (const addon of data.addons) {
    const rcc = await loadAddon(
        `https://mozilla-mobile.github.io/mozilla-vpn-client/addons/${
            addon.id}.rcc`);
    addons.push({rcc, id: addon.id, ...readCompleteness(rcc)});
  }
  return addons;
}

async function show(addons, language) {
  const container = document.getElementById('container');

  while (container.firstChild) container.firstChild.remove();

  if (language) {
    addons = addons.sort((a, b) => {
      const aa = language in a.languages ? a.languages[language] : 0;
      const bb = language in b.languages ? b.languages[language] : 0;
      return aa < bb;
    });
  } else {
    addons = addons.sort((a, b) => a.avgCompleteness < b.avgCompleteness);
  }

  let divRow;
  for (let i = 0; i < addons.length; ++i) {
    const addon = addons[i];

    if (!(i % 3)) {
      divRow = document.createElement('div');
      divRow.setAttribute('class', 'row');
      container.appendChild(divRow);
    }

    const manifest =
        JSON.parse(addon.rcc.find(a => a.name === '/manifest.json').buffer);

    let threshold = 1.0;
    if (manifest.conditions && 'translation_threshold' in manifest.conditions) {
      threshold = manifest.conditions.translation_threshold;
    }

    const divCol = document.createElement('div');
    divCol.setAttribute('class', 'col');
    divRow.appendChild(divCol);

    const divCard = document.createElement('div');

    let completeness = addon.avgCompleteness;
    let cardStyle = 'text-bg-info';
    if (language) {
      completeness =
          language in addon.languages ? addon.languages[language] : 0;
      if (completeness < threshold) {
        cardStyle = 'text-bg-danger';
      }
    } else {
      const languageCount = Object.keys(addon.languages).length;
      const lowerThreshold = Object.keys(addon.languages)
                                 .filter(l => addon.languages[l] < threshold)
                                 .length;
      if (lowerThreshold > 0) {
        cardStyle = (lowerThreshold === languageCount) ? 'text-bg-danger' :
                                                         'text-bg-warning';
      }
    }

    divCard.setAttribute('class', `card mb-3 ${cardStyle}`);
    divCard.setAttribute('style', 'width: 20rem');
    divCol.appendChild(divCard);

    const divCardHeader = document.createElement('div');
    divCardHeader.setAttribute('class', 'card-header');
    divCardHeader.textContent = manifest.name;
    divCard.appendChild(divCardHeader);

    const divCardBody = document.createElement('div');
    divCardBody.setAttribute('class', 'card-body');
    divCard.appendChild(divCardBody);

    const divCardBodyContent = document.createElement('div');
    divCardBodyContent.setAttribute('class', 'table-responsive');
    divCardBody.appendChild(divCardBodyContent);

    const divCardContent = document.createElement('table');
    divCardContent.setAttribute('class', 'table table-sm');
    divCardBodyContent.appendChild(divCardContent);

    const divCardTBody = document.createElement('tbody');
    divCardContent.appendChild(divCardTBody);

    function addLine(table, a, b) {
      const tr = document.createElement('tr');
      divCardTBody.appendChild(tr);

      const tdA = document.createElement('td');
      tdA.textContent = a;
      tr.appendChild(tdA);

      const tdB = document.createElement('td');
      if (typeof (b) === 'string') {
        tdB.textContent = b;
      } else {
        tdB.appendChild(b);
      }

      tr.appendChild(tdB);
    }

    addLine(divCardTBody, 'Id:', manifest.id);
    addLine(divCardTBody, 'Threshold:', `${threshold * 100}%`);

    const completenessSpan = document.createElement('span');
    completenessSpan.appendChild(
        document.createTextNode(`${Math.round(completeness * 100)}%`));
    addLine(divCardTBody, 'Translations:', completenessSpan);

    if (manifest.conditions) {
      for (const condition of Object.keys(manifest.conditions)) {
        addLine(
            divCardTBody, `Condition ${condition}:`,
            JSON.stringify(manifest.conditions[condition]));
      }
    }

    if (Object.keys(addon.languages).length) {
      const divCollapseLink = document.createElement('a');
      divCollapseLink.setAttribute('class', 'btn btn-link')
      divCollapseLink.setAttribute('data-bs-toggle', 'collapse');
      divCollapseLink.setAttribute('href', `#collapse-${i}`);
      divCollapseLink.setAttribute('role', 'button');
      divCollapseLink.setAttribute('aria-expanded', 'false');
      divCollapseLink.setAttribute('aria-controls', `collapse-${i}`);
      completenessSpan.appendChild(divCollapseLink);

      const image = document.createElement('img')
      image.setAttribute('src', 'zoom-in.svg');
      divCollapseLink.appendChild(image);

      const divCollapse = document.createElement('div');
      divCollapse.setAttribute('class', 'collapse');
      divCollapse.setAttribute('id', `collapse-${i}`);
      divCardBodyContent.appendChild(divCollapse);

      const divCollapseBody = document.createElement('div');
      divCollapseBody.setAttribute('class', 'card card-body');
      divCollapse.appendChild(divCollapseBody);

      Object.keys(addon.languages).sort().forEach(language => {
        const languageP = document.createElement('p');
        languageP.textContent =
            `${language}: ${Math.round(addon.languages[language] * 100)}%`;
        divCollapseBody.appendChild(languageP);
      });
    }
  }
}

function readCompleteness(rcc) {
  const data = {
    languages: {},
    avgCompleteness: 0,
  };

  const translationCompleteness =
      rcc.find(a => a.name === '/i18n/translations.completeness');
  if (translationCompleteness) {
    let total = 0.0;
    translationCompleteness.buffer.split('\n')
        .map(a => a.split(':'))
        .forEach(a => {
          if (a[0]) {
            const value = parseFloat(a[1]);
            data.languages[a[0]] = value;
            total += value;
          }
        });
    data.avgCompleteness = total / Object.keys(data.languages).length;
  }

  return data;
}

function populateLanguages(addons) {
  const languages = [];
  addons.forEach(addon => {
    Object.keys(addon.languages)
        .forEach(l => languages.includes(l) || languages.push(l));
  });

  const languagesSelect = document.getElementById('languages');
  languagesSelect.onchange = () => {
    show(addons, languagesSelect.value);
  };

  languages.sort().forEach(l => {
    const languageOption = document.createElement('option');
    languageOption.textContent = l;
    languagesSelect.appendChild(languageOption);
  });
}

loadManifestIndex().then(r => {
  populateLanguages(r);
  show(r);
});
