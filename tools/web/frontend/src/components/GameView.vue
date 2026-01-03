<script setup>
import {computed, reactive, watch} from "vue";
import Board from "./Board.vue";
import AdvantageBar from "./AdvantageBar.vue";
import MovesPanel from "./MovesPanel.vue";
import {sendMove} from "../api.js";
import {useToast} from "vue-toastification";

const toast = useToast();

const props = defineProps({
  sessionId: {type: String, required: true},
  token: {type: String, required: true},
  side: {type: String, required: true}, // "white" | "black"
  state: {type: Object, required: true},
  analysis: {type: Object, required: true}
});

const ui = reactive({
  selected: null,      // "a1"
  legal: [],           // ["a2", ...] (optional if backend provides)
  lastMove: null,      // {from,to}
  lastBestMove: null   // for anim triggers
});

const terminal = computed(() => props.state?.terminal || {status: "ongoing"});
const turnSide = computed(() => props.state?.turnSide || "");
const turnNum = computed(() => props.state?.moves?.length + 1 || "");
const myTurn = computed(() => turnSide.value && turnSide.value === props.side);
const moves = computed(() => props.state?.moves || []);

watch(
  () => props.state?.lastMove,
  (mv) => {
    if (mv?.from && mv?.to) ui.lastMove = mv;
  },
  {immediate: true}
);

watch(
  () => props.analysis?.bestMove,
  (bm) => {
    ui.lastBestMove = bm || null;
  },
  {immediate: true}
);

watch(
  () => terminal.value,
  (terminal) => {
    if (terminal.status === "win") {
      if (props.side == terminal.winner) {
        toast.success(`You won!`)
      } else {
        toast.error(`You lost!`)
      }
    }
  }
)

function canInteract() {
  return terminal.value?.status === "ongoing" && myTurn.value;
}

/**
 * Selection / move UX:
 * - click piece-square: select it (backend can enforce correct piece for kamisado)
 * - click destination: send move {from,to}
 * - if backend provides legal moves in state.legalMoves[from] use them
 */
function onSquareClick(square) {
  if (!canInteract()) return;

  const legalFromMap = props.state?.legalMovesMap || null;

  // second click => attempt move
  if (ui.selected) {
    const from = ui.selected;
    const to = square;

    // if legal list exists, require it
    if (legalFromMap && (!legalFromMap[from] || !legalFromMap[from].includes(to))) {
      // allow reselection
      ui.selected = square;
      ui.legal = legalFromMap?.[square] || [];
      return;
    }

    doMove(from, to);
    ui.selected = null;
    ui.legal = [];
    return;
  }

  // first click => select
  ui.selected = square;
  ui.legal = legalFromMap?.[square] || [];
}

async function doMove(from, to) {
  try {
    await sendMove({sessionId: props.sessionId, token: props.token, from, to});
    // no direct state set here; websocket should update quickly.
  } catch {
    // keep simple; App error banner could be extended
  }
}

</script>

<template>
  <div class="grid">
    <div class="card">
      <div class="card-body board-wrap">
        <div style="display:flex; align-items:center; justify-content:space-between; gap:12px; margin-bottom:12px">
          <div>
            <div style="font-weight:950; letter-spacing:.2px">Board</div>
            <div class="small">
              You: <b>{{ side }}</b> • Turn: <b>{{ turnNum || "?" }}</b>
              <span v-if="terminal.status !== 'ongoing'"> • Status: <b>{{ terminal.winner }} win</b></span>
            </div>
          </div>
          <div class="row">
            <span class="badge" v-if="analysis?.enabled">analysis ON</span>
            <span class="badge" v-else>analysis OFF</span>
            <span class="badge good" v-if="myTurn">your move</span>
            <span class="badge" v-else>waiting</span>
          </div>
        </div>

        <Board :state="state" :selected="ui.selected" :legalTargets="ui.legal" :lastMove="ui.lastMove"
          :bestMove="analysis?.bestMove" @square="onSquareClick" />

        <div style="margin-top: 14px" class="card" v-if="analysis?.enabled">
          <AdvantageBar :analysis="analysis" :side="side" />
        </div>
      </div>
    </div>

    <div class="card">
      <div class="card-body">
        <MovesPanel :moves="moves" :showBest="analysis?.enabled" :bestMove="analysis?.bestMove" :lastMove="ui.lastMove"
          :terminal="terminal" />
      </div>
    </div>
  </div>
</template>
