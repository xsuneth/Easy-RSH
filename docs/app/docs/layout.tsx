import { source } from '@/lib/source';
import { baseOptions } from '@/lib/layout.shared';
import { DocsLayout } from 'fumadocs-ui/layouts/docs';
import type { ReactNode } from 'react';
import { JsonLd } from '@/components/json-ld';

const baseUrl = 'https://easy-rsh.vercel.app';

export default function Layout({ children }: { children: ReactNode }) {
  return (
    <DocsLayout
      {...baseOptions()}
      tree={source.getPageTree()}
      sidebar={{
        defaultOpenLevel: 1,
      }}
    >
      <JsonLd
        data={{
          '@type': 'BreadcrumbList',
          itemListElement: [
            { '@type': 'ListItem', position: 1, name: 'Home', item: baseUrl },
            {
              '@type': 'ListItem',
              position: 2,
              name: 'Documentation',
              item: `${baseUrl}/docs`,
            },
          ],
        }}
      />
      {children}
    </DocsLayout>
  );
}
