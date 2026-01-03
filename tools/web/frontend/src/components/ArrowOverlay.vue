<script setup>
import {computed, watch, ref} from "vue";

const props = defineProps({
  lastMove: {type: Object, default: null}, // {from,to}
  bestMove: {type: Object, default: null}  // {from,to}
});

const files = "abcdefgh";
function squareCenter(sq) {
  if (!sq) return null;
  const file = sq[0];
  const rank = Number(sq.slice(1));
  const col = files.indexOf(file);
  const row = 8 - rank;

  if (col < 0 || row < 0 || col > 7 || row > 7) return null;

  // percent coords inside board
  const x = ((col + 0.5) / 8) * 100;
  const y = ((row + 0.5) / 8) * 100;
  return {x, y};
}

function linePath(from, to) {
  const a = squareCenter(from);
  const b = squareCenter(to);
  if (!a || !b) return "";

  const shorten = 5;
  const vx = b.x - a.x;
  const vy = b.y - a.y;
  const len = Math.hypot(vx, vy) || 1;
  const bx = b.x - shorten * vx / len;
  const by = b.y - shorten * vy / len;

  const mx = (a.x + bx) / 2;
  const my = (a.y + by) / 2;
  const dx = bx - a.x;
  const dy = by - a.y;
  const k = 0.0; // curvature
  const cx = mx - dy * k;
  const cy = my + dx * k;
  return `M ${a.x} ${a.y} Q ${cx} ${cy} ${bx} ${by}`;
}

const lastPath = computed(() => (props.lastMove ? linePath(props.lastMove.from, props.lastMove.to) : ""));
const bestPath = computed(() => (props.bestMove ? linePath(props.bestMove.from, props.bestMove.to) : ""));

// trigger “pop” animation when best move changes
const bestKey = ref(0);
watch(
  () => `${props.bestMove?.from || ""}-${props.bestMove?.to || ""}`,
  () => {bestKey.value++;}
);
</script>

<template>
  <svg class="arrow-layer" viewBox="0 0 100 100" preserveAspectRatio="none">
    <defs>
      <marker id="arrowHeadLast" viewBox="0 0 10 10" refX="0" refY="5" markerWidth="2.5" markerHeight="2.5"
        orient="auto-start-reverse">
        <path d="M 0 0 L 10 5 L 0 10 z" fill="rgba(255,255,255,.85)"></path>
      </marker>
      <marker id="arrowHeadBest" viewBox="0 0 10 10" refX="0" refY="5" markerWidth="2.5" markerHeight="2.5"
        orient="auto-start-reverse">
        <path d="M 0 0 L 10 5 L 0 10 z" fill="rgba(140,160,255,.95)"></path>
      </marker>
    </defs>

    <path v-if="lastPath" class="arrow-last arrow-anim" :d="lastPath" fill="none" marker-end="url(#arrowHeadLast)"
      stroke-linecap="butt" stroke-linejoin="round" opacity=".90" />

    <path v-if="bestPath" :key="bestKey" class="arrow-best arrow-anim" :d="bestPath" fill="none"
      marker-end="url(#arrowHeadBest)" stroke-linecap="round" stroke-linejoin="round" opacity=".95" />
  </svg>
</template>
