module.exports = {
  docs: [
    'getting-started',
    'overview',
    'compare-to-others',
    {
      type: 'category',
      label: 'Concepts And Architecture',
      items: [
        'data-in-asapo',
        'producer-clients',
        'consumer-clients',
        'core-architecture',
      ],
    },
    {
      type: 'category',
      label: 'Use Cases',
      items: [
        'p02.1',
      ],
    },
    {
      type: 'category',
      label: 'Code Examples',
      items: [
        'cookbook/overview',
        'cookbook/simple-producer',
        'cookbook/simple-consumer',
        'cookbook/simple-pipeline',
        'cookbook/datasets',
        'cookbook/acknowledgements',
        'cookbook/metadata',
        'cookbook/next_stream',
        'cookbook/query'
      ]
    }
  ],
};
