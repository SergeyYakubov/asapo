/** @type {import('@docusaurus/types').DocusaurusConfig} */

const path = require('path');


module.exports = {
  title: 'ASAP::O',
  tagline: 'High performance distributed streaming platform',
  url: 'http://asapo.desy.de',
  baseUrl: '/',
  onBrokenLinks: 'warn',
  onBrokenMarkdownLinks: 'warn',
  favicon: 'img/favicon.ico',
  organizationName: 'DESY', // Usually your GitHub org/user name.
  projectName: 'ASAPO', // Usually your repo name.
   customFields: {
     version: '@ASAPO_VERSION@',
   },
  plugins: [path.resolve(__dirname, 'plugins/webpackconf/src/index.js')],
  themeConfig: {
    navbar: {
      logo: {
        alt: 'ASAPO Logo',
        src: 'img/logo.svg',
        srcDark: "img/logo_white.svg"
      },
      items: [
        {
          to: 'docs/',
          activeBasePath: 'docs',
          label: 'Docs',
          position: 'left',
        },
        {to: 'blog', label: 'Blog', position: 'left'},
        {
                  label: 'API',
                  position: 'left', // or 'right'
                  items: [
                    {
                      label: 'C++',
                      href: 'http://asapo.desy.de/cpp/',
                    },
                    {
                      label: 'Python',
                      href: 'http://asapo.desy.de/python/',
                    },
                  ],
                },
        {
          href: 'https://stash.desy.de/projects/ASAPO/repos/asapo/browse?at=@ASAPO_VERSION@/',
          label: 'BitBucket',
          title: 'BitBucket',
          position: 'right',
        },
      ],
    },
    footer: {
      style: 'dark',
      copyright: `Copyright © ${new Date().getFullYear()} DESY. Built with Docusaurus.`,
    },
  },
  presets: [
    [
      '@docusaurus/preset-classic',
      {
        docs: {
          sidebarPath: require.resolve('./sidebars.js'),
        },
        blog: {
          showReadingTime: true,
          // Please change this to your repo.
        },
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },
      },
    ],
  ],
};