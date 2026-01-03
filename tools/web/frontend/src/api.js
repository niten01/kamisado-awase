const API_BASE = "https://exception-communications-slide-attended.trycloudflare.com";

export async function createSession({analysisEnabled}) {
  const res = await fetch(`${API_BASE}/api/sessions`, {
    method: "POST",
    headers: {"Content-Type": "application/json"},
    body: JSON.stringify({analysisEnabled})
  });
  if (!res.ok) throw new Error(`createSession failed: ${res.status}`);
  return await res.json();
}

// Join as a given side; backend returns a token used for moves/ws auth.
export async function joinSession({sessionId, side}) {
  const res = await fetch(`${API_BASE}/api/sessions/${sessionId}/join`, {
    method: "POST",
    headers: {"Content-Type": "application/json"},
    body: side ? JSON.stringify({side}) : undefined
  });
  if (!res.ok) throw new Error(`joinSession failed: ${res.status}`);
  return await res.json();
}

export async function fetchState({sessionId, token}) {
  const res = await fetch(`${API_BASE}/api/sessions/${sessionId}/state`, {
    headers: token ? {Authorization: `Bearer ${token}`} : {}
  });
  if (!res.ok) throw new Error(`fetchState failed: ${res.status}`);
  return await res.json();
}

export async function sendMove({sessionId, token, from, to}) {
  const res = await fetch(`${API_BASE}/api/sessions/${sessionId}/move`, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
      ...(token ? {Authorization: `Bearer ${token}`} : {})
    },
    body: JSON.stringify({from, to})
  });
  if (!res.ok) throw new Error(`sendMove failed: ${res.status}`);
  return await res.json();
}

/**
 * WebSocket messages expected (examples):
 *  - { type:"state", payload:{...fullState} }
 *  - { type:"delta", payload:{...partial} }  (optional)
 *  - { type:"analysis", payload:{ bestMove:{from:"a1",to:"a2"}, advantage:0.35 } }
 *  - { type:"terminal", payload:{ status:"win", winner:"white", reason:"..." } }
 */
export function openSessionSocket({sessionId, token, onMessage, onOpen, onClose, onError}) {
  const qs = token ? `?token=${encodeURIComponent(token)}&sid=${sessionId}` : "";
  const ws = new WebSocket(`${API_BASE}/ws/sessions${qs}`);

  ws.onopen = () => onOpen?.();
  ws.onclose = () => onClose?.();
  ws.onerror = (e) => onError?.(e);

  ws.onmessage = (evt) => {
    try {
      const msg = JSON.parse(evt.data);
      onMessage?.(msg);
    } catch {
      // ignore malformed
    }
  };

  return ws;
}

