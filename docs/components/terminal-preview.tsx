export function TerminalPreview() {
  return (
    <div className="terminal-shell" aria-label="Easy RSH server terminal preview">
      <div className="terminal-topbar">
        <div className="terminal-dots" aria-hidden="true">
          <span />
          <span />
          <span />
        </div>
        <span className="terminal-title">server@easy-rsh:~</span>
        <span className="terminal-status">LIVE</span>
      </div>
      <div className="terminal-body">
        <div className="ascii-logo" aria-hidden="true">
          <span>EASY</span>
          <span>RSH</span>
        </div>
        <p className="terminal-version">Easy Remote Shell Server v1.1.0</p>
        <div className="terminal-output">
          <p><span className="muted">Loaded</span> 1 user from data/users.txt</p>
          <p><span className="muted">Mode:</span> Multi-client (fork)</p>
          <p><span className="muted">Mode:</span> Command execution</p>
          <p className="accent">Server listening on port <mark>8080</mark></p>
          <p><span className="muted">Network:</span> 10.255.255.254:8080</p>
          <p><span className="muted">Local:</span> 127.0.0.1:8080</p>
          <br />
          <p><span className="muted">Connect:</span> ./client 10.255.255.254 8080</p>
          <br />
          <p>SIGCHLD handler installed for zombie reaping</p>
          <p>Waiting for connection <span className="cursor" /></p>
        </div>
      </div>
    </div>
  );
}
