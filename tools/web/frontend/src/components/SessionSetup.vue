<script setup>
import {computed, reactive} from "vue";
import {createSession, joinSession} from "../api.js";
import {useToast} from "vue-toastification";

const toast = useToast();

const props = defineProps({
  prefillSessionId: {type: String, default: ""}
});
const emit = defineEmits(["connected"]);

const ui = reactive({
  mode: props.prefillSessionId ? "join" : "create", // "create" | "join"
  analysisEnabled: false,
  side: "random",
  sessionId: props.prefillSessionId || "",
  shareLink: "",
  busy: false,
  info: ""
});

const canJoin = computed(() => ui.sessionId && (ui.side === "white" || ui.side === "black"));

function buildLink(sessionId) {
  const u = new URL(window.location.href);
  u.searchParams.set("session", sessionId);
  return u.toString();
}

async function doCreate() {
  ui.busy = true;
  ui.info = "";
  try {
    const created = await createSession({analysisEnabled: ui.analysisEnabled});
    const sessionId = created.sessionId;
    ui.sessionId = sessionId;

    // creator joins immediately as chosen side
    const joined = await joinSession({sessionId, side: ui.side === "random" ? undefined : ui.side});

    ui.shareLink = buildLink(sessionId);
    ui.info = "Session created. Share the link so the opponent can join.";

    toast.success("Session created, waiting for opponent")

    emit("connected", {
      sessionId,
      side: joined.side,
      token: joined.token,
      analysisEnabled: created.analysisEnabled ?? ui.analysisEnabled
    });
  } catch (e) {
    ui.info = e?.message || "Create failed";
  } finally {
    ui.busy = false;
  }
}

async function doJoin() {
  ui.busy = true;
  ui.info = "";
  try {
    const joined = await joinSession({sessionId: ui.sessionId});
    emit("connected", {
      sessionId: ui.sessionId,
      side: ui.side,
      token: joined.token,
      analysisEnabled: joined.analysisEnabled ?? true
    });
  } catch (e) {
    ui.info = e?.message || "Join failed";
  } finally {
    ui.busy = false;
  }
}

async function copyLink() {
  try {
    await navigator.clipboard.writeText(ui.shareLink);
    ui.info = "Copied link to clipboard.";
  } catch {
    ui.info = "Couldn’t copy automatically — copy it manually.";
  }
}
</script>

<template>
  <div class="card">
    <div class="card-body">
      <div class="panel-title">
        <div>
          <div style="font-weight:900; font-size:16px">Session</div>
          <div class="small">Create a room or join an existing one.</div>
        </div>
        <div class="row">
          <button class="btn" :class="{primary: ui.mode === 'create'}" @click="ui.mode = 'create'">
            Create
          </button>
          <button class="btn" :class="{primary: ui.mode === 'join'}" @click="ui.mode = 'join'">
            Join
          </button>
        </div>
      </div>

      <div v-if="ui.mode === 'create'" class="grid" style="grid-template-columns: 1fr 1fr; gap:12px">
        <div class="kv">
          <div class="k">Side</div>
          <div class="v">
            <select class="input" v-model="ui.side">
              <option value="random">Random</option>
              <option value="white">White</option>
              <option value="black">Black</option>
            </select>
          </div>
        </div>

        <div class="kv">
          <div class="k">LIVE analysis</div>
          <div class="v">
            <label style="display:flex; align-items:center; gap:10px; cursor:pointer">
              <input type="checkbox" v-model="ui.analysisEnabled" />
              <span class="small">{{ ui.analysisEnabled ? "On" : "Off" }}</span>
            </label>
          </div>
        </div>
      </div>

      <Transition name="fade-slide">
        <div v-if="ui.mode === 'join'" style="margin-top: 12px">
          <div class="small" style="margin-bottom:6px">Session ID</div>
          <input class="input" v-model="ui.sessionId" placeholder="e.g. 123" />
        </div>
      </Transition>

      <div class="row" style="margin-top: 12px">
        <button v-if="ui.mode === 'create'" class="btn primary" :disabled="ui.busy" @click="doCreate">
          {{ ui.busy ? "Creating..." : "Create & Start" }}
        </button>

        <button v-else class="btn primary" :disabled="ui.busy || !canJoin" @click="doJoin">
          {{ ui.busy ? "Joining..." : "Join" }}
        </button>
      </div>

      <Transition name="fade-slide">
        <div v-if="ui.shareLink" style="margin-top: 14px">
          <div class="small" style="margin-bottom:6px">Invite link</div>
          <div class="row" style="align-items:center">
            <input class="input" :value="ui.shareLink" readonly />
            <button class="btn" @click="copyLink">Copy</button>
          </div>
          <div class="small" style="margin-top:6px">
            This link preselects the opponent’s side.
          </div>
        </div>
      </Transition>

      <Transition name="fade-slide">
        <div v-if="ui.info" style="margin-top: 12px" class="small">
          {{ ui.info }}
        </div>
      </Transition>
    </div>
  </div>
</template>
