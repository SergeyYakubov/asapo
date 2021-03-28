/** @type {import('@docusaurus/types').DocusaurusConfig} */
module.exports = {
  title: 'ASAP::O',
  tagline: 'High performance distributed streaming platform',
  url: 'https://asapo.desy.de',
  baseUrl: '/',
  onBrokenLinks: 'warn',
  onBrokenMarkdownLinks: 'warn',
  favicon: 'img/favicon.ico',
  organizationName: 'DESY', // Usually your GitHub org/user name.
  projectName: 'ASAPO', // Usually your repo name.
   customFields: {
     version: '@ASAPO_VERSION@',
   },
  themeConfig: {
    navbar: {
      title: 'ASAPO',
      logo: {
        alt: 'ASAPO Logo',
        src: 'img/logo.svg',
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
          href: 'https://stash.desy.de/projects/ASAPO/repos/asapo/browse?at=@ASAPO_VERSION@/',
          label: 'BitBucket',
          position: 'right',
        },
        {
                  label: 'API',
                  position: 'left', // or 'right'
                  items: [
                    {
                      label: 'C++',
                      href: 'http://os-46-asapo-docs.desy.de/cpp/',
                    },
                    {
                      label: 'Python',
                      href: 'http://os-46-asapo-docs.desy.de/python/',
                    },
                  ],
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
