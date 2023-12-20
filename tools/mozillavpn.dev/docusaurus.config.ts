import {themes as prismThemes} from 'prism-react-renderer';
import type {Config} from '@docusaurus/types';
import type * as Preset from '@docusaurus/preset-classic';
import inspector from './src/plugins/inspector';
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

  i18n: {
    defaultLocale: 'en',
    locales: ['en'],
  },
  plugins: [inspector, resouces],
  presets: [
    [
      'classic',
      {
        docs: {
          sidebarPath: './sidebars.ts',
          path: '../../docs',

        },
        blog: {
          showReadingTime: true,
          // Please change this to your repo.
          // Remove this to remove the "edit this page" links.
          editUrl:
              'https://github.com/facebook/docusaurus/tree/main/packages/create-docusaurus/templates/shared/',
        },
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
      title: 'Mozilla VPN Docs',
      logo: {
        alt: 'Mozilla VPN Logo',
        src: 'res/logo-generic.png',
      },
      items: [
        {to: '/docs', label: 'Docs', position: 'left'},
        {to: '/blog', label: 'Blog', position: 'left'},
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
