import '@fontsource-variable/space-grotesk';
import '@fontsource/ibm-plex-mono/400.css';
import '@fontsource/ibm-plex-mono/500.css';
import './global.css';
import { Analytics } from "@vercel/analytics/next"

import { RootProvider } from 'fumadocs-ui/provider/next';
import type { Metadata } from 'next';
import type { ReactNode } from 'react';
import { JsonLd } from '@/components/json-ld';

const baseUrl = 'https://easy-rsh.vercel.app';

export const metadata: Metadata = {
  title: {
    default: 'Easy RSH — Remote shell, made legible',
    template: '%s | Easy RSH',
  },
  description:
    'Documentation for Easy RSH, a compact C++17 remote command server with OpenSSL authentication.',
  metadataBase: new URL(baseUrl),
  openGraph: {
    title: 'Easy RSH — Remote shell, made legible',
    description:
      'Documentation for Easy RSH, a compact C++17 remote command server with OpenSSL authentication.',
    url: baseUrl,
    siteName: 'Easy RSH',
    locale: 'en_US',
    type: 'website',
  },
  twitter: {
    card: 'summary_large_image',
    title: 'Easy RSH — Remote shell, made legible',
    description:
      'Documentation for Easy RSH, a compact C++17 remote command server with OpenSSL authentication.',
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
            url: baseUrl,
            description:
              'A compact C++17 remote command system that makes sockets, process management, authentication, and IPC easy to see and understand.',
            applicationCategory: 'DeveloperApplication',
            operatingSystem: 'Linux, POSIX',
          }}
        />
        <RootProvider theme={{ defaultTheme: 'dark' }}>{children}</RootProvider>
        <Analytics />
      </body>
    </html>
  );
}
