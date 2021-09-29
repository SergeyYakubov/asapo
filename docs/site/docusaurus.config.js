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
        {to: 'blog', label: 'Changelog', position: 'left'},
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
          type: 'docsVersionDropdown',
          //// Optional
          position: 'right',
          // Add additional dropdown items at the beginning/end of the dropdown.
          dropdownItemsBefore: [],
          dropdownItemsAfter: [],
          dropdownActiveClassDisabled: true,
          docsPluginId: 'default',
        },
        {
          href: 'https://stash.desy.de/projects/ASAPO/repos/asapo/browse/',
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
            versions: {
              current: {
                "label": "Develop",
                "path": "next"
              },
              },
        },
        blog: {
          showReadingTime: false,
          path: 'changelog',
          blogSidebarTitle: 'Versions'
        },
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },
      },
    ],
  ],
};
