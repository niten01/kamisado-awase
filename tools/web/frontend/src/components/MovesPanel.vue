<script setup>
import {computed} from "vue";

const props = defineProps({
  moves: {type: Array, default: () => []},
  showBest: {type: Boolean, required: true},
  bestMove: {type: Object, default: null},
  lastMove: {type: Object, default: null},
  terminal: {type: Object, default: () => ({status: "ongoing"})}
});

const bestText = computed(() => {
  if (!props.bestMove?.from) return "—";
  return `${props.bestMove.from} → ${props.bestMove.to}`;
});

const lastText = computed(() => {
  if (!props.lastMove?.from) return "—";
  return `${props.lastMove.from} → ${props.lastMove.to}`;
});
</script>

<template>
  <div class="panel-title">
    <div>
      <div style="font-weight:950">Moves</div>
    </div>
    <div class="row">
      <span class="badge accent" v-if="terminal?.status !== 'ongoing'">
        {{ terminal.status }} <span v-if="terminal.winner">• {{ terminal.winner }}</span>
      </span>
    </div>
  </div>

  <div class="grid" style="grid-template-columns: 1fr; gap:10px; margin-bottom: 12px">
    <div class="kv">
      <div class="k">Last move</div>
      <div class="v">{{ lastText }}</div>
    </div>
    <div class="kv" v-if="showBest">
      <div class="k">Best move</div>
      <Transition name="fade-slide" mode="out-in">
        <div class="v" :key="bestText">{{ bestText }}</div>
      </Transition>
    </div>
  </div>

  <div class="moves">
    <transition-group name="list" tag="div">
      <div v-for="(m, idx) in moves" :key="m.id || idx" class="move-item">
        <div class="left">
          <div class="ply">#{{ idx + 1 }}</div>
          <div class="mv">{{ m.notation || `${m.from}→${m.to}` }}</div>
          <div class="side">{{ m.side || "" }}</div>
        </div>
        <div class="small" style="text-align:right">
          <div>{{ m.from }} → {{ m.to }}</div>
          <div v-if="m.ts">{{ new Date(m.ts).toLocaleTimeString() }}</div>
        </div>
      </div>
    </transition-group>

    <div v-if="!moves.length" class="small">No moves yet.</div>
  </div>
</template>
