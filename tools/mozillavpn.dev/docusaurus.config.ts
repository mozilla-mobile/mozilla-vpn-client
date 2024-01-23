import {themes as prismThemes} from 'prism-react-renderer';
import type {Config} from '@docusaurus/types';
import type * as Preset from '@docusaurus/preset-classic';
import resouces from './src/plugins/resources';


const config: Config = {
  title: 'Mozilla VPN Dev Portal',
  tagline: 'Dinosaurs are cool',
  favicon: 'res/logo.ico',

  url: 'https://mozillavpn.dev',
  baseUrl: '/',

  organizationName: 'mozilla-mobile',  // Usually your GitHub org/user name.
  projectName: 'mozilla-vpn-client',   // Usually your repo name.

  onBrokenLinks: 'throw',
  onBrokenMarkdownLinks: 'warn',
  markdown: {
    mermaid: true,
  },
  themes: ['@docusaurus/theme-mermaid'],
  

  i18n: {
    defaultLocale: 'en',
    locales: ['en'],
  },
  plugins: [resouces, require('docusaurus-lunr-search')],
  presets: [
    [
      'classic',
      {
        docs: {
          routeBasePath: '/',
          sidebarPath: './src/sidebars/default.ts',
          path: '../../docs',
          showLastUpdateTime:true,
        },
//        blog: {
//          showReadingTime: true,
//          // Please change this to your repo.
//          // Remove this to remove the "edit this page" links.
//          editUrl:
//              'https://github.com/facebook/docusaurus/tree/main/packages/create-docusaurus/templates/shared/',
//        },
        theme: {
          customCss: './src/css/custom.css',
        },
      } satisfies Preset.Options,
    ],
  ],

  themeConfig: {
    // Todo: Replace with your project's social card
    image: 'res/logo-generic.png',
    navbar: {
      title: '',
      logo: {
        alt: 'Mozilla VPN Logo',
        src: 'res/logo-generic.png',
      },
      items: [
        {to: '/', label: 'Docs', position: 'left'},
        {
          label: 'Tools',
          position: 'left',
          items: [
            {
              href: 'https://mozilla-mobile.github.io/mozilla-vpn-client/inspector/',
              label: 'Inspector',
              target: '_blank',
              rel: null,
            },
            {
              href: 'https://mozilla-mobile.github.io/mozilla-vpn-client/translationsreport/',
              label: 'Translationsreport',
              target: '_blank',
              rel: null,
            },
            {
              href: 'https://mozilla-mobile.github.io/mozilla-vpn-client/logviewer/',
              label: 'Logviewer',
              target: '_blank',
              rel: null,
            },
            {
              href: 'https://mozilla-mobile.github.io/mozilla-vpn-client/?branch=main',
              label: 'Wasm Client',
              target: '_blank',
              rel: null,
            },
          ],
          className: 'navbar__link--community',
        },
        {
          href: 'https://github.com/mozilla-mobile/mozilla-vpn-client',
          label: 'GitHub',
          position: 'right',
        },
      ],
    },
    footer: {
      style: 'dark',
      links: [],
      copyright: `MPL-2.0 Inc. Built with Docusaurus.`,
    },
  } satisfies Preset.ThemeConfig,
};

export default config;
