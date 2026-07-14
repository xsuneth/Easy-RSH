'use client';

import { useState, type ComponentType } from 'react';
import { buttonVariants } from 'fumadocs-ui/components/ui/button';
import { Popover, PopoverContent, PopoverTrigger } from 'fumadocs-ui/components/ui/popover';
import { RiClaudeFill, RiMicrosoftCopilotFill } from 'react-icons/ri';
import { SiCursor, SiGithub, SiVercel } from 'react-icons/si';
import { TbBrandOpenai } from 'react-icons/tb';
import {
  ChevronDown,
  Clipboard,
  Copy,
  ExternalLink,
} from 'lucide-react';

type DocsOpenInActionsProps = {
  markdownUrl: string;
  pageTitle: string;
  pageUrl: string;
};

type Provider = {
  name: string;
  url: string;
  icon: ComponentType<{ className?: string }>;
};

const providers: Provider[] = [
  {
    name: 'GitHub',
    url: 'https://github.com/',
    icon: SiGithub,
  },
  {
    name: 'ChatGPT',
    url: 'https://chatgpt.com/',
    icon: TbBrandOpenai,
  },
  {
    name: 'Claude',
    url: 'https://claude.ai/',
    icon: RiClaudeFill,
  },
  {
    name: 'T3 Chat',
    url: 'https://t3.chat/',
    icon: SiVercel,
  },
  {
    name: 'Copilot',
    url: 'https://copilot.microsoft.com/',
    icon: RiMicrosoftCopilotFill,
  },
  {
    name: 'Cursor',
    url: 'https://cursor.com/',
    icon: SiCursor,
  },
];

export function DocsOpenInActions({
  markdownUrl,
  pageTitle,
  pageUrl,
}: DocsOpenInActionsProps) {
  const [copied, setCopied] = useState(false);
  const [copiedProvider, setCopiedProvider] = useState<string | null>(null);

  const copyText = async (text: string, onDone: () => void) => {
    try {
      await navigator.clipboard.writeText(text);
      onDone();
    } catch {
      onDone();
    }
  };

  const handleCopyMarkdown = async () => {
    await copyText(markdownUrl, () => {
      setCopied(true);
      window.setTimeout(() => setCopied(false), 1800);
    });
  };

  const handleOpenIn = async (provider: Provider) => {
    const prompt =
      `Read this documentation page and help me with it.\n\n` +
      `Title: ${pageTitle}\n` +
      `Docs URL: ${pageUrl}\n` +
      `Markdown source: ${markdownUrl}\n\n` +
      `Please use the documentation content as the primary context.`;

    await copyText(prompt, () => {
      setCopiedProvider(provider.name);
      window.setTimeout(() => setCopiedProvider(null), 1800);
    });

    window.open(provider.url, '_blank', 'noopener,noreferrer');
  };

  return (
    <div className="docs-open-in-actions">
      <button
        type="button"
        className={`${buttonVariants({ color: 'ghost', size: 'sm' })} docs-action-button`}
        onClick={handleCopyMarkdown}
        aria-label="Copy Markdown link"
      >
        <Copy className="size-4" />
        {copied ? 'Copied MD' : 'Copy MD'}
      </button>

      <Popover>
        <PopoverTrigger
          className={`${buttonVariants({ color: 'ghost', size: 'sm' })} docs-action-button`}
        >
          Open in
          <ChevronDown className="size-4" />
        </PopoverTrigger>
        <PopoverContent align="end" className="docs-open-in-menu">
          <div className="docs-open-in-list">
            {providers.map((provider) => {
              const isCopied = copiedProvider === provider.name;
              const Icon = provider.icon;

              return (
                <button
                  key={provider.name}
                  type="button"
                  className="docs-open-in-item"
                  onClick={() => handleOpenIn(provider)}
                >
                  <span className="docs-open-in-item-main">
                    {isCopied ? (
                      <Clipboard className="size-4" />
                    ) : (
                      <Icon className="docs-open-in-provider-icon" />
                    )}
                    <span>{isCopied ? 'Prompt copied' : provider.name}</span>
                  </span>
                  <ExternalLink className="size-4 docs-open-in-item-arrow" />
                </button>
              );
            })}
          </div>
        </PopoverContent>
      </Popover>
    </div>
  );
}
