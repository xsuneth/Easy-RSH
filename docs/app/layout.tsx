import '@fontsource-variable/space-grotesk';
import '@fontsource/ibm-plex-mono/400.css';
import '@fontsource/ibm-plex-mono/500.css';
import './global.css';

import { RootProvider } from 'fumadocs-ui/provider/next';
import type { Metadata } from 'next';
import type { ReactNode } from 'react';
import { JsonLd } from '@/components/json-ld';

const baseUrl = 'https://easy-rsh.vercel.app';

export const metadata: Metadata = {
  metadataBase: new URL(baseUrl),
  title: {
    default: 'Easy RSH — Remote shell, made legible',
    template: '%s | Easy RSH',
  },
  description:
    'Documentation for Easy RSH, a compact C++17 remote command server with OpenSSL authentication.',
  applicationName: 'Easy RSH',
  generator: 'Next.js',
  keywords: [
    'remote shell',
    'C++17',
    'OpenSSL',
    'remote command execution',
    'POSIX',
    'client-server',
    'documentation',
  ],
  authors: [{ name: 'Suneth Chathuranga', url: 'https://github.com/xsuneth' }],
  creator: 'Suneth Chathuranga',
  publisher: 'Suneth Chathuranga',
  alternates: {
    canonical: baseUrl,
  },
  openGraph: {
    title: 'Easy RSH — Remote shell, made legible',
    description:
      'Documentation for Easy RSH, a compact C++17 remote command server with OpenSSL authentication.',
    url: baseUrl,
    siteName: 'Easy RSH',
    locale: 'en_US',
    type: 'website',
    images: '/opengraph-image',
  },
  twitter: {
    card: 'summary_large_image',
    title: 'Easy RSH — Remote shell, made legible',
    description:
      'Documentation for Easy RSH, a compact C++17 remote command server with OpenSSL authentication.',
    images: '/opengraph-image',
  },
};

export default function RootLayout({ children }: { children: ReactNode }) {
  return (
    <html lang="en" suppressHydrationWarning>
      <body>
        <JsonLd
          data={{
            '@type': 'WebSite',
            name: 'Easy RSH',
            alternateName: 'Easy Remote Shell',
            url: baseUrl,
            description:
              'Documentation for Easy RSH, a compact C++17 remote command server with OpenSSL authentication.',
            about: {
              '@type': 'SoftwareApplication',
              name: 'Easy RSH',
              applicationCategory: 'DeveloperApplication',
              operatingSystem: 'Linux, POSIX',
            },
          }}
        />
        <RootProvider theme={{ defaultTheme: 'dark' }}>{children}</RootProvider>
      </body>
    </html>
  );
}
