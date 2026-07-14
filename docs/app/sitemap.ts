import { source } from '@/lib/source';
import type { MetadataRoute } from 'next';

const baseUrl = 'https://easy-rsh.vercel.app';

export default function sitemap(): MetadataRoute.Sitemap {
  const pages = source.getPages();

  const docEntries: MetadataRoute.Sitemap = pages.map((page) => ({
    url: `${baseUrl}${page.url}`,
    lastModified: (page.data as { date?: string }).date ?? undefined,
    changeFrequency: 'weekly',
    priority: 0.8,
  }));

  return [
    {
      url: baseUrl,
      changeFrequency: 'monthly',
      priority: 1,
    },
    ...docEntries,
  ];
}
