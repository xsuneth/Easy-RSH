import { docs } from 'collections/server';
import { loader } from 'fumadocs-core/source';
import { createElement } from 'react';
import {
  BookOpen,
  Download,
  Network,
  Plug,
  Rocket,
  Settings2,
  ShieldCheck,
  SquareTerminal,
} from 'lucide-react';

const pageIcons = {
  BookOpen,
  Download,
  Network,
  Plug,
  Rocket,
  Settings2,
  ShieldCheck,
  SquareTerminal,
} as const;

export const source = loader({
  baseUrl: '/docs',
  source: docs.toFumadocsSource(),
  icon(icon) {
    if (!icon) return undefined;

    const Icon = pageIcons[icon as keyof typeof pageIcons];
    return Icon ? createElement(Icon) : undefined;
  },
});
