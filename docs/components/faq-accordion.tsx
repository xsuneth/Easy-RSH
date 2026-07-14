'use client';

import { useEffect, useId, useRef, useState } from 'react';

type FAQItem = {
  question: string;
  answer: string;
};

type FAQAccordionProps = {
  items: FAQItem[];
};

function FAQAccordionItem({ answer, question }: FAQItem) {
  const [open, setOpen] = useState(false);
  const [height, setHeight] = useState(0);
  const contentRef = useRef<HTMLDivElement>(null);
  const panelId = useId();

  useEffect(() => {
    const element = contentRef.current;
    if (!element) return;

    const updateHeight = () => {
      setHeight(element.scrollHeight);
    };

    updateHeight();

    if (typeof ResizeObserver === 'undefined') return;

    const observer = new ResizeObserver(updateHeight);
    observer.observe(element);

    return () => observer.disconnect();
  }, []);

  return (
    <div className={`faq-item${open ? ' is-open' : ''}`}>
      <button
        type="button"
        className="faq-trigger"
        aria-expanded={open}
        aria-controls={panelId}
        onClick={() => setOpen((current) => !current)}
      >
        <span>{question}</span>
      </button>
      <div
        id={panelId}
        className="faq-panel"
        style={{ height: open ? `${height}px` : '0px' }}
      >
        <div className="faq-panel-inner" ref={contentRef}>
          <p>{answer}</p>
        </div>
      </div>
    </div>
  );
}

export function FAQAccordion({ items }: FAQAccordionProps) {
  return (
    <div className="faq-list">
      {items.map((item) => (
        <FAQAccordionItem key={item.question} {...item} />
      ))}
    </div>
  );
}
