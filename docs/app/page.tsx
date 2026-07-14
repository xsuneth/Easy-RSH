import {
  ArrowRight,
  Boxes,
  Braces,
  Network,
  ShieldCheck,
  Terminal,
} from 'lucide-react';
import Link from 'next/link';
import { Brand } from '@/components/brand';
import { TerminalPreview } from '@/components/terminal-preview';
import { JsonLd } from '@/components/json-ld';

function GitHubIcon({ size = 18 }: { size?: number }) {
  return (
    <svg
      aria-hidden="true"
      width={size}
      height={size}
      viewBox="0 0 24 24"
      fill="currentColor"
    >
      <path d="M12 .7C5.65.7.5 5.85.5 12.2c0 5.08 3.29 9.39 7.86 10.91.58.1.79-.25.79-.56v-2.23c-3.2.7-3.88-1.36-3.88-1.36-.52-1.33-1.28-1.68-1.28-1.68-1.05-.72.08-.7.08-.7 1.16.08 1.77 1.19 1.77 1.19 1.03 1.77 2.71 1.26 3.37.96.1-.75.4-1.26.73-1.55-2.56-.29-5.25-1.28-5.25-5.69 0-1.26.45-2.28 1.19-3.09-.12-.29-.52-1.46.11-3.05 0 0 .97-.31 3.16 1.18a10.98 10.98 0 0 1 5.75 0c2.19-1.49 3.16-1.18 3.16-1.18.63 1.59.23 2.76.11 3.05.74.81 1.19 1.83 1.19 3.09 0 4.42-2.7 5.39-5.27 5.68.41.36.78 1.06.78 2.14v3.24c0 .31.21.67.79.56a11.51 11.51 0 0 0 7.85-10.91C23.5 5.85 18.35.7 12 .7Z" />
    </svg>
  );
}

const features = [
  {
    icon: Network,
    number: '01',
    title: 'TCP, kept simple',
    text: 'A direct client-server protocol built on POSIX sockets, with clear connection and command flows.',
  },
  {
    icon: ShieldCheck,
    number: '02',
    title: 'Authenticated sessions',
    text: 'Salted SHA-256 credentials and cryptographically random session tokens powered by OpenSSL.',
  },
  {
    icon: Boxes,
    number: '03',
    title: 'Process isolation',
    text: 'Optional fork-per-client concurrency with automatic SIGCHLD cleanup and pipe-based output capture.',
  },
];

export default function HomePage() {
  return (
    <main className="landing">
      <JsonLd
        data={{
          '@type': 'SoftwareApplication',
          name: 'Easy RSH',
          applicationCategory: 'DeveloperApplication',
          operatingSystem: 'Linux, POSIX',
          description:
            'A compact C++17 remote command system that makes sockets, process management, authentication, and IPC easy to see and understand.',
          url: 'https://easy-rsh.vercel.app',
          softwareVersion: '1.1.0',
          sameAs: ['https://github.com/xsuneth/Easy-RSH'],
          codeRepository: 'https://github.com/xsuneth/Easy-RSH',
          mainEntityOfPage: 'https://easy-rsh.vercel.app',
          offers: {
            '@type': 'Offer',
            price: '0',
            priceCurrency: 'USD',
          },
          author: {
            '@type': 'Person',
            name: 'Suneth Chathuranga',
          },
        }}
      />
      <nav className="landing-nav">
        <Brand />
        <div className="nav-links">
          <Link href="/docs">Documentation</Link>
          <Link href="/docs/architecture">Architecture</Link>
          <a
            className="github-link"
            href="https://github.com/xsuneth/Easy-RSH"
            target="_blank"
            rel="noreferrer"
            aria-label="Easy RSH on GitHub"
          >
            <GitHubIcon size={18} />
            <span>GitHub</span>
          </a>
        </div>
      </nav>

      <section className="hero">
        <div className="hero-copy">
          <div className="eyebrow"><span /> Remote command execution, clarified</div>
          <h1>
            Your shell.
            <span>Anywhere.</span>
          </h1>
          <p className="hero-lede">
            A focused C++17 remote shell that turns sockets, processes, authentication,
            and command execution into one readable system.
          </p>
          <div className="hero-actions">
            <Link className="primary-button" href="/docs">
              Read the docs <ArrowRight size={18} />
            </Link>
            <a
              className="secondary-button"
              href="https://github.com/xsuneth/Easy-RSH"
              target="_blank"
              rel="noreferrer"
            >
              <GitHubIcon size={17} /> View source
            </a>
          </div>
          <div className="hero-meta">
            <span><Braces size={15} /> C++17</span>
            <span><Terminal size={15} /> POSIX</span>
            <span className="version-pill">v1.1.0</span>
          </div>
        </div>
        <div className="hero-terminal">
          <div className="terminal-glow" />
          <TerminalPreview />
        </div>
      </section>

      <section className="signal-strip" aria-label="Project capabilities">
        <span>OPENSSL AUTH</span>
        <span>FORK PER CLIENT</span>
        <span>PIPE + EXECVP</span>
        <span>SESSION TOKENS</span>
        <span>RAII SOCKETS</span>
      </section>

      <section className="feature-section">
        <div className="section-heading">
          <div>
            <span className="section-kicker">UNDER THE HOOD</span>
            <h2>Small surface.<br />Real systems work.</h2>
          </div>
          <p>
            Easy RSH is intentionally compact. Each subsystem is visible enough to study,
            modify, and reason about without hiding behind a framework.
          </p>
        </div>

        <div className="feature-grid">
          {features.map(({ icon: Icon, number, title, text }) => (
            <article className="feature-card" key={number}>
              <div className="feature-card-top">
                <Icon size={23} strokeWidth={1.7} />
                <span>{number}</span>
              </div>
              <h3>{title}</h3>
              <p>{text}</p>
            </article>
          ))}
        </div>
      </section>

      <section className="cta-section">
        <div>
          <span className="section-kicker">READY ON PORT 8080</span>
          <h2>From clone to connected in minutes.</h2>
        </div>
        <Link className="primary-button" href="/docs/installation">
          Start building <ArrowRight size={18} />
        </Link>
      </section>

      <footer className="landing-footer">
        <Brand compact />
        <p>Built to make operating-system concepts tangible.</p>
        <span>MIT LICENSE · 2026</span>
      </footer>
    </main>
  );
}
