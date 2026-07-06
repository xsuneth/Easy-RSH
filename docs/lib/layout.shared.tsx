import { Brand } from '@/components/brand';
import type { BaseLayoutProps } from 'fumadocs-ui/layouts/shared';

export function baseOptions(): BaseLayoutProps {
  return {
    nav: {
      title: <Brand compact linked={false} />,
      url: '/',
    },
    githubUrl: 'https://github.com/xsuneth/Easy-RSH',
  };
}
