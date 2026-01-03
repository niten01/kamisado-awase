<script setup>
import {computed} from "vue";
import ArrowOverlay from "./ArrowOverlay.vue";

const props = defineProps({
  state: {type: Object, required: true},
  selected: {type: String, default: null},
  legalTargets: {type: Array, default: () => []},
  lastMove: {type: Object, default: null}, // {from,to}
  bestMove: {type: Object, default: null}  // {from,to}
});
const emit = defineEmits(["square"]);

const board = computed(() => props.state?.board || []);
const files = "abcdefgh";

function toSquareId(row, col) {
  // board[0] is rank 8, board[7] is rank 1 (common UI)
  const file = files[col];
  const rank = 8 - row;
  return `${file}${rank}`;
}

function squareToRC(sq) {
  if (!sq || sq.length < 2) return null;
  const f = sq[0];
  const r = Number(sq.slice(1));
  const col = files.indexOf(f);
  const row = 8 - r;
  if (row < 0 || row > 7 || col < 0 || col > 7) return null;
  return {row, col};
}

function onClick(row, col) {
  emit("square", toSquareId(row, col));
}

function isSelected(row, col) {
  return props.selected === toSquareId(row, col);
}

function isLegal(row, col) {
  return props.legalTargets?.includes?.(toSquareId(row, col));
}

const colorMap = {
  orange: "rgba(255, 160, 84, .85)",
  blue: "rgba(110, 150, 255, .85)",
  purple: "rgba(190, 120, 255, .85)",
  pink: "rgba(255, 140, 200, .85)",
  yellow: "rgba(255, 230, 120, .9)",
  red: "rgba(255, 110, 110, .85)",
  green: "rgba(120, 240, 180, .85)",
  brown: "rgba(185, 130, 95, .85)"
};

function squareBg(cell) {
  // Kamisado squares are colored; backend should provide actual colors.
  // We map to pleasing colors here:
  const c = (cell?.color || "").toLowerCase();
  const base = colorMap[c] || "rgba(255,255,255,.08)";
  // subtle glass layer:
  return `linear-gradient(180deg, rgba(255,255,255,.14), rgba(0,0,0,.10)), ${base}`;
}

function pieceStyle(piece) {
  // used to animate piece reposition (in-place render per cell)
  return {
    background: colorMap[piece.color]
  }
}

function dotClass(piece) {
  return ["dot", piece?.side === "black" ? "black" : "white"];
}

function pieceClass(row, col) {
  let classes = ["piece"];
  if (props.state?.legalMovesMap?.[toSquareId(row, col)]) {
    classes.push("canmove");
  }
  return classes;
}

// A single glyph inside pieces (optional). Could show tower/monk icon later.
function pieceLabel(piece) {
  return piece?.id || "";
}

const last = computed(() => props.lastMove?.from && props.lastMove?.to ? props.lastMove : null);
const best = computed(() => props.bestMove?.from && props.bestMove?.to ? props.bestMove : null);
</script>

<template>
  <div class="board">
    <div class="squares">
      <!-- <div v-for="(row, r) in board" :key="'r' + r" v-forDummy /> -->
      <template v-for="(row, r) in board" :key="'row'+r">
        <div v-for="(cell, c) in row" :key="toSquareId(r, c)" class="square" :class="{
          selected: isSelected(r, c),
          legal: isLegal(r, c)
        }" :style="{background: squareBg(cell)}" @click="onClick(r, c)">
          {{ cell.dbg }}
          <div v-if="cell?.piece" :class="pieceClass(r, c)" :style="pieceStyle(cell?.piece)">
            <div style="display:flex; flex-direction:column; align-items:center; gap:8px">
              <div :class="dotClass(cell.piece)"></div>
              <div style="font-size:14px; opacity:.9">{{ pieceLabel(cell.piece) }}</div>
            </div>
          </div>
        </div>
      </template>
    </div>

    <!-- arrows overlay -->
    <ArrowOverlay :lastMove="last" :bestMove="best" />
  </div>
</template>
