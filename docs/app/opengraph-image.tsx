import { ImageResponse } from 'next/og';

export const contentType = 'image/png';
export const size = { width: 1200, height: 630 };

export default function Image() {
  return new ImageResponse(
    (
      <div
        style={{
          width: '100%',
          height: '100%',
          display: 'flex',
          flexDirection: 'column',
          alignItems: 'center',
          justifyContent: 'center',
          background: '#17191b',
          color: '#f2eff7',
          fontFamily: '"Space Grotesk", sans-serif',
        }}
      >
        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: '16px',
            marginBottom: '24px',
          }}
        >
          <span
            style={{
              fontSize: '48px',
              color: '#a77af2',
              fontWeight: 700,
            }}
          >
            &gt;_
          </span>
          <span
            style={{
              fontSize: '64px',
              fontWeight: 700,
              letterSpacing: '4px',
            }}
          >
            EASY RSH
          </span>
        </div>
        <p
          style={{
            fontSize: '28px',
            color: '#9c98a6',
            margin: 0,
          }}
        >
          Remote shell, made legible
        </p>
        <div
          style={{
            marginTop: '40px',
            display: 'flex',
            gap: '16px',
            fontSize: '18px',
            color: '#746b82',
          }}
        >
          <span>C++17</span>
          <span>POSIX</span>
          <span>OpenSSL</span>
          <span>Fork-per-client</span>
        </div>
      </div>
    ),
    {
      width: 1200,
      height: 630,
    },
  );
}
