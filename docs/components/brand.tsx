import Link from 'next/link';

export function Brand({
  compact = false,
  linked = true,
}: {
  compact?: boolean;
  linked?: boolean;
}) {
  const content = (
    <>
      <span className="brand-prompt" aria-hidden="true">&gt;_</span>
      <span>EASY RSH</span>
    </>
  );

  const className = `brand ${compact ? 'brand-compact' : ''}`;

  return linked ? (
    <Link className={className} href="/">{content}</Link>
  ) : (
    <span className={className}>{content}</span>
  );
}
