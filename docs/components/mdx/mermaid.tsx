'use client';

import { useEffect, useId, useState } from 'react';

type RenderedDiagram = {
  bindFunctions?: (element: Element) => void;
  svg: string;
};

const renderCache = new Map<string, Promise<RenderedDiagram>>();

const lightTheme = {
  background: '#f3eff9',
  primaryColor: '#ffffff',
  primaryTextColor: '#1c1526',
  primaryBorderColor: '#a77af2',
  lineColor: '#7f57c6',
  secondaryColor: '#efebf6',
  secondaryTextColor: '#1c1526',
  secondaryBorderColor: '#5a79cf',
  tertiaryColor: '#f6f4fb',
  tertiaryTextColor: '#1c1526',
  tertiaryBorderColor: '#bba8dd',
  noteBkgColor: '#ffffff',
  noteTextColor: '#1c1526',
  noteBorderColor: '#5a79cf',
  actorBkg: '#ffffff',
  actorBorder: '#a77af2',
  actorTextColor: '#1c1526',
  signalColor: '#7f57c6',
  signalTextColor: '#1c1526',
  labelBoxBkgColor: '#ffffff',
  labelBoxBorderColor: '#5a79cf',
  labelTextColor: '#1c1526',
  loopTextColor: '#1c1526',
  activationBkgColor: '#e5dbf7',
  activationBorderColor: '#a77af2',
};

const darkTheme = {
  background: '#17191b',
  primaryColor: '#24202d',
  primaryTextColor: '#f3f0f8',
  primaryBorderColor: '#a77af2',
  lineColor: '#a77af2',
  secondaryColor: '#1d1f22',
  secondaryTextColor: '#f3f0f8',
  secondaryBorderColor: '#7895e8',
  tertiaryColor: '#17191b',
  tertiaryTextColor: '#f3f0f8',
  tertiaryBorderColor: '#4f3a6d',
  noteBkgColor: '#24202d',
  noteTextColor: '#f3f0f8',
  noteBorderColor: '#7895e8',
  actorBkg: '#24202d',
  actorBorder: '#a77af2',
  actorTextColor: '#f3f0f8',
  signalColor: '#c6a8ff',
  signalTextColor: '#f3f0f8',
  labelBoxBkgColor: '#1d1f22',
  labelBoxBorderColor: '#7895e8',
  labelTextColor: '#f3f0f8',
  loopTextColor: '#f3f0f8',
  activationBkgColor: '#4f3a6d',
  activationBorderColor: '#a77af2',
};

function getTheme(mode: 'light' | 'dark') {
  return mode === 'dark' ? darkTheme : lightTheme;
}

const mermaidReady = import('mermaid').then(({ default: mermaid }) => mermaid);

function renderDiagram(id: string, chart: string, mode: 'light' | 'dark') {
  const key = `${mode}:${id}:${chart}`;
  const cached = renderCache.get(key);
  if (cached) return cached;

  const promise = mermaidReady.then((mermaid) => {
    mermaid.initialize({
      startOnLoad: false,
      securityLevel: 'strict',
      theme: 'base',
      fontFamily: 'IBM Plex Mono, monospace',
      flowchart: { curve: 'basis', useMaxWidth: true },
      sequence: { useMaxWidth: true },
      themeVariables: getTheme(mode),
    });

    return mermaid.render(`mermaid-${mode}-${id}`, chart.replaceAll('\\n', '\n'));
  });

  renderCache.set(key, promise);
  return promise;
}

export function Mermaid({ chart }: { chart: string }) {
  const id = useId().replaceAll(':', '');
  const [diagram, setDiagram] = useState<RenderedDiagram>();
  const [failed, setFailed] = useState(false);
  const [mode, setMode] = useState<'light' | 'dark'>('dark');

  useEffect(() => {
    const root = document.documentElement;
    const syncTheme = () => {
      setMode(root.classList.contains('dark') ? 'dark' : 'light');
    };

    syncTheme();

    const observer = new MutationObserver(syncTheme);
    observer.observe(root, { attributes: true, attributeFilter: ['class'] });

    return () => {
      observer.disconnect();
    };
  }, []);

  useEffect(() => {
    let active = true;
    setFailed(false);
    setDiagram(undefined);

    renderDiagram(id, chart, mode)
      .then((result) => {
        if (active) setDiagram(result);
      })
      .catch(() => {
        if (active) setFailed(true);
      });

    return () => {
      active = false;
    };
  }, [chart, id, mode]);

  if (failed) {
    return <pre className="mermaid-fallback">{chart}</pre>;
  }

  if (!diagram) {
    return <div className="mermaid-loading">Rendering diagram...</div>;
  }

  return (
    <div
      className="mermaid-diagram"
      role="img"
      aria-label="Architecture diagram"
      ref={(element) => {
        if (element) diagram.bindFunctions?.(element);
      }}
      dangerouslySetInnerHTML={{ __html: diagram.svg }}
    />
  );
}
