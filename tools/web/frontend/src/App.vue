<script setup>
import {computed, reactive, onMounted} from "vue";
import SessionSetup from "./components/SessionSetup.vue";
import GameView from "./components/GameView.vue";
import {fetchState, joinSession, openSessionSocket} from "./api.js";
import {useToast} from "vue-toastification";

function parseQuery() {
  const u = new URL(window.location.href);
  return {
    sessionId: u.searchParams.get("session") || "",
    // side optional in link; join screen can override
    side: u.searchParams.get("side") || ""
  };
}

const app = reactive({
  sessionId: null,
  token: "",
  side: "", // "white" | "black"
  connected: false,
  ready: false,
  error: "",
  state: null,     // game state from backend
  analysis: {
    enabled: false,
    bestMove: null,   // {from,to}
    advantageWhite: 0,      // [-1..1] or any scalar; UI clamps nicely
    formattedScoreWhite: "",
    formattedScoreBlack: ""
  }
});

let ws = null;
const toast = useToast();

function applyMessage(msg) {
  if (!msg || !msg.type) return;
  if (msg.type === "state") {
    app.state = msg.payload;
  } else if (msg.type == "ready") {
    app.ready = true;
    toast.info("Opponent is ready")
  } else if (msg.type === "delta") {
    // simple shallow merge; adapt as needed
    app.state = {...(app.state || {}), ...(msg.payload || {})};
  } else if (msg.type === "analysis") {
    app.analysis = {...app.analysis, ...msg.payload};
  }
}

async function connect({sessionId, token, ready, side, analysisEnabled}) {
  app.error = "";
  app.sessionId = sessionId;
  app.side = side || "";
  app.token = token || "";
  app.ready = ready || false;
  app.analysis.enabled = !!analysisEnabled;

  // initial state
  try {
    const st = await fetchState({sessionId, token: app.token});
    app.state = st;
  } catch (e) {
    app.error = e?.message || "Failed to fetch state";
  }

  // ws live updates
  try {
    ws?.close?.();
    ws = openSessionSocket({
      sessionId,
      token: app.token,
      onOpen: () => (app.connected = true),
      onClose: () => (app.connected = false),
      onMessage: applyMessage,
      onError: () => (app.connected = false)
    });
  } catch {
    app.connected = false;
  }
}

async function joinFromLinkIfPossible() {
  const q = parseQuery();
  if (!q.sessionId) return;

  app.sessionId = q.sessionId;
  try {
    const j = await joinSession({sessionId: q.sessionId});
    await connect({
      sessionId: q.sessionId,
      token: j.token,
      side: j.side,
      ready: true,
      analysisEnabled: j.analysisEnabled ?? true
    });
  } catch (e) {
    app.error = e?.message || "Auto-join failed";
  }
}

onMounted(joinFromLinkIfPossible);

const inGame = computed(() =>
  !!app.token && !!app.state && !!app.ready
);

</script>

<template>
  <div class="container">
    <div class="header">
      <div class="brand">
        <div class="logo"></div>
        <div class="title">
          <h1>Kamisado Awase • Single Round</h1>
          <p>Live sessions • engine analysis • one-click join</p>
        </div>
      </div>

      <div class="row">
        <div class="kv">
          <div class="k">Connection</div>
          <div class="v">
            <span :style="{color: app.connected ? 'rgba(80,220,160,1)' : 'rgba(255,120,120,1)'}">
              {{ app.connected ? "LIVE" : "OFF" }}
            </span>
          </div>
        </div>
        <div class="kv" v-if="sessionId">
          <div class="k">Session</div>
          <div class="v">{{ sessionId }}</div>
        </div>
      </div>
    </div>

    <Transition name="fade-slide">
      <div v-if="error" class="card" style="margin-bottom:14px">
        <div class="card-body">
          <div style="display:flex; align-items:center; justify-content:space-between; gap:12px">
            <div>
              <div style="font-weight:800">Something went wrong</div>
              <div class="small">{{ error }}</div>
            </div>
            <button class="btn danger" @click="error = ''">Dismiss</button>
          </div>
        </div>
      </div>
    </Transition>

    <SessionSetup v-if="!inGame" :prefillSessionId="app.sessionId" @connected="connect" />

    <GameView v-else :sessionId="app.sessionId" :token="app.token" :side="app.side" :state="app.state"
      :analysis="app.analysis" />
  </div>
</template>
