import '@fontsource-variable/space-grotesk';
import '@fontsource/ibm-plex-mono/400.css';
import '@fontsource/ibm-plex-mono/500.css';
import './global.css';
import { Analytics } from "@vercel/analytics/next"

import { RootProvider } from 'fumadocs-ui/provider/next';
import type { Metadata } from 'next';
import type { ReactNode } from 'react';

export const metadata: Metadata = {
  title: {
    default: 'Easy RSH — Remote shell, made legible',
    template: '%s | Easy RSH',
  },
  description:
    'Documentation for Easy RSH, a compact C++17 remote command server with OpenSSL authentication.',
};

export default function RootLayout({ children }: { children: ReactNode }) {
  return (
    <html lang="en" suppressHydrationWarning>
      <body>
        <RootProvider theme={{ defaultTheme: 'dark' }}>{children}</RootProvider>\
        <Analytics/>
      </body>
    </html>
  );
}
