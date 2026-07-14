import { getMDXComponents } from '@/components/mdx';
import { DocsOpenInActions } from '@/components/docs-open-in-actions';
import { source } from '@/lib/source';
import { JsonLd } from '@/components/json-ld';
import {
  DocsBody,
  DocsDescription,
  DocsPage,
  DocsTitle,
} from 'fumadocs-ui/layouts/docs/page';
import type { Metadata } from 'next';
import { notFound } from 'next/navigation';

const baseUrl = 'https://easy-rsh.vercel.app';

export default async function Page(props: {
  params: Promise<{ slug?: string[] }>;
}) {
  const params = await props.params;
  const page = source.getPage(params.slug);
  if (!page) notFound();

  const MDX = page.data.body;
  const pageDate = (page.data as { date?: string }).date;
  const pageUrl = `${baseUrl}${page.url}`;
  const markdownPath = params.slug?.length
    ? `${params.slug.join('/')}.mdx`
    : 'index.mdx';
  const markdownUrl =
    `https://github.com/xsuneth/Easy-RSH/blob/main/docs/content/docs/${markdownPath}`;

  return (
    <DocsPage toc={page.data.toc}>
      <JsonLd
        data={{
          '@type': 'TechArticle',
          headline: page.data.title,
          description: page.data.description,
          url: pageUrl,
          mainEntityOfPage: pageUrl,
          author: {
            '@type': 'Person',
            name: 'Suneth Chathuranga',
            url: 'https://github.com/xsuneth',
          },
          publisher: {
            '@type': 'Person',
            name: 'Suneth Chathuranga',
            url: 'https://github.com/xsuneth',
          },
          ...(pageDate && {
            datePublished: pageDate,
            dateModified: pageDate,
          }),
        }}
      />
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
            {
              '@type': 'ListItem',
              position: 3,
              name: page.data.title,
              item: pageUrl,
            },
          ],
        }}
      />
      <div className="docs-page-actions-row">
        <DocsOpenInActions
          markdownUrl={markdownUrl}
          pageTitle={page.data.title}
          pageUrl={pageUrl}
        />
      </div>
      <DocsTitle>{page.data.title}</DocsTitle>
      {page.data.description && (
        <DocsDescription>{page.data.description}</DocsDescription>
      )}
      <DocsBody>
        <MDX components={getMDXComponents()} />
      </DocsBody>
    </DocsPage>
  );
}

export function generateStaticParams() {
  return source.generateParams();
}

export async function generateMetadata(props: {
  params: Promise<{ slug?: string[] }>;
}): Promise<Metadata> {
  const params = await props.params;
  const page = source.getPage(params.slug);
  if (!page) notFound();
  const pageDate = (page.data as { date?: string }).date;

  return {
    title: page.data.title,
    description: page.data.description,
    alternates: {
      canonical: `${baseUrl}${page.url}`,
    },
    openGraph: {
      title: page.data.title,
      description: page.data.description,
      url: `${baseUrl}${page.url}`,
      type: 'article',
      images: '/opengraph-image',
      ...(pageDate && {
        publishedTime: pageDate,
        modifiedTime: pageDate,
      }),
    },
    twitter: {
      card: 'summary_large_image',
      title: page.data.title,
      description: page.data.description,
      images: '/opengraph-image',
    },
  };
}
